#include "QOutPutDialog.h"
#include "ui_QOutPutDialog.h"
#include <QPushButton>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPalette>
#include <QColor>
#include "StaticValue.h"
#include "ClassDefined.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

QOutPutDialog::QOutPutDialog(int itemID,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QOutPutDialog)
{
    ui->setupUi(this);

    QPushButton* btn_ok = ui->push_button->button(QDialogButtonBox::Ok);
    btn_ok->setText(tr("确定"));
    QPushButton* btn_Cancel = ui->push_button->button(QDialogButtonBox::Cancel);
    btn_Cancel->setText(tr("取消"));

    ui->label->setPalette(QPalette(QColor::fromRgb(0,0,255)));

    m_ItemID = itemID;
    InitData();
}


QOutPutDialog::~QOutPutDialog()
{
    delete ui;
}

void QOutPutDialog::InitData()
{
    OutPutDevice* outPut_info = StaticValue::GetInstance()->
            m_output_device.value(m_ItemID);			//m_ItemID卡设备号，几号卡就是几
    if(outPut_info)
    {
        ui->name->setText(outPut_info->name);						//名称		例如：40路开关量1
        ui->type->setText(outPut_info->type);						//类型，		例如：SL40
        ui->serialPorts->setItemText(0,outPut_info->serialPorts);	//串口号
        ui->serialPorts->setCurrentIndex(0);
        ui->ipAddress->setText(outPut_info->ipAddress);				//网络地址，	例如：192.168.1.3
        ui->port->setText(QString("%1").arg(outPut_info->port));	//端口号		例如：6000
        ui->Address->setValue(outPut_info->Address.toInt());		//地址		例如：0
        if(outPut_info->isSixteenths)								//16进制		
            ui->ixteenths->setChecked(true);
        else
            ui->ixteenths->setChecked(false);						//槽号
        ui->slotNum->setValue(outPut_info->slotNum);				//40
        ui->comment->setText(outPut_info->comment);					//备注
		InitMapping(outPut_info->m_mapping, outPut_info->m_delay_ms);		//映射，里面是对应关系，需要在此处增加延迟
    }
}

//绘制表格
void QOutPutDialog::InitTable()
{
    QString qStrPort = tr("端口号");
    QString qStrDeviceID = tr("设备ID");
	QString qStrDelayID = tr("延迟(ms)");
    QStringList head_list;
	head_list << qStrPort << qStrDeviceID << qStrDelayID;				//获得第一排的数据：端口号、设备ID、延迟(ms)
    ui->mapping->clearContents();
//	ui->mapping->setColumnCount(2);										//画两列
	ui->mapping->setColumnCount(3);										//画三列，延迟(ms)就出来了
	ui->mapping->setAlternatingRowColors(true);							//居中的意思？
//	ui->mapping->setAlternatingRowColors(false);						//true和false结果看不出有啥区别
	ui->mapping->setHorizontalHeaderLabels(head_list);					//绘制第一排的内容，屏蔽此行会显示：123，不显示：端口号、设备ID、延迟(ms)
//    ui->mapping->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->mapping->setSelectionBehavior(QAbstractItemView::SelectRows);	//屏蔽此行看不出变化
}

//绘制表格
//void QOutPutDialog::InitMapping(QMap<int,int> device_map)
void QOutPutDialog::InitMapping(QMap<int, int> device_map, QMap<int, int> delay_ms_map)
{
    InitTable();									//绘制表格
    m_device_map    =   device_map;					//
    QList<int> keys = device_map.keys();			//获得板卡设备数量，SL40:40，估计SL32:32
    for(int i = 0; i < keys.size(); ++i)
    {
		int id = keys.at(i);						//板子的通道号，SL40是1-40
		int device_id = device_map[id];				//设备号，系统总的排序的设备号
		int delay_ms = delay_ms_map[id];							//延迟值

        int iRowCount = ui->mapping->rowCount();
        if(iRowCount <= i)
            ui->mapping->insertRow(iRowCount);

		QTableWidgetItem* id_item = new QTableWidgetItem(QString::number(id));					//板子通道号
		QTableWidgetItem* deviceId_item = new QTableWidgetItem(QString::number(device_id));		//设备号
		QTableWidgetItem* delay_item = new QTableWidgetItem(QString::number(delay_ms));			//延迟值，单位ms
		ui->mapping->setAlternatingRowColors(true);							//
		ui->mapping->setItem(i,0,id_item);			//绘制第一列数据，板子的通道号
		id_item->setFlags(id_item->flags() & ~Qt::ItemIsEditable);		// non editable 设备序号不可编辑
		ui->mapping->setItem(i,1, deviceId_item);	//绘制第二列数据，设备号
		ui->mapping->setItem(i,2, delay_item);		//绘制第三列数据，延迟值
    }
}

