#include "ontimesetdialog.h"
#include "ui_ontimesetdialog.h"

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
#include <QMessageBox>


OnTimeSetDialog::OnTimeSetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnTimeSetDialog)
{
    ui->setupUi(this);
//右侧时间设置tree的初始化
    //根节点
    ui->treeWidget_2->setColumnCount(1); //设置列数
    ui->treeWidget_2->setHeaderLabel(tr("日期")); //设置头的标题

    Item1 = new QTreeWidgetItem(ui->treeWidget_2, QStringList(tr("星期一")));
 //   QTreeWidgetItem *Item1_1 = new QTreeWidgetItem(imageItem1,QStringList(QString("Band1"))); //子节点1
 //   imageItem1->addChild(imageItem1_1); //添加子节点

    Item2 = new QTreeWidgetItem(ui->treeWidget_2, QStringList(tr("星期二")));
//   QTreeWidgetItem *imageItem2_1 = new QTreeWidgetItem(imageItem2,QStringList(QString("Band1"))); //子节点1
//    QTreeWidgetItem *imageItem2_2 = new QTreeWidgetItem(imageItem2,QStringList(QString("Band2"))); //子节点2
//    imageItem2->addChild(imageItem2_1);  //添加子节点
//    imageItem2->addChild(imageItem2_2);
    Item3 = new QTreeWidgetItem(ui->treeWidget_2, QStringList(tr("星期三")));
    Item4 = new QTreeWidgetItem(ui->treeWidget_2, QStringList(tr("星期四")));
    Item5 = new QTreeWidgetItem(ui->treeWidget_2, QStringList(tr("星期五")));
    Item6 = new QTreeWidgetItem(ui->treeWidget_2, QStringList(tr("星期六")));
    Item7 = new QTreeWidgetItem(ui->treeWidget_2, QStringList(tr("星期日")));
    //子节点
//右侧时间列表的子节点要从保存的文件里读进来，保存文件方式考虑和触摸屏软一样


//左侧时间设置tree的初始化，把编曲文件读进来
    ui->treeWidget->setColumnCount(1); //设置列数
    ui->treeWidget->setHeaderLabel(tr("曲目")); //设置头的标题


    music_info = StaticValue::GetInstance()->m_music_info;
    keys = music_info.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        MusicInfo* music = music_info[keys.at(i)];

   //     QList<QVariant> variant;
        variant << music->name;
  //      QTreeItem* pItem = new QTreeItem(variant,cmp_pItem);
        pItem=new QTreeWidgetItem(ui->treeWidget,variant,0);
 //       pItem->setType(COMP_DEVICE);
  //      pItem->setItemID(music->id);
   //     pItem->setItemName(music->name);
    //    cmp_pItem->appendChild(pItem);
        variant.clear();
    }



    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
}


OnTimeSetDialog::~OnTimeSetDialog()
{
    delete ui;
}


void OnTimeSetDialog::OnButtonClicked()
{
//插入
    QTreeWidgetItem* weekItem=ui->treeWidget_2->currentItem();//获得当前节点
    if(NULL==weekItem)//没有选择节点
        return;
    QString str_week=weekItem->text(ui->treeWidget_2->currentColumn()); //获得点击的星期几的文字

    QTime ontimebegin = ui->timeEdit->time();
    QString str_begin = ontimebegin.toString("hh:mm:ss");
    QTime ontimestop = ui->timeEdit_2->time();
    QString str_stop = ontimestop.toString("hh:mm:ss");

    if(ontimebegin>=ontimestop)
        QMessageBox::information(this, tr("提示"), tr("结束时间不能小于开始时间，请重新输入！"),
                                 QMessageBox::Ok);

//   QTreeWidgetItem* timeItem;
//   timeItem->setText(0,str_begin+"-"+str_stop);
//   weekItem->addChild(timeItem);

    QTreeWidgetItem* timeItem;
    timeItem = new QTreeWidgetItem(weekItem,QStringList(str_begin+"-"+str_stop));





}
