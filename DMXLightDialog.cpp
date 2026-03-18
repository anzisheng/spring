#include "DMXLightDialog.h"
#include "ui_DMXLightDialog.h"
#include <QTableWidgetItem>
#include "StaticValue.h"
#include "CommonFile.h"

#include "CollidingRectItem.h"
#include <QMessageBox>

//新建DMX编曲动作
DMXLightDialog::DMXLightDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMXLightDialog)
{
    ui->setupUi(this);

	m_iKeepTime = 1000;
	m_iDelayTime = 100;
	m_iPerialTime = 2000;
	m_iLoopTimes = -1;

	m_iOutputValue1 = 255;
	m_iOutputValue2 = 0;
	m_iStartDelay = 0;

	m_iFadeIn = 0;
	m_iFadeOut = 0;

	m_iReverseFlag = 0;
	m_iFromLeft = 0;

    ui->page_alwayson->show();
    ui->page_domino->hide();
    ui->page_fromto->hide();
    ui->page_track->hide();
    ui->page_rainbow->hide();

    ui->action_select->addItem("on");
    ui->action_select->addItem("domino");
    ui->action_select->addItem("track");
    ui->action_select->addItem("fromto");
    ui->action_select->addItem("rainbow");
    ui->action_select->setCurrentIndex(0);

    QStringList head_list;
    head_list << tr("通道号") <<
                 tr("时间长度") <<
                 tr("通道动作");

	//绘制显示框
    ui->route_table->setColumnCount(3);
    ui->route_table->setColumnWidth(0,50);
    ui->route_table->setColumnWidth(1,150);
    ui->route_table->setColumnWidth(2,360);			//
    ui->route_table->setAlternatingRowColors(true);
    ui->route_table->setHorizontalHeaderLabels(head_list);
    ui->route_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->route_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    int iRouteNum = StaticValue::GetInstance()->m_SysConfig.value("DMXRouteNum").toInt();
    if(iRouteNum <= 0)
        iRouteNum = 3;
    for(int i = 0; i < iRouteNum; ++i)
    {
        ui->route_table->insertRow(i);
        ui->route_table->setItem(i,0,new QTableWidgetItem(QString::number(i)));
        ui->route_table->setItem(i,1,new QTableWidgetItem("0"));
        ui->route_table->setItem(i,2,new QTableWidgetItem("on 0"));
    }

    ui->always_on_output->setText("0");
    ui->route_table->setFocus();
    ui->route_table->selectRow(0);		//选择焦点在第0行
    InitOrbitInfo();

//	setData(1000, 100, 2000, -1, 255, 0, 0, 0, 0);
//	void DMXLightDialog::setData(QMap<int, RouteInfo *> dmx_info)
	connect(ui->route_table,SIGNAL(doubleClicked(const QModelIndex & index)),this,SLOT(SetSelectedData(const QModelIndex & index)));
}

DMXLightDialog::~DMXLightDialog()
{
    delete ui;
}

