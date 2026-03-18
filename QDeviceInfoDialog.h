#ifndef QDEVICEINFODIALOG_H
#define QDEVICEINFODIALOG_H

#include <QDialog>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#pragma execution_character_set("utf-8")
namespace Ui {
class QDeviceInfoDialog;
}

class QDeviceInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QDeviceInfoDialog(int itemID,QWidget *parent = 0);
    ~QDeviceInfoDialog();

private:
    void InitData();

private slots:
    void on_push_button_accepted();

private:
    Ui::QDeviceInfoDialog *ui;
    int m_ItemID;
};

#endif // QDEVICEINFODIALOG_H
