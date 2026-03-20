#include "QSubTreeView.h"
#include <QMessageBox>
#include "CommonFile.h"
#include <QDebug>
#include "StaticValue.h"
#include "ClassDefined.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "QTreeItem.h"
#include "QTreeMode.h"
#include "editiddialog.h"

QSubTreeView::QSubTreeView(QWidget *parent) :
QTreeView(parent)
{
	m_Menu = new QMenu(this);
	m_NewOrbit = new QAction(m_Menu);
	m_NewOrbit->setText(tr("新建轨迹动作"));

	m_NewDMX = new QAction(m_Menu);
	m_NewDMX->setText(tr("新建标准DMX512灯光设备"));
	m_NewSL32 = new QAction(m_Menu);
	m_NewSL32->setText(tr("新建E-SL32输出设备"));
	m_NewSL40 = new QAction(m_Menu);
	m_NewSL40->setText(tr("新建E-SL40输出设备"));
	m_NewCanMain = new QAction(m_Menu);
	m_NewCanMain->setText(tr("新建E-CAN主控设备"));
	m_NewTTL = new QAction(m_Menu);
	m_NewTTL->setText(tr("新建TTL长灯光设备"));

	m_NewOutPut = new QMenu(m_Menu);
	m_NewOutPut_16 = new QAction(m_NewOutPut);
	m_NewOutPut_32 = new QAction(m_NewOutPut);
	m_NewOutPut_48 = new QAction(m_NewOutPut);
	m_NewOutPut_64 = new QAction(m_NewOutPut);

	m_NewOutPut_16->setData(16);
	m_NewOutPut_32->setData(32);
	m_NewOutPut_48->setData(48);
	m_NewOutPut_64->setData(64);

	m_NewOutPut_16->setText("16路输出设备");
	m_NewOutPut_32->setText("32路输出设备");
	m_NewOutPut_48->setText("48路输出设备");
	m_NewOutPut_64->setText("64路输出设备");
	m_NewOutPut->addAction(m_NewOutPut_16);
	m_NewOutPut->addAction(m_NewOutPut_32);
	m_NewOutPut->addAction(m_NewOutPut_48);
	m_NewOutPut->addAction(m_NewOutPut_64);
	m_NewOutPut->setTitle(tr("新建输出设备"));

	m_NewshowTeam = new QAction(m_Menu);
	m_NewshowTeam->setText(tr("新建表演编队"));
	m_NewMusic = new QAction(m_Menu);
	m_NewMusic->setText(tr("新建编曲"));
	m_NewProgramControl = new QAction(m_Menu);
	m_NewProgramControl->setText(tr("新建程控"));
	m_NewQueryMusic = new QAction(m_Menu);
	m_NewQueryMusic->setText(tr("新建表演序列"));

	m_Delete = new QAction(m_Menu);
	//    m_Delete->setText(tr("删除"));
	m_Delete->setText(tr("删除"));

	m_reconnectMusicAction = new QAction(m_Menu);
	m_reconnectMusicAction->setText(tr("重新关联音乐"));
	m_reconnectMusicAction->setVisible(false);		//什么意思？

	m_manualAssignmentId = new QAction(m_Menu);
	m_manualAssignmentId->setText(tr("手工赋值id号"));

	connect(m_NewDMX, SIGNAL(triggered()), this, SLOT(newDMX()));
	connect(m_NewSL32, SIGNAL(triggered()), this, SLOT(newSL32()));
	connect(m_NewSL40, SIGNAL(triggered()), this, SLOT(newSL40()));
	connect(m_NewCanMain, SIGNAL(triggered()), this, SLOT(newCanMain()));
	connect(m_NewTTL, SIGNAL(triggered()), this, SLOT(newTTL()));

	connect(m_NewOrbit, SIGNAL(triggered()), this, SLOT(newOrbit()));
	connect(m_NewOutPut_16, SIGNAL(triggered()), this, SLOT(newOutPut()));
	connect(m_NewOutPut_32, SIGNAL(triggered()), this, SLOT(newOutPut()));
	connect(m_NewOutPut_48, SIGNAL(triggered()), this, SLOT(newOutPut()));
	connect(m_NewOutPut_64, SIGNAL(triggered()), this, SLOT(newOutPut()));

	connect(m_NewshowTeam, SIGNAL(triggered()), this, SLOT(newShowTeam()));

	connect(m_Delete, SIGNAL(triggered()), this, SLOT(deleteItem()));						//动作关联对应的函数
	connect(m_manualAssignmentId, SIGNAL(triggered()), this, SLOT(manualAssignmentId()));	//动作关联对应的函数

	connect(m_NewMusic, SIGNAL(triggered()), this, SLOT(newMusic()));
	connect(m_NewProgramControl, SIGNAL(triggered()), this, SLOT(newProgramControl()));
	connect(m_NewQueryMusic, SIGNAL(triggered()), this, SLOT(newQueryMusic()));
}

void QSubTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		QModelIndex index = currentIndex();
		if (!index.isValid())
		{
			QMessageBox::information(this, "test", "index is invalid!!!");
			return;
		}
		m_pCurrentItem = static_cast<QTreeItem*>(index.internalPointer());
		if (NULL == m_pCurrentItem)
			return;
		if (!m_pCurrentItem->IsContainer())      //鼠标双击消息不能正确的到达这里，为什么呢？
		{
			//            QMessageBox::information(this,"test","鼠标双击树控件");
			emit leftDoubleClicked(m_pCurrentItem->getType(),
				m_pCurrentItem->GetItemID());
		}
	}
}

