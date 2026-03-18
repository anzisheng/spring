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
     QTreeItem.cpp

     A container for items of data supplied by the simple tree model.
 */

#include <QStringList>

#include "QTreeItem.h"

QTreeItem::QTreeItem(const QList<QVariant> &data, QTreeItem *parent)
{
    parentItem = parent;
    itemData = data;

    m_IsContainer = false;
}

QTreeItem::~QTreeItem()
{
    qDeleteAll(childItems);
}

void QTreeItem::appendChild(QTreeItem *item)
{
    childItems.append(item);
}

void QTreeItem::removeChild(QTreeItem *child)
{
    childItems.removeAll(child);
}

QTreeItem *QTreeItem::child(int row)
{
    return childItems.value(row);
}

int QTreeItem::childCount() const
{
    return childItems.count();
}

int QTreeItem::columnCount() const
{
    return itemData.count();
}

QVariant QTreeItem::data(int column) const
{
    return itemData.value(column);
}

QTreeItem *QTreeItem::parent()
{
    return parentItem;
}

int QTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<QTreeItem*>(this));

    return 0;
}


//////////////////////////////////���¾�Ϊ�¼�ҵ�����///////////////////////////////////


void QTreeItem::setContainer(bool bIsContainer)
{
    m_IsContainer = bIsContainer;
}

bool QTreeItem::IsContainer()
{
    return m_IsContainer;
}

void QTreeItem::setType(int iType)
{
    m_iType = iType;
}

int QTreeItem::getType()
{
    return m_iType;
}

void QTreeItem::setJsonValue(QJsonValue json_value)
{
    m_jsonValue = json_value;
}

QJsonValue QTreeItem::GetJsonValue()
{
    return m_jsonValue;
}

void QTreeItem::setItemName(QString qStrName)
{
    m_ItemName = qStrName;
}

QString QTreeItem::GetItemName()
{
    return m_ItemName;
}

void QTreeItem::setItemID(int ID)
{
    m_ID = ID;
}

int QTreeItem::GetItemID()
{
    return m_ID;
}
