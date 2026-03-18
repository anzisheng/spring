#include "remotedlg.h"
#include "ui_remotedlg.h"
#include <QNetworkInterface>
RemoteDlg::RemoteDlg(remotecontrol *cursetting, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteDlg)
{
    curset_ =   cursetting;
    ui->setupUi(this);
    ui->CommonPort->setText(QString("%1").arg(curset_->CommonPort));
    ui->DMXport->setText(QString("%1").arg(curset_->DMXPort));
    ui->DMXaddr->setText(curset_->DMXAddr);
    ui->DataIndex->setText(QString("%1").arg(curset_->DataIndex));
    ui->DataLength->setText(QString("%1").arg(curset_->DataLength));

    if(curset_->WorkingMode==remotecontrol::COMMON)
    {
        ui->radioButton->setChecked(true);
    }
    else if(curset_->WorkingMode==remotecontrol::DMX)
    {
        ui->radioButton_2->setChecked(true);
    }
    else if(curset_->WorkingMode==remotecontrol::NONE)
    {
        ui->radioButton_3->setChecked(true);
    }
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            ui->listWidget->addItem(address.toString());
    }
}

RemoteDlg::~RemoteDlg()
{
    delete ui;
}

void RemoteDlg::on_buttonBox_accepted()
{
    if(ui->radioButton->isChecked())
    {
        curset_->WorkingMode    =   remotecontrol::COMMON;
    }
    else if(ui->radioButton_2->isChecked())
    {
        curset_->WorkingMode    =   remotecontrol::DMX;
    }
    else if(ui->radioButton_3->isChecked())
    {
        curset_->WorkingMode    =   remotecontrol::NONE;
    }
    curset_->CommonPort     =   ui->CommonPort->text().toInt();
    curset_->DMXPort        =   ui->DMXport->text().toInt();
    curset_->DMXAddr        =   ui->DMXaddr->text();
    curset_->DataIndex      =   ui->DataIndex->text().toInt();
    curset_->DataLength     =   ui->DataLength->text().toInt();
}