void QSubTreeView::mousePressEvent(QMouseEvent * event)
{
	QTreeView::mousePressEvent(event);

	QModelIndex index = this->currentIndex();
	if (!index.isValid())
		return;
	m_pCurrentItem = static_cast<QTreeItem*>(index.internalPointer());
	if (NULL == m_pCurrentItem)
		return;
	if (event->button() == Qt::LeftButton)
	{
		if (QApplication::keyboardModifiers() == Qt::ControlModifier)
		{
			if (m_pCurrentItem->getType() == SHOWTEAMS_DEVICE)
			{
				emit addShowTeam(m_pCurrentItem->GetItemID());
			}
		}
	}
	else if (event->button() == Qt::RightButton)
	{
		int iCurrentType = m_pCurrentItem->getType();
		//        m_Menu->clear(); clear 后actions句柄即为空
		clearMenu();
		if (m_pCurrentItem->IsContainer())   //容器类型不允许删除
		{
			if (iCurrentType == ORBIT_DEVICE)
			{
				m_Menu->addAction(m_NewOrbit);
			}
			else if (iCurrentType == OUTPUT_DEVICE)
			{
				m_Menu->addAction(m_NewOutPut->menuAction());
				m_Menu->addAction(m_NewDMX);
				m_Menu->addAction(m_NewSL32);
				m_Menu->addAction(m_NewSL40);
				m_Menu->addAction(m_NewCanMain);
				m_Menu->addAction(m_NewTTL);
			}
			else if (iCurrentType == SHOWTEAMS_DEVICE)
			{
				m_Menu->addAction(m_NewshowTeam);
			}
			else if (iCurrentType == COMP_DEVICE)
			{
				m_Menu->addAction(m_NewMusic);
				m_Menu->addAction(m_NewProgramControl);
			}
			else if (iCurrentType == QUEUE_DEVICE)
			{
				m_Menu->addAction(m_NewQueryMusic);
			}
		}
		else
		{
			if (ELECTRIC_DEVICE != iCurrentType && !m_pCurrentItem->IsContainer())
			{
				m_Menu->addAction(m_Delete);
				if (iCurrentType == COMP_DEVICE)
				{
					m_reconnectMusicAction->setVisible(true);
					m_Menu->addAction(m_reconnectMusicAction);
				}
				m_Menu->addAction(m_manualAssignmentId);
			}
		}
		QPoint global_pos = this->mapToGlobal(event->pos());
		if (m_Menu->actions().size() > 0)
			m_Menu->exec(global_pos);
		
		if (iCurrentType == COMP_DEVICE)
		{
			m_reconnectMusicAction->setVisible(false);
		}
	}


}

void QSubTreeView::clearMenu()
{
	//      m_Menu->clear();
	m_Menu->removeAction(m_NewDMX);
	m_Menu->removeAction(m_NewOutPut->menuAction());
	m_Menu->removeAction(m_NewshowTeam);
	m_Menu->removeAction(m_NewOrbit);
	m_Menu->removeAction(m_NewMusic);
	m_Menu->removeAction(m_NewProgramControl);
	m_Menu->removeAction(m_NewQueryMusic);
	m_Menu->removeAction(m_Delete);
//	m_Menu->removeAction(m_Delete);
	m_Menu->removeAction(m_NewSL32);
	m_Menu->removeAction(m_NewSL40);
	m_Menu->removeAction(m_NewCanMain);
	m_Menu->removeAction(m_NewTTL);
}

void QSubTreeView::newOrbit()
{
	qDebug() << "new orbit";
	Orbit* orbit = new Orbit();
	orbit->name = tr("表演轨迹") +
		QString::number(orbit->id);
	orbit->time_length = 1000;
	orbit->delay_time = 10;
	orbit->reversal = false;
	orbit->reversal_output = false;

	QString qStrSql = QString("insert into orbit(name,time_length,delay_time,reversal,reversal_ouput)"
		" values('%1',%2,%3,%4,%5)").
		arg(orbit->name).arg(orbit->time_length).arg(orbit->delay_time).
		arg(orbit->reversal).arg(orbit->reversal_output);
	QSqlQuery query;
	if (!query.exec(qStrSql))
	{
		qDebug() << "add orbit false!" << query.lastError().text();
		return;
	}
	if (!query.exec("select distinct last_insert_rowid() from orbit"))
	{
		qDebug() << "select insert id false!" << query.lastError().text();
		return;
	}
	if (query.next())
	{
		orbit->id = query.value(0).toInt();
		StaticValue::GetInstance()->m_show_orbit[orbit->id] = orbit;
		resetTreeView();
	}
}