//上面的方框里面填充字符串，不是下面的EDIT
void DMXLightDialog::setData(QMap<int, RouteInfo *> dmx_info)
{
    user_route_setting = dmx_info;
    QList<int> keys = dmx_info.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        RouteInfo* route_info = dmx_info[keys.at(i)];
        QString qStrValue("");

        if(route_info->type == DMX512_ALWAYSON)  //常开
        {
            qStrValue = QString("on %1").arg(route_info->out_value1);

			ui->action_select->setCurrentIndex(route_info->type);
        }
        else if(route_info->type == DMX512_DOMINO)  //多米诺
        {
			qStrValue = QString("domino %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11").
				arg(route_info->keep_time).arg(route_info->delay_time).arg(route_info->perial_time).
				arg(route_info->out_value1).arg(route_info->out_value2).arg(route_info->loop_time).
				arg(route_info->fade_in).arg(route_info->fade_out).arg(route_info->start_delay).
				arg(route_info->reverse_flag).arg(route_info->from_left);

			ui->action_select->setCurrentIndex(route_info->type);

			if (0 == route_info->id)
			{	//控件按照焦点所对的0行数据初始化下面的控件内容
				ui->KeepTime->setText(QString::number(route_info->keep_time));
				ui->DelayTime->setText(QString::number(route_info->delay_time));
				ui->PerialTime->setText(QString::number(route_info->perial_time));
				ui->OutValue1->setText(QString::number(route_info->out_value1));
				ui->OutValue2->setText(QString::number(route_info->out_value2));
				ui->LoopTimes->setText(QString::number(route_info->loop_time));
				ui->FadeIn->setText(QString::number(route_info->fade_in));
				ui->FadeOut->setText(QString::number(route_info->fade_out));
				ui->StartDelay->setText(QString::number(route_info->start_delay));

				if (route_info->reverse_flag)
					ui->ReverseFlag->setChecked(true);
				else
					ui->ReverseFlag->setChecked(false);

				if (route_info->from_left)
					ui->FromLeft->setChecked(true);
				else
					ui->FromLeft->setChecked(false);
			}
		}
        else if(route_info->type == DMX512_TRACK)  //轨迹
        {
            qStrValue = QString("track %1 %2 %3 %4").arg(route_info->orbit_id).
                    arg(route_info->loop_time).arg(route_info->delay_time).arg(route_info->start_delay);

			ui->action_select->setCurrentIndex(route_info->type);

        }
		else if (route_info->type == DMX512_FROMTO)  //从哪到哪
		{
			qStrValue = QString("fromto %1 %2 %3 %4 %5").arg(route_info->out_value1).
				arg(route_info->out_value2).arg(route_info->delay_time).
				arg(route_info->change_speed).arg(route_info->start_delay);

			ui->action_select->setCurrentIndex(route_info->type);
		}
		else if (route_info->type == DMX512_RAINBOW)  
        {
            qStrValue = QString("rainbow %1").arg(route_info->circle);

			ui->action_select->setCurrentIndex(route_info->type);
		}
        ui->route_table->item(i,2)->setText(qStrValue);
    }
}

void DMXLightDialog::InitOrbitInfo()
{
    ui->orbit_id->clear();
    QMap<int,Orbit*> orbit_list = StaticValue::GetInstance()->m_show_orbit;
    QList<int> keys = orbit_list.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        int id = keys.at(i);
        Orbit* orbit_info = orbit_list[id];
        QString name = orbit_info->name;
        QString qStrShow = QString::number(id) + "." + name;
        ui->orbit_id->addItem(qStrShow,id);
    }
}

//在对话框内点击上面的行
QVector<int> DMXLightDialog::getSelectRows()
{
    QVector<int> row_list;
    QList<QTableWidgetItem*> items = ui->route_table->selectedItems();
    int count = items.count();
    for(int i=0;i<count;i++)
    {
        int row = ui->route_table->row(items.at(i));//获取选中的行
        if(!row_list.contains(row))
            row_list.push_back(row);
    }
    return row_list;
}

