#ifndef QOUTPUTDIALOG_H
#define QOUTPUTDIALOG_H

#include <QDialog>
#include <QJsonValue>
#include <QMap>

#pragma execution_character_set("utf-8")

namespace Ui {
class QOutPutDialog;
}
class QTableWidgetItem;
class QOutPutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QOutPutDialog(int itemID,QWidget *parent = 0);
    ~QOutPutDialog();

private:
//	void InitMapping(QMap<int,int> device_map);
	void InitMapping(QMap<int, int> device_map, QMap<int, int> delay_ms);
    void InitData();
    void InitTable();
    bool m_TextChangeNotify = false;      //表格更改通知开关
//    QTableWidgetItem*   m_DoubleClickedItem   =   nullptr;
    QString m_DoubleClickedText;
    QMap<int,int>  m_device_map;

private slots:

    void on_push_button_accepted();

    void on_mapping_itemDoubleClicked(QTableWidgetItem *item);

    void on_mapping_itemChanged(QTableWidgetItem *item);

//    void on_mapping_itemPressed(QTableWidgetItem *item);

//    void on_mapping_itemSelectionChanged();

private:
    Ui::QOutPutDialog *ui;
    int m_ItemID;
};

#endif // QOUTPUTDIALOG_H
