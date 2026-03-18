#ifndef ONTIMESETDIALOG_H
#define ONTIMESETDIALOG_H

#include <QDialog>
#include <QTreeWidget>


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
#pragma execution_character_set("utf-8")
namespace Ui {
class OnTimeSetDialog;
}

class OnTimeSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OnTimeSetDialog(QWidget *parent = 0);
    ~OnTimeSetDialog();
private slots:
    void OnButtonClicked();

public:
 //星期一 --- 星期日的 item
    QTreeWidgetItem *Item1;
    QTreeWidgetItem *Item2;
    QTreeWidgetItem *Item3;
    QTreeWidgetItem *Item4;
    QTreeWidgetItem *Item5;
    QTreeWidgetItem *Item6;
    QTreeWidgetItem *Item7;
//左树和曲目相关的变量
    QMap<int,MusicInfo*> music_info;
    QList<int> keys ;
    QStringList variant;
    QTreeWidgetItem *pItem;//左树曲目item


private:
    Ui::OnTimeSetDialog *ui;
};

#endif // ONTIMESETDIALOG_H