//在DMX编辑对话框内选择下拉选择项
void DMXLightDialog::on_action_select_currentIndexChanged(const QString &arg1)
{
	//暂存数据

	//dmx dominuo
	m_iKeepTime = ui->KeepTime->text().toInt();
	m_iDelayTime = ui->DelayTime->text().toInt();
	m_iPerialTime = ui->PerialTime->text().toInt();
	m_iOutputValue1 = ui->OutValue1->text().toInt();
	m_iOutputValue2 = ui->OutValue2->text().toInt();
	m_iLoopTimes = ui->LoopTimes->text().toInt();
	m_iFadeIn = ui->FadeIn->text().toInt();
	m_iFadeOut = ui->FadeOut->text().toInt();
	m_iStartDelay = ui->StartDelay->text().toInt();
	if (ui->ReverseFlag->isChecked())
		m_iReverseFlag = true;
	else
		m_iReverseFlag = false;

	if (ui->FromLeft->isChecked())
		m_iFromLeft = true;
	else
		m_iFromLeft = false;

    if(arg1.compare("on") == 0)
    {
        ui->page_alwayson->show();
        ui->page_domino->hide();
        ui->page_fromto->hide();
        ui->page_track->hide();
        ui->page_rainbow->hide();

		ui->OutValue1->setText(QString::number(m_iOutputValue1));
    }
    else if(arg1.compare("domino") == 0)
    {
        ui->page_alwayson->hide();
        ui->page_domino->show();
        ui->page_fromto->hide();
        ui->page_track->hide();
        ui->page_rainbow->hide();

		ui->KeepTime->setText(QString::number(m_iKeepTime));
		ui->DelayTime->setText(QString::number(m_iDelayTime));
		ui->PerialTime->setText(QString::number(m_iPerialTime));

		ui->OutValue1->setText(QString::number(m_iOutputValue1));
		ui->OutValue2->setText(QString::number(m_iOutputValue2));
		ui->LoopTimes->setText(QString::number(m_iLoopTimes));

		ui->FadeIn->setText(QString::number(m_iFadeIn));
		ui->FadeOut->setText(QString::number(m_iFadeOut));
		ui->StartDelay->setText(QString::number(m_iStartDelay));

		if (ui->FromLeft->isChecked())
			m_iReverseFlag = true;
		else
			m_iReverseFlag = false;

		if (ui->FromLeft->isChecked())
			m_iFromLeft = true;
		else
			m_iFromLeft = false;
    }
    else if(arg1.compare("track") == 0)
    {
        ui->page_alwayson->hide();
        ui->page_domino->hide();
        ui->page_fromto->hide();
        ui->page_track->show();
        ui->page_rainbow->hide();
    }
    else if(arg1.compare("fromto") == 0)
    {
        ui->page_alwayson->hide();
        ui->page_domino->hide();
        ui->page_fromto->show();
        ui->page_track->hide();
        ui->page_rainbow->hide();
    }
    else if (arg1.compare("rainbow")==0)
    {
        ui->page_alwayson->hide();
        ui->page_domino->hide();
        ui->page_fromto->hide();
        ui->page_track->hide();
        ui->page_rainbow->show();
    }
}

//确定退出DMX编辑对话框,从对话框获取数据，是从上面的框获得数据
//数值都得从字符串来，不从下面的EDIT和Check等等控件来
void DMXLightDialog::on_buttonBox_accepted()                //确定
{
    int rowCount = ui->route_table->rowCount();
    for(int i = 0; i < rowCount; ++i)
    {	//item(i,0)，i是上面显示框二点行信息，后面是列信息
        QTableWidgetItem* item = ui->route_table->item(i,0);
        if(!item)
            continue;
        RouteInfo* route_info = new RouteInfo();
        route_info->id = ui->route_table->item(i,0)->text().toInt();
        //        QString qStrTime = ui->route_table->item(i,1)->text();
        QString qStrValue = ui->route_table->item(i,2)->text();		//获得第二列的字符串
        QStringList route_info_list = qStrValue.split(" ");			//去掉字符串内部空格，并且转换为数字
        if(route_info_list.isEmpty())
            continue;

        QString type = route_info_list.at(0);
        if(type.compare("on") == 0)
        {
            route_info->type = DMX512_ALWAYSON;
            route_info->out_value1 = route_info_list.at(1).trimmed().toInt();
        }
		else if (type.compare("domino") == 0)
		{
			route_info->type = DMX512_DOMINO;

			route_info->keep_time = route_info_list.at(1).trimmed().toInt();
			route_info->delay_time = route_info_list.at(2).trimmed().toInt();
			route_info->perial_time = route_info_list.at(3).trimmed().toInt();

			route_info->out_value1 = route_info_list.at(4).trimmed().toInt();
			route_info->out_value2 = route_info_list.at(5).trimmed().toInt();
			route_info->loop_time = route_info_list.at(6).trimmed().toInt();

			route_info->fade_in = route_info_list.at(7).trimmed().toInt();
			route_info->fade_out = route_info_list.at(8).trimmed().toInt();
			route_info->start_delay = route_info_list.at(9).trimmed().toInt();

			route_info->reverse_flag = route_info_list.at(10).trimmed().toInt();	//数值都得从字符串来，不从下面的EDIT和Check等等控件来
			route_info->from_left = route_info_list.at(11).trimmed().toInt();
        }
        else if(type.compare("track") == 0)
        {
            route_info->type = DMX512_TRACK;
            route_info->orbit_id = route_info_list.at(1).trimmed().toInt();
            route_info->loop_time = route_info_list.at(2).trimmed().toInt();
            route_info->delay_time = route_info_list.at(3).trimmed().toInt();
            route_info->start_delay = route_info_list.at(4).trimmed().toInt();
        }
        else if(type.compare("fromto") == 0)
        {
            route_info->type = DMX512_FROMTO;
            route_info->out_value1 = route_info_list.at(1).trimmed().toInt();
            route_info->out_value2 = route_info_list.at(2).trimmed().toInt();
            route_info->delay_time = route_info_list.at(3).trimmed().toInt();
            route_info->change_speed = route_info_list.at(4).trimmed().toInt();
            route_info->start_delay = route_info_list.at(5).trimmed().toInt();
        }
        else if (type.compare("rainbow")==0)
        {
            route_info->type = DMX512_RAINBOW;
            route_info->circle = route_info_list.at(1).trimmed().toInt();
        }
		m_route_setting[route_info->id] = route_info;
	//	user_route_setting[route_info->id] = route_info;

	//	CollidingRectItem::m_route_setting[route_info->id] = route_info;
    }
}

