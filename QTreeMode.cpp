
/****************************************************************************
 **
 ** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
 **     of its contributors may be used to endorse or promote products derived
 **     from this software without specific prior written permission.
 **
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

/*
     QPersonSelectTreeModel.cpp

     Provides a simple tree model to show how to create and use hierarchical
     models.
 */
#include "QTreeMode.h"
#include <QtGui>
#include <string>
#include "QTreeItem.h"
#include "CommonFile.h"
#include "StaticValue.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
using namespace std;


QTreeMode::QTreeMode(QObject *parent)
    : QAbstractItemModel(parent)
{
    qDebug()<<"TreeMode构造开始";
    QList<QVariant> rootData;
    rootData << tr("系统设置");
    rootItem = new QTreeItem(rootData);
    setupModelData();
    qDebug()<<"TreeMode构造结束";
}

QTreeMode::~QTreeMode()
{
    delete rootItem;
}

int QTreeMode::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<QTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant QTreeMode::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    QTreeItem *item = static_cast<QTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags QTreeMode::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant QTreeMode::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex QTreeMode::index(int row, int column, const QModelIndex &parent)
const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    QTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<QTreeItem*>(parent.internalPointer());

    QTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex QTreeMode::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    QTreeItem *childItem = static_cast<QTreeItem*>(index.internalPointer());
    QTreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int QTreeMode::rowCount(const QModelIndex &parent) const
{
    QTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<QTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

// -1: error
// 1: container
// 0: node
int QTreeMode::IsContainer(const QModelIndex &index)
{
    if (!index.isValid())
        return -1;

    QTreeItem *item = static_cast<QTreeItem*>(index.internalPointer());
    if( NULL == item)
        return -1;

    if(item->IsContainer())
        return 1;
    return 0;
}


//////////////////////////////////////////////////以下为具体业务逻辑///////////////////////////////////////////////////


//设置model属性值
void QTreeMode::setupModelData()
{

    QList<QVariant> variant;
    variant << tr("输出设备");
    QTreeItem* output_Item = new QTreeItem(variant,rootItem);
    output_Item->setType(OUTPUT_DEVICE);
    output_Item->setContainer(true);
    rootItem->appendChild(output_Item);
    setupOutPutDevice(output_Item);
    variant.clear();

    variant << tr("表演编队");
    QTreeItem* showTeam_pItem = new QTreeItem(variant,rootItem);
    showTeam_pItem->setType(SHOWTEAMS_DEVICE);
    showTeam_pItem->setContainer(true);
    rootItem->appendChild(showTeam_pItem);
    setupshowTeamsDevice(showTeam_pItem);
    variant.clear();

    variant << tr("表演轨迹");
    QTreeItem* orbit_pItem = new QTreeItem(variant,rootItem);
    orbit_pItem->setType(ORBIT_DEVICE);
    orbit_pItem->setContainer(true);
    rootItem->appendChild(orbit_pItem);
    setupShowOrbit(orbit_pItem);
    variant.clear();

    openFileDir();

    variant << tr("编曲文件");
    QTreeItem* cmp_pItem = new QTreeItem(variant,rootItem);
    cmp_pItem->setType(COMP_DEVICE);
    cmp_pItem->setContainer(true);
    rootItem->appendChild(cmp_pItem);
    setupCmpDevice(cmp_pItem);
    variant.clear();

    variant << tr("表演序列");
    QTreeItem* queue_pItem = new QTreeItem(variant,rootItem);
    queue_pItem->setType(QUEUE_DEVICE);
    queue_pItem->setContainer(true);
    rootItem->appendChild(queue_pItem);
    setupQueueDevice(queue_pItem);
    variant.clear();

    variant << tr("电气设备");
    QTreeItem* electric_pItem = new QTreeItem(variant,rootItem);
    electric_pItem->setType(ELECTRIC_DEVICE);
    electric_pItem->setContainer(true);
    rootItem->appendChild(electric_pItem);
    setupElectricDevice(electric_pItem);

    //add by eagle for tree music add, 这里要关联编曲, 这里就不需要了，因为mudicinfo里面自带
    // Add a new menu item for Music Tracks
    //  QList<QVariant> musicTracksData;
    //  musicTracksData << ("音乐曲目");
    //  QTreeItem *musicTracksItem = new QTreeItem(musicTracksData, rootItem);
    //  rootItem->appendChild(musicTracksItem);

    //add end.
}

void QTreeMode::setupOutPutDevice(QTreeItem* parent)
{
    QMap<int,OutPutDevice*> output_device = StaticValue::GetInstance()->m_output_device;
    int iSize = output_device.size();
    QList<int> keys = output_device.keys();
    for(int i = 0; i < iSize; i++)
    {
        int id = keys.at(i);
        QString name = output_device[id]->name;
        QList<QVariant> variant;
        variant << (QString::number(id) + "." + name);
        QTreeItem* pItem = new QTreeItem(variant,parent);
        pItem->setType(OUTPUT_DEVICE);
        pItem->setItemID(id);
        pItem->setItemName(name);
        pItem->setContainer(false);
        parent->appendChild(pItem);
    }
}

void QTreeMode::setupElectricDevice(QTreeItem* parent)
{
    QMap<int,DeviceInfo*> device_info = StaticValue::GetInstance()->m_device_map;
    int iSize = device_info.size();
    QList<int> keys = device_info.keys();
    for(int i = 0; i < iSize; i++)
    {
        int id = keys.at(i);
        QString name = device_info[id]->name;
        QList<QVariant> variant;
        variant << (QString::number(id) + "." + name);
        QTreeItem* pItem = new QTreeItem(variant,parent);
        pItem->setType(ELECTRIC_DEVICE);
        pItem->setItemID(id);
        pItem->setItemName(name);
        pItem->setContainer(false);
        parent->appendChild(pItem);
    }
}


void QTreeMode::setupshowTeamsDevice(QTreeItem* parent)
{
    QMap<int,ShowTeam*> show_team = StaticValue::GetInstance()->m_show_team;
    QList<int> keys = show_team.keys();
    for(int i = 0; i < show_team.size(); ++i)
    {
        int id = keys.at(i);
        ShowTeam* team = show_team[id];
        QString name = team->name;

        QList<QVariant> variant;
        variant << QString::number(id) + "." + name;
        QTreeItem* pItem = new QTreeItem(variant,parent);
        pItem->setType(SHOWTEAMS_DEVICE);
        pItem->setItemID(id);
        pItem->setItemName(name);
        pItem->setContainer(false);
        parent->appendChild(pItem);
    }
}

void QTreeMode::setupShowOrbit(QTreeItem *parent)
{
    QMap<int,Orbit*> show_orbit = StaticValue::GetInstance()->m_show_orbit;
    QList<int> keys = show_orbit.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        Orbit* orbit = show_orbit[keys.at(i)];
        QList<QVariant> variant;
        variant << QString::number(orbit->id) + "." + orbit->name;
        QTreeItem* pItem = new QTreeItem(variant,parent);
        pItem->setType(ORBIT_DEVICE);
        pItem->setItemID(orbit->id);
        pItem->setItemName(orbit->name);
        pItem->setContainer(false);
        parent->appendChild(pItem);
    }
}

