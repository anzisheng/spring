#include "aosetting.h"
#include "ui_aosetting.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>

#pragma execution_character_set("utf-8")

AOsetting::AOsetting( QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AOsetting)
{
    ui->setupUi(this);

    Manultimer_ =   new QTimer;
    AOvolset_   =   new QUdpSocket;
    connect(Manultimer_,&QTimer::timeout,this,&AOsetting::ManualTimeup);
}

AOsetting::~AOsetting()
{
    delete ui;
}

void AOsetting::accept()
{

    lowVoltage_ =   ui->lineEdit_3->text().toFloat();

    HighVoltage_    =   ui->lineEdit_4->text().toFloat();

    ipaddr_     =   ui->lineEdit_5->text();

    enable_     =   ui->output->isChecked();

    //写入到数据库

    QSqlQuery   query;

    QString qStrSql  =   QString("update AOsetting set addr = '%1',lowvoltage = %2,highvoltage = %3 where 1 = 1 ").
		arg(ipaddr_).arg(lowVoltage_).arg(HighVoltage_);
    //QString qStrSql  =   QString("update AOsetting set addr = '%1',lowvoltage = %2,highvoltage = %3 where 1 = 1 ").arg(ipaddr_).arg(lowVoltage_).arg(HighVoltage_);


    if(query.exec(qStrSql))
    {

    }
    else
    {
        QSqlError errinfo = query.lastError();
        QMessageBox::information(this,"警告","无法存储AO变频数据!");
    }

    QDialog::accept();
}

void AOsetting::resetsetting()
{
    ui->lineEdit_5->setText(ipaddr_);

    ui->lineEdit_3->setText( QString("%1").arg(lowVoltage_));

    ui->lineEdit_4->setText( QString("%1").arg(HighVoltage_));

    ui->output->setChecked(enable_);
}

void AOsetting::on_pushButton_clicked()
{
    qreal maxvol    =   ui->lineEdit_2->text().toFloat();

    qreal manualvol =   ui->manualvol->text().toFloat();

    if(manualvol>maxvol)
    {
        QMessageBox::information(this,"警告","超过最大电压限制！");
        return;
    }

    curDuration_    =   0;

    Manultimer_->start(100);
}

void AOsetting::ManualTimeup()
{
    qreal duration  =   ui->duration->text().toFloat()*1000;   //转换成毫秒

    if(curDuration_>=duration)          //手动注水到时
    {
        Manultimer_->stop();
        sendvol(0);
        ui->pushButton->setText("启动手动补水");
        return;
    }

    ui->pushButton->setText(tr("注水时间%1").arg(curDuration_/1000.0));

    qreal maxvol    =   ui->lineEdit_2->text().toFloat();

    qreal manualvol =   ui->manualvol->text().toFloat();

    int digvol  =   manualvol*255/maxvol;

    sendvol(digvol);

    curDuration_+=100;
}

void AOsetting::sendvol(int vol)
{
    char k  =   0;
    QByteArray  ComByte;  //指令序列
    ComByte.append("AH",2);
    ComByte.append(k);
    for(int i=0;i<16;i++)
    {
        ComByte.append(vol);
    }
    Q_ASSERT(ComByte.size()==19);
    unsigned char t=0;
    for(int i=0;i<19;i++)
        t+=ComByte[i];
    ComByte.append(t);
    AOvolset_->writeDatagram(ComByte,QHostAddress(ui->lineEdit_5->text()),6000);
}

void AOsetting::closeEvent(QCloseEvent *event)
{
    if(Manultimer_->isActive())
    {
        Manultimer_->stop();
        sendvol(0);
    }
}