//应用当前通道的处理，从下面的EDIT、Check等控件获得数据，送到上面的数据框
void DMXLightDialog::on_submit_clicked()                //apply应用
{

    QString qStrValue("");
    int index = ui->action_select->currentIndex();
    if(index == DMX512_ALWAYSON)  //常开
    {
        int out_value1 = ui->always_on_output->text().toInt();
        qStrValue = QString("on %1").arg(out_value1);
    }
    else if(index == DMX512_DOMINO)  //多米诺
    {
		m_iKeepTime = ui->KeepTime->text().toInt();
		m_iDelayTime = ui->DelayTime->text().toInt();
		m_iPerialTime = ui->PerialTime->text().toInt();
		m_iOutputValue1 = ui->OutValue1->text().toInt();
		m_iOutputValue2 = ui->OutValue2->text().toInt();
		m_iLoopTimes = ui->LoopTimes->text().toInt();
		m_iFadeIn = ui->FadeIn->text().toInt();
		m_iFadeOut = ui->FadeOut->text().toInt();
		m_iStartDelay = ui->StartDelay->text().toInt();

		if (ui->ReverseFlag->isChecked())
			m_iReverseFlag = true;
		else
			m_iReverseFlag = false;

		if (ui->FromLeft->isChecked())
			m_iFromLeft = true;
		else
			m_iFromLeft = false;

		qStrValue = QString("domino %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11").
			arg(m_iKeepTime).arg(m_iDelayTime).arg(m_iPerialTime).
			arg(m_iOutputValue1).arg(m_iOutputValue2).arg(m_iLoopTimes).
			arg(m_iFadeIn).arg(m_iFadeOut).arg(m_iStartDelay).arg(m_iReverseFlag).arg(m_iFromLeft);
    }
    else if(index == DMX512_TRACK)  //轨迹
    {
        //选择轨迹的ID
        int orbit_id = ui->orbit_id->itemData(ui->orbit_id->currentIndex()).toInt();
        int loop_time = ui->orbit_loop_time->text().toInt();
        int delay_time = ui->orbit_delay_time->text().toInt();
        int start_delay = ui->orbit_start_delay->text().toInt();
        qStrValue = QString("track %1 %2 %3 %4").arg(orbit_id).
                arg(loop_time).arg(delay_time).arg(start_delay);
    }
    else if(index == DMX512_FROMTO)  //从哪到哪
    {
        int out_value1 = ui->fromto_out_value1->text().toInt();
        int out_value2 = ui->fromto_out_value2->text().toInt();
        int delay_time = ui->fromto_delay_time->text().toInt();
        int change_speed = ui->fromto_change_speed->text().toInt();
        int start_delay = ui->fromto_start_delay->text().toInt();
        qStrValue = QString("fromto %1 %2 %3 %4 %5").arg(out_value1).
                arg(out_value2).arg(delay_time).arg(change_speed).arg(start_delay);
    }
    else if (index== DMX512_RAINBOW)
    {
        int circle = ui->rainbow_circle->text().toInt();
        qStrValue = QString("rainbow %1").arg(circle);
    }
    QVector<int> row_list = getSelectRows();
    for(int i = 0 ; i < row_list.size(); ++i)
    {
        int index = row_list.at(i);
        ui->route_table->item(index,2)->setText(qStrValue);
    }
}

