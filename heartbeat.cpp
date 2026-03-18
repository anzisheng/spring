#include "heartbeat.h"
#include "ui_heartbeat.h"

heartbeat::heartbeat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::heartbeat)
{
    ui->setupUi(this);
}

heartbeat::~heartbeat()
{
    delete ui;
}


void heartbeat::on_checkBox_clicked(bool checked)
{

}
