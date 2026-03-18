#include "QDeviceInfoDialog.h"
#include "ui_QDeviceInfoDialog.h"
#include <QMessageBox>
#include <QPushButton>
#include "ClassDefined.h"
#include "StaticValue.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

QDeviceInfoDialog::QDeviceInfoDialog(int itemID,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QDeviceInfoDialog)
{
    ui->setupUi(this);

    QPushButton* btn_ok = ui->push_button->button(QDialogButtonBox::Ok);
    btn_ok->setText(tr("确定"));
    QPushButton* btn_Cancel = ui->push_button->button(QDialogButtonBox::Cancel);
    btn_Cancel->setText(tr("取消"));

    m_ItemID = itemID;
    InitData();
}


QDeviceInfoDialog::~QDeviceInfoDialog()
{
    delete ui;
}

void QDeviceInfoDialog::InitData()
{
    DeviceInfo* device_info = StaticValue::GetInstance()->m_device_map.value(m_ItemID);
    if(device_info)
    {
        ui->ID->setValue(device_info->id);
        ui->name->setText(device_info->name);
        ui->type->setText(device_info->type);
        ui->pos_x->setValue(device_info->pos_x);
        ui->pos_y->setValue(device_info->pos_y);
        ui->pos_z->setValue(device_info->pos_z);
        if(device_info->isDefault)
            ui->default_show->setChecked(true);
        else
            ui->default_show->setChecked(false);
    }
}

void QDeviceInfoDialog::on_push_button_accepted()
{
    DeviceInfo* device_info = StaticValue::GetInstance()->m_device_map.value(m_ItemID);
    device_info->id =  ui->ID->value();
    device_info->name = ui->name->text();
    device_info->type = ui->type->text();
    device_info->pos_x = ui->pos_x->value();
    device_info->pos_y = ui->pos_y->value();
    device_info->pos_z = ui->pos_z->value();

    if(ui->default_show->isChecked())
        device_info->isDefault = true;
    else
        device_info->isDefault = false;
    QString qStrSql = QString("update device_info "
                              "set name = '%1',type = '%2',pos_x = %3,"
                              "pos_y = %4,pos_z = %5,isDefault = %6 where id = %7").
            arg(device_info->name).arg(device_info->type).arg(device_info->pos_x).
            arg(device_info->pos_y).arg(device_info->pos_z).
            arg(device_info->isDefault).arg(device_info->id);
    QSqlQuery query(qStrSql);
    if(!query.exec())
    {
        qDebug() << "Update device_info false!sql:" <<qStrSql
                 <<" error:"<<query.lastError().text();
    }
}