//点击DMX编辑对话框的上面显示的一行的对应函数，x是点击的行号，y没有用，估计是列？
void DMXLightDialog::cellClick(int x,int y)
{
   QList<int> keys = user_route_setting.keys();   //获得存在数据库里的dmx的数据
//    int selected_rows =ui->route_table->currentRow();
   if(keys.length()==0)
       return;
   if(keys.length()<x)
       return;
   RouteInfo* route_info = user_route_setting[keys.at(x)];   //获得点击的那一行的数据

//    QList<int> keys = dmx_info.keys();

    QString qStrValue("");
    if(route_info->type == DMX512_ALWAYSON)  //常开
    {
        ui->page_alwayson->show();
        ui->page_domino->hide();
        ui->page_fromto->hide();
        ui->page_track->hide();
        ui->action_select->setCurrentIndex(0);

        qStrValue = QString::number(route_info->out_value1);
        ui->always_on_output->setText(qStrValue);
    }
    else if(route_info->type == DMX512_DOMINO)  //多米诺
    {
        ui->page_alwayson->hide();
        ui->page_domino->show();
        ui->page_fromto->hide();
        ui->page_track->hide();
        ui->action_select->setCurrentIndex(1);

		qStrValue = QString::number(route_info->keep_time);
		ui->KeepTime->setText(qStrValue);				//KeepTime在UI里面定义

		qStrValue = QString::number(route_info->delay_time);
		ui->DelayTime->setText(qStrValue);

		qStrValue = QString::number(route_info->perial_time);
		ui->PerialTime->setText(qStrValue);

		qStrValue = QString::number(route_info->out_value1);
		ui->OutValue1->setText(qStrValue);

		qStrValue = QString::number(route_info->out_value2);
		ui->OutValue2->setText(qStrValue);

		qStrValue = QString::number(route_info->loop_time);
		ui->LoopTimes->setText(qStrValue);

		qStrValue = QString::number(route_info->fade_in);
		ui->FadeIn->setText(qStrValue);

		qStrValue = QString::number(route_info->fade_out);
		ui->FadeOut->setText(qStrValue);

		qStrValue = QString::number(route_info->start_delay);
		ui->StartDelay->setText(qStrValue);

		if (route_info->reverse_flag)
			ui->ReverseFlag->setChecked(true);
		else
			ui->ReverseFlag->setChecked(false);

		if (route_info->from_left)
			ui->FromLeft->setChecked(true);
		else
			ui->FromLeft->setChecked(false);
    }

    else if(route_info->type == DMX512_TRACK)  //轨迹
    {
        ui->page_alwayson->hide();
        ui->page_domino->hide();
        ui->page_fromto->hide();
        ui->page_track->show();
        ui->action_select->setCurrentIndex(2);

     //   qStrValue=QString::number(route_info->orbit_id);
        ui->orbit_id->setCurrentIndex(route_info->orbit_id-1);

        qStrValue=QString::number(route_info->loop_time);
        ui->orbit_loop_time->setText(qStrValue);

        qStrValue=QString::number(route_info->delay_time);
        ui->orbit_delay_time->setText(qStrValue);

        qStrValue=QString::number(route_info->start_delay);
        ui->orbit_start_delay->setText(qStrValue);
    }
    else if(route_info->type == DMX512_FROMTO)  //从哪到哪
    {
        ui->page_alwayson->hide();
        ui->page_domino->hide();
        ui->page_fromto->show();
        ui->page_track->hide();
        ui->action_select->setCurrentIndex(3);

        qStrValue=QString::number(route_info->out_value1);
        ui->fromto_out_value1->setText(qStrValue);

        qStrValue=QString::number(route_info->out_value2);
        ui->fromto_out_value2->setText(qStrValue);

        qStrValue=QString::number(route_info->delay_time);
        ui->fromto_delay_time->setText(qStrValue);

        qStrValue=QString::number(route_info->change_speed);
        ui->fromto_change_speed->setText(qStrValue);

        qStrValue=QString::number(route_info->start_delay);
        ui->fromto_start_delay->setText(qStrValue);
     }
	else if (route_info->type == DMX512_RAINBOW)  //从哪到
	{
		ui->page_alwayson->hide();
		ui->page_domino->hide();
		ui->page_fromto->hide();
		ui->page_track->hide();
        ui->page_rainbow->show();
		ui->action_select->setCurrentIndex(4);
        ui->rainbow_circle->setValue(route_info->circle);
    }
}