void QSubTreeView::newOutPut()
{
	QAction* ActionFrom = qobject_cast<QAction*>(sender());
	int waynum = ActionFrom->data().toInt();
	//    QMessageBox::information(this,"点击信息",QString("%1").arg(ActionFrom->data().toString()));
	bool bReturn = false;
	if (QSqlDatabase::database().transaction())
	{
		int size = StaticValue::GetInstance()->m_output_device.size();
		QString qStrName = tr("%1路开关量%2").arg(waynum).arg(size + 1);
		qDebug() << "new OutPut";
		QString qStrSql = QString("insert into output_device(name,type,ipAddress,port)"
			" values('%1','DO64Ethenet','192.168.1.11','6000')").
			arg(qStrName);
		QSqlQuery query;
		if (!query.exec(qStrSql))
		{
			qDebug() << "add output_device false!" << query.lastError().text();
			goto END;
		}
		if (!query.exec("select distinct last_insert_rowid() from output_device"))
		{
			qDebug() << "select output_device false!" << query.lastError().text();
			goto END;
		}

		if (query.next())
		{
			int output_device_id = query.value(0).toInt();
			OutPutDevice* out_put_device = new OutPutDevice();
			out_put_device->id = output_device_id;
			out_put_device->name = qStrName;
			out_put_device->type = "DO64Ethenet";
			out_put_device->ipAddress = "192.168.1.11";
			out_put_device->port = 6000;
			out_put_device->slotNum = waynum;
			for (int i = 0; i < waynum; ++i)
			{
				DeviceInfo* device_info = new DeviceInfo();
				device_info->elec_device_id = i;
				device_info->output_device_id = output_device_id;
				device_info->name = tr("阀");
				device_info->type = tr("水帘阀");
				qStrSql = QString("insert into device_info(elec_device_id,"
					"output_device_id,name,type) "
					"values(%1,%2,'%3','%4')").arg(device_info->elec_device_id).
					arg(device_info->output_device_id).arg(device_info->name).arg(device_info->type);
				if (!query.exec(qStrSql))
				{
					qDebug() << "add output_device false!" << query.lastError().text();
					goto END;
				}
				if (!query.exec("select distinct last_insert_rowid() from device_info"))
				{
					qDebug() << "select output_device false!" << query.lastError().text();
					goto END;
				}
				if (query.next())
				{
					device_info->id = query.value(0).toInt();
					StaticValue::GetInstance()->m_device_map[device_info->id] = device_info;
					out_put_device->m_mapping[i] = device_info->id;
				}
			}
			StaticValue::GetInstance()->m_output_device[out_put_device->id] = out_put_device;

			resetTreeView();
			bReturn = true;
		}
	}
END:
	if (bReturn)
	{
		bool bCommit = QSqlDatabase::database().commit();
		if (bCommit)
			qDebug() << tr("提交灯光设备成功");
		else
			qDebug() << tr("提交灯光设备失败");
		resetTreeView();
	}
	else
	{
		bool rollback = QSqlDatabase::database().rollback();
		if (rollback)
			qDebug() << tr("回滚灯光设备插入操作成功");
		else
			qDebug() << tr("回滚灯光设备插入操作失败");
	}

}

//add 标准DMX512 light
void QSubTreeView::newDMX()
{
	bool bReturn = false;
	if (QSqlDatabase::database().transaction())
	{
		//获得新板卡的信息，name、type、IPaddr、port
		OutPutDevice* out_put_Device = new OutPutDevice();
		out_put_Device->name = tr("标准DMX512灯光");
		out_put_Device->type = tr("DMX512");
		out_put_Device->ipAddress = tr("192.168.1.13");
		out_put_Device->port = 6000;

		//插入添加到数据库的output_device项
		QString qStrSql = QString("insert into output_device(name,type,"
			"ipAddress,port) values('%1','%2','%3','%4')")
			.arg(out_put_Device->name).arg(out_put_Device->type).
			arg(out_put_Device->ipAddress).arg(out_put_Device->port);
		QSqlQuery query;
		if (!query.exec(qStrSql))
		{
			qDebug() << "new output_device false!" << query.lastError().text();
			goto END;
		}

		//获取最后一次插入的号码
		if (!query.exec("select distinct last_insert_rowid() from output_device"))
		{
			qDebug() << "new output_device false!" << query.lastError().text();
			goto END;
		}
		int output_device_id = 0;
		if (!query.next())
		{
			qDebug() << "new output_device false!" << query.lastError().text();
			goto END;
		}

		output_device_id = query.value(0).toInt();			//获得板卡序列号，最后插入的output_device号码
		StaticValue::GetInstance()->m_output_device[output_device_id] = out_put_Device;

		//新建输出设备
		int route_id = 0;
		//从配置文件中读取灯光通道数量定义，不是一个好的办法，应该从创建之初让用户选择
		int iRouteNum = StaticValue::GetInstance()->m_SysConfig.value("DMXRouteNum").toInt();	//从文件获得灯通道数，一般是4
		if (iRouteNum <= 0)
			iRouteNum = 3;
		//del by guo 0906 int iTotal = 512 % iRouteNum == 0 ? 512 / iRouteNum : 512 / iRouteNum + 1;
		int iTotal = 512 % iRouteNum == 0 ? 512 / iRouteNum : 512 / iRouteNum;		//计算iTotal的值，一个域灯的数量

		//循环添加DMX灯设备，分别添加到device_info项、route_mapping项
		for (int i = 0; i < iTotal; ++i)
		{
			DeviceInfo* device_info = new DeviceInfo();
			device_info->output_device_id = output_device_id;
			device_info->name = "DMX512";
			device_info->type = "DMX512";
			qStrSql = QString("insert into device_info(output_device_id,"		//把板卡信息存储到数据库的device_info里面
				"name,type) values(%1,'%2','%3')").
				arg(device_info->output_device_id).arg(device_info->name).arg(device_info->type);
			if (!query.exec(qStrSql))
			{
				qDebug() << "insert device_info false" <<
					query.lastError().text();
				goto END;
			}
			int device_id = 0;
			if (!query.exec("select distinct last_insert_rowid() from device_info"))	//用于获取当前数据库连接中最后一次插入操作生成的自增ID，并确保结果唯一
			{
				qDebug() << "new output_device false!" << query.lastError().text();
				goto END;
			}
			if (!query.next())
			{
				qDebug() << "new output_device false!" << query.lastError().text();
				goto END;
			}
			device_id = query.value(0).toInt();		//获得新的设备号，最后的device_info号码
			StaticValue::GetInstance()->m_device_map[device_id] = device_info;

			//每一个电气设备增加三个通道
			for (int j = 0; j < iRouteNum; ++j)
			{
				//把output_device_id是板卡序列号，device_id是灯设备号，route_id是通道号，三个信息添加写入数据库route_mapping项
				qStrSql = QString("insert into route_mapping(id,device_id,output_device_id) values(%1,%2,%3)").
					arg(route_id).arg(device_id).arg(output_device_id);		//output_device_id是板卡序列号，device_id是灯设备号，route_id是通道号
				if (!query.exec(qStrSql))
				{
					qDebug() << "new route_mapping false!" << query.lastError().text();
					goto END;
				}
				StaticValue::GetInstance()->
					m_route_mapping.insert(device_id, route_id);	//建立灯设备号device_id到通道号route_id的映射
				route_id++;
				if (route_id >= 512)
					break;
			}
			if (route_id >= 512)
				break;
		}
		bReturn = true;
	}
END:
	if (bReturn)
	{
		bool bCommit = QSqlDatabase::database().commit();
		if (bCommit)
			qDebug() << tr("提交灯光设备成功");
		else
			qDebug() << tr("提交灯光设备失败");
		resetTreeView();
	}
	else
	{
		bool rollback = QSqlDatabase::database().rollback();
		if (rollback)
			qDebug() << tr("回滚灯光设备插入操作成功");
		else
			qDebug() << tr("回滚灯光设备插入操作失败");
	}
}

