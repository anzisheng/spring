#ifndef QTREEMODE_H
#define QTREEMODE_H

#include <QAbstractItemModel>
#include "QTreeItem.h"
#include <QDir>
#pragma execution_character_set("utf-8")

class QTreeMode : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QTreeMode(QObject *parent = 0);
    ~QTreeMode();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //��ӵ�
    int IsContainer(const QModelIndex &index);

//private:
public:
    void setupModelData();
    void setupOutPutDevice(QTreeItem* parent);
    void setupElectricDevice(QTreeItem* parent);
    void setupshowTeamsDevice(QTreeItem* parent);
    void setupShowOrbit(QTreeItem* parent);

//private:
public:
    QTreeItem *rootItem;

    void openFileDir();
    void setupQueueDevice(QTreeItem *parent);
    void setupCmpDevice(QTreeItem *parent);

private:
    QString LoadFileName;
    QDir m_dir;
    int dir_count;
};

#endif // QTREEMODE_H