void QOutPutDialog::on_push_button_accepted()
{
    bool bReturn = false;
    if(QSqlDatabase::database().transaction())
    {
        OutPutDevice* outPut_info = StaticValue::GetInstance()->
                m_output_device.value(m_ItemID);
        outPut_info->name = ui->name->text();
        outPut_info->type = ui->type->text();
        outPut_info->serialPorts = ui->serialPorts->currentText();
        outPut_info->ipAddress = ui->ipAddress->text();
        outPut_info->port = ui->port->text().toInt();
        outPut_info->Address = ui->Address->text();
        if(ui->ixteenths->isChecked())
            outPut_info->isSixteenths = true;
        else
            outPut_info->isSixteenths = false;
        outPut_info->slotNum = ui->slotNum->text().toInt();
        outPut_info->comment = ui->comment->toPlainText();

        QString qStrSQL = QString("update output_device set name = '%1',type = '%2',"
                                  "serialPorts = '%3',ipAddress='%4',port = '%5',Address = '%6',"
                                  "isSixteenths = %7,slotNum=%8,comment='%9' where id = %10").
                arg(outPut_info->name).arg(outPut_info->type).arg(outPut_info->serialPorts).
                arg(outPut_info->ipAddress).arg(outPut_info->port).arg(outPut_info->Address).
                arg(outPut_info->isSixteenths).arg(outPut_info->slotNum).
                arg(outPut_info->comment).arg(m_ItemID);
        QSqlQuery query(qStrSQL);
        if(!query.exec())
        {
            qDebug() <<"update output_device false!sql:"<<qStrSQL<<
                       " error:"<<query.lastError().text();
            goto END;
        }

        outPut_info->m_mapping.clear();
		outPut_info->m_delay_ms.clear();			//清除内容？
        int rowCount = ui->mapping->rowCount();
        for(int i = 0; i < rowCount;++i)
        {
            int elec_id = ui->mapping->item(i,0)->data(0).toInt();
            int device_id = ui->mapping->item(i,1)->data(0).toInt();
			int delay_ms = ui->mapping->item(i, 2)->data(0).toInt();			//???

            outPut_info->m_mapping[elec_id] = device_id;
			outPut_info->m_delay_ms[elec_id] = delay_ms;

			//设置数据库里面的device_info项
			//设置id(设备号)值为device_id的设备的elec_device_id项为elec_id里面存储的新值
            qStrSQL = QString("update device_info set elec_device_id = %1 where id = %2")
                    .arg(elec_id).arg(device_id);
            if(!query.exec(qStrSQL))
            {
                qDebug() <<"update device_info false!sql:"<<qStrSQL<<
                           " error:"<<query.lastError().text();
                goto END;
            }

			//更新延迟值
			qStrSQL = QString("update device_info set DelayInms = %1 where id = %2")
				.arg(delay_ms).arg(device_id);
			if (!query.exec(qStrSQL))
			{
				qDebug() << "update device_info false!sql:" << qStrSQL <<
					" error:" << query.lastError().text();
				goto END;
			}

        }
        bReturn = true;
    }
END:
    if(bReturn)
    {
        bool bCommit = QSqlDatabase::database().commit();
        if(bCommit)
            qDebug() << tr("保存输出设备信息成功!");
        else
            qDebug() << tr("保存输出设备信息失败!");
    }
    else
    {
        bool rollback = QSqlDatabase::database().rollback();
        if(rollback)
            qDebug() << tr("回滚输出设备信息成功!");
        else
            qDebug() << tr("回滚输出设备信息失败!");
    }
}




void QOutPutDialog::on_mapping_itemDoubleClicked(QTableWidgetItem *item)
{
    m_TextChangeNotify    =   true;
    m_DoubleClickedText   = item->text();
}

//在此函数中调用了setText,会导致此函数被递归调用，尽量不要再改动此函数，坑太多，暂时没有太好的解决办法
void QOutPutDialog::on_mapping_itemChanged(QTableWidgetItem *item)
{
    if(m_TextChangeNotify==false)
    {
        return;
    }
	if (1 == item->column())
	{	//第一列检查设备号范围，第二列延迟值不需要检查
		bool vaildData = false;
		foreach(int i, m_device_map)
		{
			if (i == item->text().toInt())      //合法的设备号
			{
				vaildData = true;
				break;
			}
		}
		if (vaildData == false)                        //不合法的设备号，数据回滚
		{
			QMessageBox::information(this, "友情提示", "不合法的设备号");
			m_TextChangeNotify = false;
			item->setText(m_DoubleClickedText);
		}
	}

	int t1 = ui->mapping->rowCount();		//获得总行数
	int  t2,t3;
    for(int i = 0;i<ui->mapping->rowCount();i++)
    {
		t2 = item->row();		//修改的行
		t3 = item->column();	//修改的列
        if(item->row()==i)
        {
            continue;			//不参与自身查找
        }
        QTableWidgetItem*   itritem =      ui->mapping->item(i,1);	//
        if(itritem->text()==item->text())
        {
            m_TextChangeNotify=false;
            itritem->setText(m_DoubleClickedText);
            return;
        }
    }
}

//void QOutPutDialog::on_mapping_itemPressed(QTableWidgetItem *item)
//{
//    QMessageBox::information(this,"友情提示","itemPressed");
//}

//void QOutPutDialog::on_mapping_itemSelectionChanged()
//{
//    QMessageBox::information(this,"友情提示","itemSelectionChanged");
//}