void QSubTreeView::newSL32()
{
	int waynum = 32;
	bool bReturn = false;
	QString sIpAddr = "192.168.1.";
	QString sIpAddrTemp = "";

	if (QSqlDatabase::database().transaction())
	{
		int size = StaticValue::GetInstance()->m_output_device.size();
		QString qStrName = tr("%1路开关量%2").arg(waynum).arg(size + 1);
//		QString qStrSql = QString("insert into output_device(name,type,ipAddress,port)"
//			" values('%1','SL32','192.168.1.11','6000')").
//			arg(qStrName);

		sIpAddrTemp = QString::number(size + 3);
		sIpAddr = "192.168.1." + sIpAddrTemp;

		QString qStrSql = QString("insert into output_device(name,type,ipAddress,port)"
				" values('%1','SL32','%2','6000')").
				arg(qStrName).arg(sIpAddr);

		QSqlQuery query;
		if (!query.exec(qStrSql))
		{
			qDebug() << "add output_device false!" << query.lastError().text();
			goto END;
		}
		if (!query.exec("select distinct last_insert_rowid() from output_device"))
		{
			qDebug() << "select output_device false!" << query.lastError().text();
			goto END;
		}

		if (query.next())
		{
			int output_device_id = query.value(0).toInt();
			OutPutDevice* out_put_device = new OutPutDevice();
			out_put_device->id = output_device_id;
			out_put_device->name = qStrName;
			out_put_device->type = "SL32";
		//	out_put_device->ipAddress = "192.168.1.11";
			out_put_device->ipAddress = sIpAddr;

			out_put_device->port = 6000;
			out_put_device->slotNum = waynum;
			for (int i = 0; i < waynum; ++i)
			{
				DeviceInfo* device_info = new DeviceInfo();
				device_info->elec_device_id = i;
				device_info->output_device_id = output_device_id;
				device_info->name = tr("阀");
				device_info->type = tr("水帘阀");
				device_info->delayinms = i + 1;										//创建的时候缺省的延时值

				qStrSql = QString("insert into device_info(elec_device_id,"			//
					"output_device_id,name,type,DelayInms) "
					"values(%1,%2,'%3','%4','%5')").arg(device_info->elec_device_id).
					arg(device_info->output_device_id).arg(device_info->name).arg(device_info->type).arg(device_info->delayinms);
				if (!query.exec(qStrSql))
				{
					qDebug() << "add output_device false!" << query.lastError().text();
					goto END;
				}
				if (!query.exec("select distinct last_insert_rowid() from device_info"))
				{
					qDebug() << "select output_device false!" << query.lastError().text();
					goto END;
				}
				if (query.next())
				{
					device_info->id = query.value(0).toInt();
					StaticValue::GetInstance()->m_device_map[device_info->id] = device_info;
					out_put_device->m_mapping[i] = device_info->id;

					out_put_device->m_delay_ms[i] = device_info->delayinms;			//延时值，缺省初值都是0
				}
			}
			StaticValue::GetInstance()->m_output_device[out_put_device->id] = out_put_device;

			resetTreeView();
			bReturn = true;
		}
	}
END:
	if (bReturn)
	{
		bool bCommit = QSqlDatabase::database().commit();
		if (bCommit)
			qDebug() << tr("提交灯光设备成功");
		else
			qDebug() << tr("提交灯光设备失败");
		resetTreeView();
	}
	else
	{
		bool rollback = QSqlDatabase::database().rollback();
		if (rollback)
			qDebug() << tr("回滚灯光设备插入操作成功");
		else
			qDebug() << tr("回滚灯光设备插入操作失败");
	}
}