//在Tree上显示编曲
void QTreeMode::setupCmpDevice(QTreeItem* parent)
{
    QMap<int,MusicInfo*> music_info = StaticValue::GetInstance()->m_music_info;
    QList<int> keys = music_info.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        MusicInfo* music = music_info[keys.at(i)];
        QList<QVariant> variant;
	//	variant << music->name;
		variant << QString::number(music->id) + ". " + music->name;
        QTreeItem* pItem = new QTreeItem(variant,parent);
        pItem->setType(COMP_DEVICE);
        pItem->setItemID(music->id);
		pItem->setItemName(music->name);
        parent->appendChild(pItem);
    }
}

void QTreeMode::setupQueueDevice(QTreeItem* parent)
{

    QStringList filters;
    filters<<"*.que";
    m_dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters格式（后缀为.cmp或.que）

    dir_count = m_dir.count();
    if(dir_count <= 0)
    {
        return;
    }

    QList<QFileInfo> *fileInfo=new QList<QFileInfo>(m_dir.entryInfoList(filters));
    for(int i=0;i < dir_count;i++)
    {
        LoadFileName = fileInfo->at(i).fileName();
        QList<QVariant> variant;
        variant << LoadFileName;
        QTreeItem* pItem = new QTreeItem(variant,parent);
        pItem->setType(QUEUE_DEVICE);
        pItem->setItemID(i);
        pItem->setItemName(LoadFileName);
        parent->appendChild(pItem);
    }
}



void QTreeMode::openFileDir()
{
    //判断路径是否存在
    QString path;
    path=m_dir.currentPath();
    QString foldername="/LOGFILE";
    path=path+foldername;

    bool exist = m_dir.exists(path);
    if(!exist)
    {
        return;
    }
    m_dir.setCurrent(path);

}

