#include "ipadimagesetdialog.h"
#include "ui_ipadimagesetdialog.h"

#include "StaticValue.h"
#include <QSettings>

IpadImageSetDialog::IpadImageSetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IpadImageSetDialog)
{
    ui->setupUi(this);
    ui->lineEdit->setText(StaticValue::GetInstance()->str_ipaimageW);
    ui->lineEdit_2->setText(StaticValue::GetInstance()->str_ipaimageH);
    ui->lineEdit_3->setText(StaticValue::GetInstance()->str_ipaimageTimes);
    ui->lineEdit_4->setText(StaticValue::GetInstance()->str_ipaimageDelayTime);
}

IpadImageSetDialog::~IpadImageSetDialog()
{
    delete ui;
}

void IpadImageSetDialog::on_buttonBox_accepted()
{
//    ui->lineEdit->setText("OK");
    //Qt中使用QSettings类读写ini文件
       //QSettings构造函数的第一个参数是ini文件的路径,第二个参数表示针对ini文件,第三个参数可以缺省
       QSettings *configIniWrite = new QSettings("imageset.ini", QSettings::IniFormat);
       //向ini文件中写入内容,setValue函数的两个参数是键值对
       //向ini文件的第一个节写入内容,size节下的第一个参数
       configIniWrite->setValue("/size/first", ui->lineEdit->text());
       //向ini文件的第一个节写入内容,size节下的第二个参数
       configIniWrite->setValue("size/second", ui->lineEdit_2->text());
       //向ini文件的第二个节写入内容,other节下的第一个参数
       configIniWrite->setValue("/other/first", ui->lineEdit_3->text());
       //向ini文件的第二个节写入内容,other节下的第二个参数
       configIniWrite->setValue("other/second", ui->lineEdit_4->text());

       //写入完成后删除指针
       delete configIniWrite;

       StaticValue::GetInstance()->str_ipaimageW = ui->lineEdit->text();
       StaticValue::GetInstance()->str_ipaimageH = ui->lineEdit_2->text();
       StaticValue::GetInstance()->str_ipaimageTimes = ui->lineEdit_3->text();
       StaticValue::GetInstance()->str_ipaimageDelayTime = ui->lineEdit_4->text();

}

void IpadImageSetDialog::on_buttonBox_rejected()
{
 //   ui->lineEdit_2->setText("no");
}