void QSubTreeView::newSL40()
{
	int waynum = 40;
	bool bReturn = false;
	QString sIpAddr = "192.168.1.";
	QString sIpAddrTemp = "";

	if (QSqlDatabase::database().transaction())
	{
		int size = StaticValue::GetInstance()->m_output_device.size();		//获得已有板卡的数量，好从后面添加

		//获得名称，譬如：'40路开关量3'
		QString qStrName = tr("%1路开关量%2").arg(waynum).arg(size + 1);	//获得卡名字，前一个基础上+1
		//		QString qStrSql = QString("insert into output_device(name,type,ipAddress,port)"
		//			" values('%1','SL40','192.168.1.11','6000')").
		//			arg(qStrName);

		//获得网络地址
		sIpAddrTemp = QString::number(size + 3);	//偏移量3，从3开始
		sIpAddr = "192.168.1." + sIpAddrTemp;

		//获得完整信息，是个字符串，譬如：'40路开关量3','SL40','192.168.1.5','6000'
		QString qStrSql = QString("insert into output_device(name,type,ipAddress,port)"
			" values('%1','SL40','%2','6000')").
			arg(qStrName).arg(sIpAddr);

		QSqlQuery query;
		if (!query.exec(qStrSql))
		{
			qDebug() << "add output_device false!" << query.lastError().text();
			goto END;
		}
		if (!query.exec("select distinct last_insert_rowid() from output_device"))
		{
			qDebug() << "select output_device false!" << query.lastError().text();
			goto END;
		}

		//query.next()这一句
		if (query.next())		//query.next()，不理解意思
		{
			int output_device_id = query.value(0).toInt();			//板卡号码
			OutPutDevice* out_put_device = new OutPutDevice();		//声明一个新的OutPutDevice
			out_put_device->id = output_device_id;					//板卡id号
			out_put_device->name = qStrName;						//板卡名称
			out_put_device->type = "SL40";							//板卡类型
			//	out_put_device->ipAddress = "192.168.1.11";
			out_put_device->ipAddress = sIpAddr;					//板卡IP地址

			out_put_device->port = 6000;							//板卡端口号
			out_put_device->slotNum = waynum;						//板卡槽号，40
			//const int  Valve_Num =  120;
			for (int i = 0; i <  waynum; ++i)						//根据槽号设置子变量
			{
				DeviceInfo* device_info = new DeviceInfo();
				device_info->elec_device_id = i;
				device_info->output_device_id = output_device_id;
				device_info->name = tr("阀");					//名称：阀
				device_info->type = tr("水帘阀");				//类型：水帘阀

				
				qStrSql = QString("insert into device_info(elec_device_id,"			//
					"output_device_id,name,type,DelayInms) "
					"values(%1,%2,'%3','%4','%5')").arg(device_info->elec_device_id).
					arg(device_info->output_device_id).arg(device_info->name).arg(device_info->type).arg(device_info->delayinms);
				if (!query.exec(qStrSql))
				{
					qDebug() << "add output_device false!" << query.lastError().text();
					goto END;
				}
				if (!query.exec("select distinct last_insert_rowid() from device_info"))
				{
					qDebug() << "select output_device false!" << query.lastError().text();
					goto END;
				}
				if (query.next())
				{
					device_info->id = query.value(0).toInt();									//设备号，第三个板子的第一个设备，81 //anzs
					StaticValue::GetInstance()->m_device_map[device_info->id] = device_info;	//映射
					out_put_device->m_mapping[i] = device_info->id;								//映射到m_mapping



					////anzs code for delay time for each valve
					const float Arc_Length = 3.0f;
					const float Arc_Height = 2.0f;
					const int   Valve_Num = 120;
					const float Tank_Height = 0.36;
					const float g = 9.8;
					float flope = .0f;
					const float deltaX = Arc_Length / (Valve_Num );
					float height = .0f;
					
					if (device_info->id <= Valve_Num / 2)
					{
						flope = (Arc_Height - Tank_Height) / (Arc_Length / 2); // 斜率，用斜线拟合弧形 deltaY/deltaX
						height = Tank_Height + flope * deltaX * (device_info->id);
					}
					else
					if (device_info->id > Valve_Num / 2)
					{
						flope = (Tank_Height - Arc_Height ) / (Arc_Length/2   );
						height = Arc_Height + flope * deltaX * (device_info->id - Valve_Num / 2);
					}		
																

					float t = sqrt(2 * height / g);//自由落体到地面的时间 t = sqrt(2*height/g)

					device_info->delayinms = round(t * 1000);//i+1;										//创建的时候缺省的延时值





					out_put_device->m_delay_ms[i] = device_info->delayinms;			//延时值，缺省初值都是0
				}
			}
			StaticValue::GetInstance()->m_output_device[out_put_device->id] = out_put_device;

			resetTreeView();
			bReturn = true;
		}
	}
END:
	if (bReturn)
	{
		bool bCommit = QSqlDatabase::database().commit();
		if (bCommit)
			qDebug() << tr("提交灯光设备成功");
		else
			qDebug() << tr("提交灯光设备失败");
		resetTreeView();
	}
	else
	{
		bool rollback = QSqlDatabase::database().rollback();
		if (rollback)
			qDebug() << tr("回滚灯光设备插入操作成功");
		else
			qDebug() << tr("回滚灯光设备插入操作失败");
	}
}

void QSubTreeView::newCanMain()
{
	int waynum = 800;					//800路阀
	bool bReturn = false;
	QString sIpAddr = "192.168.1.";
	QString sIpAddrTemp = "";

	if (QSqlDatabase::database().transaction())
	{
		int size = StaticValue::GetInstance()->m_output_device.size();		//获得已有板卡的数量，好从后面添加

		//获得名称，譬如：'40路开关量3'
		QString qStrName = tr("%1路CAN主控设备%2").arg(waynum).arg(size + 1);	//获得卡名字，前一个基础上+1
		//		QString qStrSql = QString("insert into output_device(name,type,ipAddress,port)"
		//			" values('%1','SL40','192.168.1.11','6000')").
		//			arg(qStrName);

		//获得网络地址
		sIpAddrTemp = QString::number(size + 3);	//偏移量3，从3开始
		sIpAddr = "192.168.1." + sIpAddrTemp;

		//获得完整信息，是个字符串，譬如：'40路开关量3','SL40','192.168.1.5','6000'
		QString qStrSql = QString("insert into output_device(name,type,ipAddress,port)"
			" values('%1','CanMain','%2','6000')").
			arg(qStrName).arg(sIpAddr);

		QSqlQuery query;
		if (!query.exec(qStrSql))
		{
			qDebug() << "add output_device false!" << query.lastError().text();
			goto END;
		}
		if (!query.exec("select distinct last_insert_rowid() from output_device"))
		{
			qDebug() << "select output_device false!" << query.lastError().text();
			goto END;
		}

		//query.next()这一句
		if (query.next())		//query.next()，不理解意思
		{
			int output_device_id = query.value(0).toInt();			//板卡号码
			OutPutDevice* out_put_device = new OutPutDevice();		//声明一个新的OutPutDevice
			out_put_device->id = output_device_id;					//板卡id号
			out_put_device->name = qStrName;						//板卡名称
			out_put_device->type = "CanMain";						//板卡类型
			//	out_put_device->ipAddress = "192.168.1.11";
			out_put_device->ipAddress = sIpAddr;					//板卡IP地址

			out_put_device->port = 6000;							//板卡端口号
			out_put_device->slotNum = waynum;						//板卡槽号，40
			for (int i = 0; i < waynum; ++i)						//根据槽号设置子变量
			{
				DeviceInfo* device_info = new DeviceInfo();
				device_info->elec_device_id = i;
				device_info->output_device_id = output_device_id;
				device_info->name = tr("阀");					//名称：阀
				device_info->type = tr("水帘阀");				//类型：水帘阀
				device_info->delayinms = i + 1;										//创建的时候缺省的延时值

				qStrSql = QString("insert into device_info(elec_device_id,"			//
					"output_device_id,name,type,DelayInms) "
					"values(%1,%2,'%3','%4','%5')").arg(device_info->elec_device_id).
					arg(device_info->output_device_id).arg(device_info->name).arg(device_info->type).arg(device_info->delayinms);
				if (!query.exec(qStrSql))
				{
					qDebug() << "add output_device false!" << query.lastError().text();
					goto END;
				}
				if (!query.exec("select distinct last_insert_rowid() from device_info"))
				{
					qDebug() << "select output_device false!" << query.lastError().text();
					goto END;
				}
				if (query.next())
				{
					device_info->id = query.value(0).toInt();									//设备号，第三个板子的第一个设备，81
					StaticValue::GetInstance()->m_device_map[device_info->id] = device_info;	//映射
					out_put_device->m_mapping[i] = device_info->id;								//映射到m_mapping

					out_put_device->m_delay_ms[i] = device_info->delayinms;			//延时值，缺省初值都是0
				}
			}

			bReturn = true;

			StaticValue::GetInstance()->m_output_device[out_put_device->id] = out_put_device;

			resetTreeView();
			bReturn = true;
		}
	}
END:
	if (bReturn)
	{
		bool bCommit = QSqlDatabase::database().commit();
		if (bCommit)
			qDebug() << tr("提交灯光设备成功");
		else
			qDebug() << tr("提交灯光设备失败");
		resetTreeView();
	}
	else
	{
		bool rollback = QSqlDatabase::database().rollback();
		if (rollback)
			qDebug() << tr("回滚灯光设备插入操作成功");
		else
			qDebug() << tr("回滚灯光设备插入操作失败");
	}
}

//添加TTL长灯光设备，1920
void QSubTreeView::newTTL()
{
	bool bReturn = false;
	if (QSqlDatabase::database().transaction())
	{
		//获得新板卡的信息，name、type、IPaddr、port
		OutPutDevice* out_put_Device = new OutPutDevice();
		out_put_Device->name = tr("TTL长灯光");
		out_put_Device->type = tr("DMX512");
		out_put_Device->ipAddress = tr("192.168.1.13");
		out_put_Device->port = 6000;

		//插入添加到数据库的output_device项
		QString qStrSql = QString("insert into output_device(name,type,"
			"ipAddress,port) values('%1','%2','%3','%4')")
			.arg(out_put_Device->name).arg(out_put_Device->type).
			arg(out_put_Device->ipAddress).arg(out_put_Device->port);
		QSqlQuery query;
		if (!query.exec(qStrSql))
		{
			qDebug() << "new output_device false!" << query.lastError().text();
			goto END;
		}

		//获取最后一次插入的号码
		if (!query.exec("select distinct last_insert_rowid() from output_device"))
		{
			qDebug() << "new output_device false!" << query.lastError().text();
			goto END;
		}
		int output_device_id = 0;
		if (!query.next())
		{
			qDebug() << "new output_device false!" << query.lastError().text();
			goto END;
		}

		output_device_id = query.value(0).toInt();			//获得板卡序列号，最后插入的output_device号码
		StaticValue::GetInstance()->m_output_device[output_device_id] = out_put_Device;

		//新建输出设备
		int route_id = 0;
		//从配置文件中读取灯光通道数量定义，不是一个好的办法，应该从创建之初让用户选择
		int iRouteNum = StaticValue::GetInstance()->m_SysConfig.value("DMXRouteNum").toInt();	//从文件获得灯通道数，一般是4
		if (iRouteNum <= 0)
			iRouteNum = 3;
		//del by guo 0906 int iTotal = 512 % iRouteNum == 0 ? 512 / iRouteNum : 512 / iRouteNum + 1;
		int iTotal = 1920 % iRouteNum == 0 ? 1920 / iRouteNum : 1920 / iRouteNum;		//计算iTotal的值，一个域灯的数量

		//循环添加DMX灯设备，分别添加到device_info项、route_mapping项
		for (int i = 0; i < iTotal; ++i)
		{
			DeviceInfo* device_info = new DeviceInfo();
			device_info->output_device_id = output_device_id;
			device_info->name = "DMX512";
			device_info->type = "DMX512";
			qStrSql = QString("insert into device_info(output_device_id,"		//把板卡信息存储到数据库的device_info里面
				"name,type) values(%1,'%2','%3')").
				arg(device_info->output_device_id).arg(device_info->name).arg(device_info->type);
			if (!query.exec(qStrSql))
			{
				qDebug() << "insert device_info false" <<
					query.lastError().text();
				goto END;
			}
			int device_id = 0;
			if (!query.exec("select distinct last_insert_rowid() from device_info"))	//用于获取当前数据库连接中最后一次插入操作生成的自增ID，并确保结果唯一
			{
				qDebug() << "new output_device false!" << query.lastError().text();
				goto END;
			}
			if (!query.next())
			{
				qDebug() << "new output_device false!" << query.lastError().text();
				goto END;
			}
			device_id = query.value(0).toInt();		//获得新的设备号，最后的device_info号码
			StaticValue::GetInstance()->m_device_map[device_id] = device_info;

			//每一个电气设备增加三个通道
			for (int j = 0; j < iRouteNum; ++j)
			{
				//把output_device_id是板卡序列号，device_id是灯设备号，route_id是通道号，三个信息添加写入数据库route_mapping项
				qStrSql = QString("insert into route_mapping(id,device_id,output_device_id) values(%1,%2,%3)").
					arg(route_id).arg(device_id).arg(output_device_id);		//output_device_id是板卡序列号，device_id是灯设备号，route_id是通道号
				if (!query.exec(qStrSql))
				{
					qDebug() << "new route_mapping false!" << query.lastError().text();
					goto END;
				}
				StaticValue::GetInstance()->
					m_route_mapping.insert(device_id, route_id);	//建立灯设备号device_id到通道号route_id的映射
				route_id++;
				if (route_id >= 1920)
					break;
			}
			if (route_id >= 1920)
				break;
		}
		bReturn = true;
	}
END:
	if (bReturn)
	{
		bool bCommit = QSqlDatabase::database().commit();
		if (bCommit)
			qDebug() << tr("提交灯光设备成功");
		else
			qDebug() << tr("提交灯光设备失败");
		resetTreeView();
	}
	else
	{
		bool rollback = QSqlDatabase::database().rollback();
		if (rollback)
			qDebug() << tr("回滚灯光设备插入操作成功");
		else
			qDebug() << tr("回滚灯光设备插入操作失败");
	}
}

void QSubTreeView::newShowTeam()
{
	qDebug() << "new ShowTeam";
	QString qStrName(tr("空编队"));
	QString qStrSql = QString("insert into show_team(name,minNum,maxNum,"
		"arrayNum) values('%1',%2,%3,%4)").
		arg(qStrName).arg(1).arg(1).arg(1);
	QSqlQuery query;
	if (!query.exec(qStrSql))
	{
		qDebug() << "add show_team false!" << query.lastError().text();
		return;
	}
	if (!query.exec("select distinct last_insert_rowid() from show_team"))
	{
		qDebug() << "select show_team false!" << query.lastError().text();
		return;
	}
	if (query.next())
	{
		ShowTeam* show_team = new ShowTeam();
		show_team->id = query.value(0).toInt();
		show_team->name = qStrName;
		show_team->minNum = 1;
		show_team->maxNum = 1;
		show_team->arrayNum = 1;
		StaticValue::GetInstance()->m_show_team[show_team->id] = show_team;
		resetTreeView();
	}
}

void QSubTreeView::newMusic()
{
	emit addMusic();
}

void QSubTreeView::newProgramControl()
{
	emit addProgramControl();
}

void QSubTreeView::newQueryMusic()
{
	emit addQueryMusic();
}

void QSubTreeView::deleteItem()
{
	bool bReturn = false;
	int id = m_pCurrentItem->GetItemID();
	if (m_pCurrentItem->getType() == ORBIT_DEVICE)
	{
		bReturn = deleteOrbit(id);
	}
	else if (m_pCurrentItem->getType() == OUTPUT_DEVICE)
	{
		bReturn = deleteOutPutDevice(id);
	}
	else if (m_pCurrentItem->getType() == SHOWTEAMS_DEVICE)
	{
		bReturn = deleteShowTeam(id);
	}
	else if (m_pCurrentItem->getType() == COMP_DEVICE)
	{
		bReturn = deleteComp(id);
	}

	if (bReturn)
	{
		resetTreeView();
		QSqlQuery query;
		if (!query.exec("delete from sqlite_sequence"))
		{
			qDebug() << "delete sqlite_sequence false!" << query.lastError().text();
			return;
		}
	}
}

void QSubTreeView::manualAssignmentId()
{
	QList<int> list;

	bool bReturn = false;
	int id = m_pCurrentItem->GetItemID();

	if (m_pCurrentItem->getType() == COMP_DEVICE)
	{
		EditIDDialog editid_dialog;
		editid_dialog.setModal(true);
		editid_dialog.setData(id);
		if (QDialog::Accepted == editid_dialog.exec())
		{
			int new_id = editid_dialog.m_iNewID;
			QString qStrSql = QString("UPDATE music SET id = %1 WHERE id = %2").arg(new_id).arg(id);
			QSqlQuery query;
			if (!query.exec(qStrSql))
			{
				qDebug() << "UPDATE music id false!" << query.lastError().text();
			}
			else
			{	//修改music成功，继续修改music_mapping
				QString qStrSql = QString("UPDATE music_mapping SET music_id = %1 WHERE music_id = %2").arg(new_id).arg(id);
				QSqlQuery query;
				if (!query.exec(qStrSql))
				{
					qDebug() << "UPDATE music_mapping music_id false!" << query.lastError().text();
				}
				else
					QMessageBox::information(this, "警告", "修改music和music_mapping项成功，需重启软件！");

			/*	QString name = StaticValue::GetInstance()->m_music_info[id]->name;
				StaticValue::GetInstance()->m_music_info.remove(id);
				for (int i = 0; i < list.size(); ++i)
				{
					StaticValue::GetInstance()->m_music_action.remove(list.at(i));
					StaticValue::GetInstance()->m_MusicActions.remove(list.at(i));
				} */
			}

		//	QMessageBox::information(this, "警告", "确定！");
		}
		else
		{
		//	QMessageBox::information(this, "警告", "取消！");
		}
	//	addAction(m_NewSL32);
	//	bReturn = deleteComp(id);
	}

/*	if (bReturn)
	{
		resetTreeView();
		QSqlQuery query;
		if (!query.exec("delete from sqlite_sequence"))
		{
			qDebug() << "delete sqlite_sequence false!" << query.lastError().text();
			return;
		}
	} */
}

bool QSubTreeView::deleteOrbit(int id)
{
	QString qStrSql = QString("delete from orbit where id = %1").arg(id);
	QSqlQuery query;
	if (!query.exec(qStrSql))
	{
		qDebug() << "delete orbit false!" << query.lastError().text();
		return false;
	}
	qStrSql = QString("delete from time_orbit where orbit_id = %1").arg(id);
	if (!query.exec(qStrSql))
	{
		qDebug() << "delete time_orbit false!" << query.lastError().text();
		return false;
	}
	StaticValue::GetInstance()->m_show_orbit.remove(id);
	return true;
}

bool QSubTreeView::deleteOutPutDevice(int id)
{
	QString qStrSql = QString("delete from output_device where id = %1").arg(id);
	QSqlQuery query;
	if (!query.exec(qStrSql))
	{
		qDebug() << "delete output_device false!" << query.lastError().text();
		return false;
	}
	qStrSql = QString("delete from device_info where output_device_id = %1").arg(id);
	if (!query.exec(qStrSql))
	{
		qDebug() << "delete device_info false!" << query.lastError().text();
		return false;
	}
	qStrSql = QString("delete from route_mapping where output_device_id = %1").arg(id);
	if (!query.exec(qStrSql))
	{
		qDebug() << "delete route_mapping false!" << query.lastError().text();
		return false;
	}
	QMap<int, DeviceInfo*> device_map = StaticValue::GetInstance()->m_device_map;
	QList<int> keys = device_map.keys();
	for (int i = 0; i < keys.size(); ++i)
	{
		int device_id = keys.at(i);
		DeviceInfo* device_info = device_map[device_id];
		if (device_info->output_device_id == id)
			StaticValue::GetInstance()->m_device_map.remove(device_id);
		StaticValue::GetInstance()->m_route_mapping.remove(device_id);
	}
	StaticValue::GetInstance()->m_output_device.remove(id);
	return true;
}

bool QSubTreeView::deleteShowTeam(int id)
{
	QString qStrSql = QString("delete from show_team where id = %1").arg(id);
	QSqlQuery query;
	if (!query.exec(qStrSql))
	{
		qDebug() << "delete show_team false!" << query.lastError().text();
		return false;
	}
	StaticValue::GetInstance()->m_show_team.remove(id);
	return true;
}

bool QSubTreeView::deleteComp(int id)
{
	QList<int> list;
	bool bReturn = false;
	if (QMessageBox::Yes == QMessageBox::information(this,
		tr("提示"),
		tr("是否要删除此编曲文件？"),
		QMessageBox::Yes | QMessageBox::No))
	{
		if (QSqlDatabase::database().transaction())
		{
			QString qStrSql = QString("select music_action_id from"
				" music_mapping where music_id = %1").arg(id);
			QSqlQuery query;
			if (!query.exec(qStrSql))
			{
				qDebug() << "select music action id false!" << query.lastError().text();
				goto END;
			}

			while (query.next())
			{
				int music_action_id = query.value(0).toInt();
				list.push_back(music_action_id);
				qStrSql = QString("delete from music_action where id = %1").
					arg(music_action_id);
				if (!query.exec(qStrSql))
				{
					qDebug() << "delete music action false!" << query.lastError().text();
					goto END;
				}
			}
			qStrSql = QString("delete from music_mapping where music_id = %1").arg(id);
			if (!query.exec(qStrSql))
			{
				qDebug() << "delete music mapping false!" << query.lastError().text();
				goto END;
			}
			qStrSql = QString("delete from music where id = %1").arg(id);
			if (!query.exec(qStrSql))
			{
				qDebug() << "delete music file false!" << query.lastError().text();
				goto END;
			}
			bReturn = true;
		}
	END:
		if (bReturn)
		{
			bReturn = QSqlDatabase::database().commit();
			if (bReturn)
			{
				qDebug() << tr("删除编曲文件成功！");
				QString name = StaticValue::GetInstance()->m_music_info[id]->name;
				StaticValue::GetInstance()->m_music_info.remove(id);
				for (int i = 0; i < list.size(); ++i)
				{
					StaticValue::GetInstance()->m_music_action.remove(list.at(i));
					StaticValue::GetInstance()->m_MusicActions.remove(list.at(i));
				}
				emit deleteCompSignal(name);
			}
			else
				qDebug() << tr("删除编曲文件失败！");

		}

		if (!bReturn)
		{
			bReturn = QSqlDatabase::database().rollback();
			if (bReturn)
				qDebug() << tr("回滚删除编曲文件操作成功！");
			else
				qDebug() << tr("回滚删除编曲文件操作失败！");
		}
	}
	return bReturn;
}

void QSubTreeView::resetTreeView()
{
	this->setModel(new QTreeMode());
	this->collapseAll();
	this->expandToDepth(0);
}


