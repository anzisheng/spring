#include "CollidingRectItem.h"
#include <QGraphicsScene>
#include <math.h>
#include <QGraphicsView>
#include "CommonFile.h"
#include "myitem.h"
#include "DominoSetting.h"
#include <QSqlError>
#include <QSqlQuery>
#include "SelectImageDialog.h"
#include "DMXLightDialog.h"
#include "ClockDialog.h"
#include <QDebug>
#include "MusicWidget.h"
#include "MusicGraphicsView.h"
#include <QMessageBox>

//打开的编曲如果偶编曲的片段，才会运行到这里，由多少个标曲的片段，这个调用多少次
//这里还不能作为打开文件的标志，因为如果打开的试空文件，不会执行到这里
//但是如果打开空编曲文件，也恶意按开始，不会崩溃
CollidingRectItem::CollidingRectItem(
        int type,QRectF rect,QColor color, QGraphicsItem* parent )
{
//    m_boundRect = rect;

    ResetRect(rect);
    m_rectColor = color;
    m_IsYangWen = true;

    m_cursor=new QCursor;
    setAcceptDrops(true);
    setFlag(QGraphicsItem::ItemIsMovable,false);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);

    MyItem* item = (MyItem*)parent;
    QStringList row_lists = item->m_Show_Team_list.split("\n");
    for(int i = 0; i < row_lists.size();++i)
    {
        QList<int> list;
        QString sub_teams = row_lists.at(i);
        QStringList sub_team_list = sub_teams.split(",");
        for(int j = 0; j < sub_team_list.size(); ++j)
        {
            list.append(sub_team_list.at(j).toInt());
        }
        m_show_teams.append(list);
    }

//    InitData();
	for (int i = 0; i < 6000; i++)
	{
		keep_time_flag[i]=0;   //保持中的标志
		keep_time[i]=0;		 //保持中的时间
	}

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 6000; j++)
		{
			dmx_keep_time_flag[i][j] = 0;   //保持中的标志
			dmx_keep_time[i][j] = 0;		 //保持中的时间
		}
	}

/*	QList<int> keys = m_route_setting.keys();
	for (int i = 0; i < keys.size(); ++i)
	{
		RouteInfo* route = m_route_setting[keys.at(i)];
	//	if (DMX512_ALWAYSON == route->type)
	//	{
	//		qStrTemp = QString("on %1").arg(route->out_value1);
	//	}
		for (int j = 0; j < 1000; i++)
		{
			route->dmx_keep_time_flag[i][j] = 0;   //保持中的标志
			route->dmx_keep_time[i][j] = 0;		 //保持中的时间
		}
	} */

    m_type = type;
    leftTimeTip_    =   new QGraphicsSimpleTextItem(this);
    rightTimeTip_   =   new QGraphicsSimpleTextItem(this);
    this->setParent(item);
}

void CollidingRectItem::InitData()
{

    m_orbit_id = 0;
    m_type = ALWAYS_ON;

//    m_out_value1 = 255;
//    m_out_value2 = 0;
    m_len1  =   2;
    m_len2  =   3;
    m_speed =   300;
    m_keep_time = 0;
    m_delay_time = 0;
    m_perial_time = 1;
    m_loop_time = -1;
//    m_start_delay = 0;
    m_change_speed = 0;

	m_out_value1 = 255;
	m_out_value2 = 0;
	m_start_delay = 0;

    cScale=false;
    direction=0;
    m_IsDeviceOn = false;
    m_ImagePath = "";
    m_IsShowing = false;

/*	for (int i = 0; i < 1000; i++)
	{
		keep_time_flag[i]=0;   //保持中的标志
		keep_time[i]=0;		 //保持中的时间
	} */
}

QRectF CollidingRectItem::boundingRect() const
{
    return m_boundRect;
}

void CollidingRectItem::paint(QPainter *painter,
                              const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    Q_UNUSED(widget);
    QRectF bound_rect = boundingRect();
    QBrush Brush(m_rectColor);
    //Qt::ContainsItemBoundingRect
    QList<QGraphicsItem*> item_list = scene()->collidingItems(this);
    if(item_list.size() == 1) //只和一个item有碰撞
    {
		//qDebug() << "只和一个item有碰撞";
        QGraphicsItem* pItem = item_list.at(0);
        if(pItem->zValue() == 0) //和父item有碰撞,
        {
            Brush.setColor(Qt::red);
            if(m_IsDeviceOn)
            {
                m_IsDeviceOn = false;
            }
        }
    }
    else if(item_list.size() == 2)
    {
        Brush.setColor(Qt::green);
        if(!m_IsDeviceOn)
        {
            //和两个item有碰撞而且没有发送过信号
			//qDebug() << "和两个item有碰撞而且没有发送过信号";
            QGraphicsItem* pItem1 = item_list.at(0);
            QGraphicsItem* pItem2 = item_list.at(1);

            //和时间轴、父item有碰撞
            if(pItem1->zValue() == 2 && pItem2->zValue() == 0)
            {
//                timer->start(iTimeOutNum);
                m_IsDeviceOn = true;
            }
            if(pItem1->zValue() == 0 && pItem2->zValue() == 2)
            {
//                timer->start(iTimeOutNum);
                m_IsDeviceOn = true;
            }
        }
    }

    QString show_text = getDrawText();			//获取字符串

	painter->setRenderHint(QPainter::Antialiasing, true);
	const qreal radius = 4.0;
	QPainterPath path;
	path.addRoundedRect(bound_rect, radius, radius);

	QColor baseColor = Brush.color();
	painter->setBrush(baseColor);
	painter->setPen(QPen(baseColor.darker(130)));
	painter->drawPath(path);

	const qreal handleWidth = 6.0;
	QRectF leftHandle(bound_rect.left(), bound_rect.top(), handleWidth, bound_rect.height());
	QRectF rightHandle(bound_rect.right() - handleWidth, bound_rect.top(), handleWidth, bound_rect.height());
	QColor handleColor = baseColor.darker(110);
	if (option && (option->state & QStyle::State_Selected))
	{
		handleColor = baseColor.lighter(120);
	}
	painter->setBrush(handleColor);
	painter->setPen(Qt::NoPen);
	painter->drawRoundedRect(leftHandle, 2.0, 2.0);
	painter->drawRoundedRect(rightHandle, 2.0, 2.0);

	QColor gripColor = handleColor.lighter(140);
	painter->setBrush(gripColor);
	painter->setPen(Qt::NoPen);
	const qreal dotRadius = 1.2;
	const qreal dotGap = 4.0;
	qreal centerY = bound_rect.center().y();
	for (int i = -1; i <= 1; ++i)
	{
		qreal cy = centerY + i * dotGap;
		QPointF leftCenter(leftHandle.center().x(), cy);
		QPointF rightCenter(rightHandle.center().x(), cy);
		painter->drawEllipse(leftCenter, dotRadius, dotRadius);
		painter->drawEllipse(rightCenter, dotRadius, dotRadius);
	}

	painter->setPen(QPen(baseColor.darker(150)));
	painter->drawText(bound_rect, show_text);

}

//获得编曲界面显示的编曲动作表示的字符串
QString CollidingRectItem::getDrawText()
{
   QString text;
   if(ALWAYS_ON == m_type)
    {
        text = QString("On %1").arg(m_out_value1);
    }
    else if(DOMINO_RUN == m_type)
    {
		text = QString("domino %1 %2 %3 %4 %5 %6 %7 %8 %9").
			arg(m_keep_time).arg(m_delay_time).arg(m_perial_time).arg(m_loop_time).arg(m_out_value1).arg(m_out_value2).arg(m_start_delay).
			arg(m_reverse_flag).arg(m_from_left);
    }
    else if(IMAGE_SHOW == m_type)
    {
        text = QString("image %1 %2").arg(m_delay_time).arg(m_loop_time);
    }
    else if(DMX512_SHOW == m_type)
    {
        text = "dmx ";
        QList<int> keys = m_route_setting.keys();
        for(int i = 0; i < keys.size(); ++i)
        {
            QString qStrTemp;
            RouteInfo* route = m_route_setting[keys.at(i)];
            if(DMX512_ALWAYSON == route->type)
            {
                qStrTemp = QString("on %1").arg(route->out_value1);
            }
            else if(DMX512_DOMINO == route->type)
            {
				qStrTemp = QString("domino %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11").
					arg(route->keep_time).arg(route->delay_time).arg(route->perial_time).
					arg(route->out_value1).arg(route->out_value2).arg(route->loop_time).
					arg(route->fade_in).arg(route->fade_out).arg(route->start_delay).
					arg(route->reverse_flag).arg(route->from_left);
            }
            else if(DMX512_TRACK == route->type)
            {
                qStrTemp = QString("track %1 %2 %3 %4").arg(route->orbit_id).arg(route->loop_time).
                        arg(route->delay_time).arg(route->start_delay);
            }
            else if(DMX512_FROMTO == route->type)
            {
                qStrTemp = QString("fromto %1 %2 %3 %4").arg(route->out_value1).arg(route->out_value2).
                        arg(route->delay_time).arg(route->change_speed).arg(route->start_delay);
            }
			else if (DMX512_RAINBOW == route->type)
			{
                qStrTemp = QString("rainbow %1").arg(route->circle);
			}
            text += qStrTemp;
            text += " ";
        }
    }
	else if (CLOCK_SHOW == m_type)
	{
	//	text = QString("image %1 %2").arg(m_delay_time).arg(m_loop_time);
		text = QString("clock %1 %2").arg(m_delay_time).arg(m_loop_time);
	}

    return text;

}

QPainterPath CollidingRectItem::shape()const{
    QPainterPath path;
    path.addRect(m_boundRect);
    return path;
}

bool CollidingRectItem::contains(const QPointF &point) const
{
	const qreal selectMargin = 4.0;
	QRectF hitRect = m_boundRect.adjusted(-selectMargin, 0.0, selectMargin, 0.0);
	return hitRect.contains(point);
}

void CollidingRectItem::ResetRect(QRectF rect){
    m_boundRect = rect;
    m_InnerRect = rect;                 //设置内层矩形，如果鼠标在内层矩形中，改变鼠标样式
	const qreal edgeWidth = 6.0;
	qreal innerLeft = rect.left() + edgeWidth;
	qreal innerRight = rect.right() - edgeWidth;
	if (innerRight <= innerLeft)
	{
		innerLeft = rect.left() + 2.0;
		innerRight = rect.right() - 2.0;
		if (innerRight <= innerLeft)
		{
			innerLeft = rect.left();
			innerRight = rect.right();
		}
	}
    m_InnerRect.setLeft(innerLeft);
    m_InnerRect.setRight(innerRight);
    update(boundingRect());
}

void CollidingRectItem::setscaleRect(QRectF rect){
	m_boundRect = rect;
	m_InnerRect = rect;                 //设置内层矩形，如果鼠标在内层矩形中，改变鼠标样式
	m_InnerRect.setLeft(rect.left());
	m_InnerRect.setRight(rect.right());
	update(boundingRect());
}
void CollidingRectItem::setID(int id)
{
    this->id = id;
}

int CollidingRectItem::getID()
{
    return this->id;
}

int CollidingRectItem::getType()
{
    return this->m_type;
}

void CollidingRectItem::setType(int iType)
{
    this->m_type = iType;
}

void CollidingRectItem::setIsShowing(bool bState)
{
    m_IsShowing = bState;
}

bool CollidingRectItem::getIsShowing()
{
    return m_IsShowing;
}

QMap<int, RouteInfo *> CollidingRectItem::getRouteSetting()
{
    return m_route_setting;
}

QList<QList<int> > CollidingRectItem::getShowTeamsList()
{
    return this->m_show_teams;
}

//阀常开
void CollidingRectItem::setData(int iOutputValue1)
{
	m_out_value1 = iOutputValue1;
}

//多米诺
void CollidingRectItem::setData(int iKeepTime,int iDelayTime,
	int iPerialTime, int iLoopTimes, int iOutputValue1, int iOutputValue2, int iStartDelay, int iReverseFlag, int iFromLeft)
{
    m_keep_time = iKeepTime;
    m_delay_time = iDelayTime;
    m_perial_time = iPerialTime;
    m_loop_time = iLoopTimes;

	m_out_value1 = iOutputValue1;
	m_out_value2 = iOutputValue2;
	m_start_delay = iStartDelay;

	m_reverse_flag = iReverseFlag;
	m_from_left = iFromLeft;
}

//图片
void CollidingRectItem::setData(QString qStrImagePath,int delayTime,int LoopTime,bool bIsYangWen)
{
    m_ImagePath = qStrImagePath;
    m_loop_time = LoopTime;
    m_delay_time = delayTime;
    m_IsYangWen = bIsYangWen;
    MyItem* parent = (MyItem*)this->parent();
    resizeImage2ShowTeam(parent->m_Show_Team_list);
}

//DMX
void CollidingRectItem::setData(int type, int orbit_id, int out_value1, int out_value2,
                                int keep_time, int delay_time, int perial_time,
                                int loop_time, int start_delay, int change_speed,
								int fade_in, int fade_out, int reverse_flag, int from_left)
{
    m_type = type;
    m_orbit_id = orbit_id;
    m_out_value1 = out_value1;
    m_out_value2 = out_value2;
    m_keep_time = keep_time;
    m_delay_time = delay_time;
    m_perial_time = perial_time;
    m_loop_time = loop_time;
    m_start_delay = start_delay;
    m_change_speed = change_speed;
	m_fade_in = fade_in;
	m_fade_out = fade_out;
	m_reverse_flag = reverse_flag;
	m_from_left = from_left;
}

//时钟Clock
void CollidingRectItem::setData(int clockstylesel, int timeleadvalue, int delayTime, int LoopTime, bool bIsYangWen)
{
//	m_ImagePath = qStrImagePath;
	m_clockstylesel = clockstylesel;
	m_timeleadvalue = timeleadvalue;
	m_loop_time = LoopTime;
	m_delay_time = delayTime;
	m_IsYangWen = bIsYangWen;

	//下面这两行没懂
	MyItem* parent = (MyItem*)this->parent();
	resizeImage2ShowTeam(parent->m_Show_Team_list);
}

void CollidingRectItem::setData(QMap<int, RouteInfo*> setting)
{
    m_route_setting = setting;
}

void CollidingRectItem::setTimes(float begin_time, float end_time)
{
    this->begin_time = begin_time;
    this->end_time = end_time;
}

void CollidingRectItem::resizeImage2ShowTeam(QString device_list)
{
    QImage img(m_ImagePath);
    if(!img.isNull())
    {
        m_show_team_list = device_list;
        QStringList list = device_list.split("\n");
        int array_num = list.size();
 //       new_image = img.scaled(array_num,64,Qt::KeepAspectRatio);
        new_image = img.scaledToWidth(array_num);
    }
}

//void CollidingRectItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
//{
//    QPointF scene_pos = event->scenePos();
//    m_boundRect.setX(scene_pos.y());
//    update(m_boundRect);
//    this->moveBy(0,0);
//}

//编曲界面双击打开编辑对话框
void CollidingRectItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(Qt::LeftButton == event->button())
    {
        QString qStrSql;
        if(DOMINO_RUN == m_type)
        {	//多米诺
            DominoSetting domino;
            domino.setData(m_keep_time,m_delay_time,
				m_perial_time, m_loop_time, m_out_value1, m_out_value2, m_start_delay,m_reverse_flag,m_from_left );
            domino.setModal(true);
            if(QDialog::Accepted == domino.exec())
            {
                m_keep_time = domino.m_iKeepTime;
                m_delay_time = domino.m_iDelayTime;
                m_perial_time = domino.m_iPerialTime;
				m_loop_time = domino.m_iLoopTimes;

				m_out_value1 = domino.m_iOutputValue1;
				m_out_value2 = domino.m_iOutputValue2;
				m_start_delay= domino.m_iStartDelay;

				m_reverse_flag = domino.m_iReverseFlag;
				m_from_left = domino.m_iFromLeft;

			//	m_route_setting = setting;

				//创建ControlInfo的列，不成功
			/*	QSqlQuery create("CREATE table IF NOT EXISTS  `route` ( `keep_time` INTEGER, `delay_time` INTEGER, `perial_time` INTEGER, `loop_time` TEXT, `out_value1` INTEGER, `out_value2` INTEGER, `start_delay` INTEGER , `reverse_flag` INTEGER ,`from_left` INTEGER )");
				if (!create.exec())
				{
				//	QMessageBox::information(this, "警告", "有错误!");
				//	QMainWindow::close();
					qDebug() << "update route false!sql:" << qStrSql <<
						"error:" << create.lastError().text();
				} */

			/*	qStrSql = QString("update route set keep_time = %1,"
					"delay_time = %2,perial_time = %3,loop_time = %4,"
					"out_value1 = %5,out_value2 = %6,start_delay = %7,reverse_flag = %8,from_left = %9"
					" where music_action_id = %10").arg(m_keep_time).
					arg(m_delay_time).arg(m_perial_time).
					arg(m_loop_time).arg(m_out_value1).arg(m_out_value2).arg(m_start_delay).
					arg(m_reverse_flag).arg(m_from_left).
					arg(id);

				QSqlQuery query;
				if (!query.exec(qStrSql))
				{
					qDebug() << "update route false!sql:" << qStrSql <<
						"error:" << query.lastError().text();
				} */

                qStrSql = QString("update route set keep_time = %1,"
                                  "delay_time = %2,perial_time = %3,loop_time = %4,"
								  "out_value1 = %5,out_value2 = %6,start_delay = %7,reverse_flag = %8,from_left = %9"
                                  " where music_action_id = %10").arg(m_keep_time).
                        arg(m_delay_time).arg(m_perial_time).
						arg(m_loop_time).arg(m_out_value1).arg(m_out_value2).arg(m_start_delay).
						arg(m_reverse_flag).arg(m_from_left).
						arg(id);

                QSqlQuery query;
                if(!query.exec(qStrSql))
                {
                    qDebug() <<"update route false!sql:"<<qStrSql<<
                               "error:"<<query.lastError().text();
                }
            }
        }
        else if(IMAGE_SHOW == m_type)
        {
            SelectImageDialog img_dlg;
            img_dlg.setData(m_ImagePath,m_delay_time
                            ,m_loop_time,m_IsYangWen);
            img_dlg.setModal(true);
            if(QDialog::Accepted == img_dlg.exec())
            {
                m_ImagePath = img_dlg.m_ImagePath;
                resizeImage2ShowTeam(m_show_team_list);
                m_delay_time = img_dlg.m_DelayTime;
                m_loop_time = img_dlg.m_LoopTime;
                m_IsYangWen = img_dlg.m_IsYangWen;
                qStrSql = QString("update route set img_path = '%1',"
                                  "delay_time = %2,loop_time = %3,yang_wen=%4 where music_action_id = %5").
                        arg(m_ImagePath).arg(m_delay_time).arg(m_loop_time).arg(m_IsYangWen).arg(id);
                QSqlQuery query;
                if(!query.exec(qStrSql))
                {
                    qDebug() <<"update route false!sql:"<<qStrSql<<
                               "error:"<<query.lastError().text();
                }
            }
        }
        else if(DMX512_SHOW == m_type)
        {
            DMXLightDialog dmx_dlg;
            dmx_dlg.setData(m_route_setting);
            dmx_dlg.setModal(true);
            if(QDialog::Accepted == dmx_dlg.exec())
            {
				m_route_setting = dmx_dlg.m_route_setting;
			//	m_route_setting = dmx_dlg.user_route_setting;

			//	setting = m_route_setting;

                QList<int> keys = m_route_setting.keys();
                for(int i = 0 ; i < keys.size(); ++i)
                {
                    int route_id = keys.at(i);
                    RouteInfo* dmx_set = m_route_setting[route_id];

				//	MusicAction* music_action = StaticValue::GetInstance()->m_music_action[route_id];
				//	QMap<int, RouteInfo*> route_info = music_action->route_info;
				//	route_info[route_id] = m_route_setting[route_id];

                    qStrSql = QString("update route set type = %1,out_value1 = %2,"
                                      "out_value2 = %3,keep_time = %4,delay_time = %5,"
                                      "perial_time = %6,loop_time = %7,start_delay = %8,"
                						"orbit_id = %9,change_speed = %10,len1 = %11,len2 = %12,speed = %13,circle = %14,"
										"fade_in = %15,fade_out = %16,reverse_flag = %17,from_left = %18"
				                		" where route_id = %19 and music_action_id = %20").
                            arg(dmx_set->type).arg(dmx_set->out_value1).arg(dmx_set->out_value2).
                            arg(dmx_set->keep_time).arg(dmx_set->delay_time).arg(dmx_set->perial_time).
                            arg(dmx_set->loop_time).arg(dmx_set->start_delay).arg(dmx_set->orbit_id).
                            arg(dmx_set->change_speed).arg(dmx_set->len1).arg(dmx_set->len2).arg(dmx_set->speed).arg(dmx_set->circle).
							arg(dmx_set->fade_in).arg(dmx_set->fade_out).arg(dmx_set->reverse_flag).arg(dmx_set->from_left).arg(route_id).arg(id);

                    QSqlQuery query;
					if(!query.exec(qStrSql))
                    {
                        QString tmpstr  =   query.lastError().text();   //没有column len1
                        qDebug() <<"update route false!sql:"<<qStrSql<<
                                   "error:"<<query.lastError().text();
                    }
                }
            }
        }
		else if (CLOCK_SHOW == m_type)
		{
			ClockDialog clock_dlg;
			clock_dlg.setData(m_clockstylesel, m_timeleadvalue, m_delay_time
				, m_loop_time, m_IsYangWen);
			clock_dlg.setModal(true);
			if (QDialog::Accepted == clock_dlg.exec())
			{
				m_clockstylesel = clock_dlg.m_ClockStyleSel;
				m_timeleadvalue = clock_dlg.m_TimeLeadValue;
				m_delay_time = clock_dlg.m_DelayTime;
				m_loop_time = clock_dlg.m_LoopTime;
				m_IsYangWen = clock_dlg.m_IsYangWen;
				qStrSql = QString("update route set ClockStyle = '%1',"
					"delay_time = %2,loop_time = %3,yang_wen=%4,TimeLeadValue=%5 where music_action_id = %6").
					arg(m_clockstylesel).arg(m_delay_time).arg(m_loop_time).arg(m_IsYangWen).arg(m_timeleadvalue).arg(id);
				QSqlQuery query;
				if (!query.exec(qStrSql))
				{
					qDebug() << "update route false!sql:" << qStrSql <<
						"error:" << query.lastError().text();
				}
			}
		}

    }
}

void CollidingRectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event){

    if(!m_InnerRect.contains(event->pos()))
    {
        QPointF Spos    =   event->pos();
        qreal inner_left,inner_right,outer_left,outer_right;
        inner_left  =   m_InnerRect.left();
        inner_right =   m_InnerRect.right();
        outer_left  =   m_boundRect.left();
        outer_right =   m_boundRect.right();

        if((outer_left<=Spos.x())&&(Spos.x()<=inner_left))    //左侧
        {
            direction   =   7;

        }
        else if((inner_right<=Spos.x())&&(Spos.x()<=outer_right))  //右侧
        {
            direction   =   3;
        }

        setCursor(Qt::SizeHorCursor);
    }
    else
    {
        setCursor(Qt::OpenHandCursor);
    }
    update();

    QGraphicsItem::hoverMoveEvent(event);
}

//鼠标拖动编曲块的处理
void CollidingRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if(event->buttons()==Qt::LeftButton)
	{
        //拖拽 或者 改变边缘大小的处理

		//判断光标状态
		QList<QGraphicsView *> Views   =   scene()->views();
		Q_ASSERT(Views.size()>=1);
		QGraphicsView* GV  =   Views.at(0);
		MusicWidget* parentView    =  qobject_cast<MusicWidget*>(GV->parent());
		MusicGraphicsView* musicView = qobject_cast<MusicGraphicsView*>(GV);

		QCursor curcursor  =   cursor();           //获取当前光标类型

//		if(m_boundRect.x()<parentView->m_HeaderWidth)      //出界判断
//		{
//			return;
//		}
		if(curcursor.shape()==Qt::SizeHorCursor)
		{
			QRectF tem =    m_boundRect;
			const qreal snapDistance = 6.0;

			switch (direction)
			{
				case 3:                                          //右侧
				{	//鼠标拖动编曲块右边框
					qreal targetX = event->pos().x();
					if (musicView)
					{
						QVector<CutSolidVerticalLine*> cutLines = musicView->getcutsolidlines();
						qreal bestX = targetX;
						qreal bestDist = snapDistance + 1.0;
						for (int i = 0; i < cutLines.size(); ++i)
						{
							CutSolidVerticalLine* line = cutLines.at(i);
							if (!line)
							{
								continue;
							}
							qreal lineX = line->getX();
							qreal dist = qAbs(lineX - targetX);
							if (dist <= snapDistance && dist < bestDist)
							{
								bestDist = dist;
								bestX = lineX;
							}
						}
						if (bestDist <= snapDistance && bestX > tem.left() + 1.0)
						{
							targetX = bestX;
						}
					}
					tem.setRight(targetX);             //更新Rect右侧位置
				//	QTime endtime    =   QTime::fromMSecsSinceStartOfDay((tem.right()-parentView->m_HeaderWidth)/parentView->m_balance*1000.0);
					QTime endtime = QTime::fromMSecsSinceStartOfDay((tem.right() - parentView->m_HeaderWidth)*parentView->time_length / (parentView->m_EditView - 30.0) * 1000.0);
					QRectF righttextrect =   rightTimeTip_->boundingRect();
					rightTimeTip_->setText(endtime.toString("m:s:z"));
					QPointF    pt  =   tem.topRight();
					rightTimeTip_->setPos(pt.x()-righttextrect.width()/2,pt.y()-righttextrect.height());	//输出时间文字
					prepareGeometryChange();
					break;
				}
				case 7:
				{	//鼠标拖动编曲块左边框
					qreal targetX = event->pos().x();
					if (musicView)
					{
						QVector<CutSolidVerticalLine*> cutLines = musicView->getcutsolidlines();
						qreal bestX = targetX;
						qreal bestDist = snapDistance + 1.0;
						for (int i = 0; i < cutLines.size(); ++i)
						{
							CutSolidVerticalLine* line = cutLines.at(i);
							if (!line)
							{
								continue;
							}
							qreal lineX = line->getX();
							qreal dist = qAbs(lineX - targetX);
							if (dist <= snapDistance && dist < bestDist)
							{
								bestDist = dist;
								bestX = lineX;
							}
						}
						if (bestDist <= snapDistance && bestX < tem.right() - 1.0)
						{
							targetX = bestX;
						}
					}
					tem.setLeft(targetX);              //更新Rect左侧位置
				//	QTime starttime    =   QTime::fromMSecsSinceStartOfDay((tem.left()-parentView->m_HeaderWidth)/parentView->m_balance*1000.0);
					QTime starttime = QTime::fromMSecsSinceStartOfDay((tem.left() - parentView->m_HeaderWidth)*parentView->time_length / (parentView->m_EditView - 30.0) *1000.0);
					leftTimeTip_->setText(starttime.toString("m:s:z"));
					QRectF lefttextrect =   leftTimeTip_->boundingRect();
					QPointF    pt  =   tem.topLeft();
					leftTimeTip_->setPos(pt.x()-lefttextrect.width()/2,pt.y()-lefttextrect.height());	//输出时间文字
					prepareGeometryChange();
					break;
				}
				default:
				{
					break;
				}
			}
			if(tem.x()>=parentView->m_HeaderWidth)
			{
				this->ResetRect(tem);
				this->scene()->update();                    //重绘，解决残影
			}
			return;        //处理完边缘拖拽以后，退出
		}
		else if(curcursor.shape()==Qt::OpenHandCursor)
		{	//鼠标拖动整个编曲块
			QPointF newPos    =   event->pos();
			QPointF oldPos    =   event->lastPos();
			QPointF computPos =   newPos-oldPos;
			QPointF   curPos  =   pos();
			QRectF rect1,rect2;
			prepareGeometryChange();

			if(m_boundRect.x()<=parentView->m_HeaderWidth&&computPos.x()<0)         //左侧越界，并继续向左移动
			{
				return;
			}
			if(m_boundRect.right()>=parentView->m_EditView+parentView->m_HeaderWidth&&computPos.x()>0)    //右侧越界，并继续向右移动
			{
				return;
			}

//			setPos(curPos.x()+computPos.x(),curPos.y());

			m_boundRect.adjust(computPos.x(),0,computPos.x(),0);
			QRectF tem =   m_boundRect;

			//tem.left()值的范围是150-1600@1920屏幕分辨率，m_HeaderWidth的值是150，
			//获得编曲块的begin_time和end_time
			QTime starttime = QTime::fromMSecsSinceStartOfDay((tem.left() - parentView->m_HeaderWidth)*parentView->time_length / (parentView->m_EditView - 30.0) *1000.0);
			QTime endtime = QTime::fromMSecsSinceStartOfDay((tem.right() - parentView->m_HeaderWidth)*parentView->time_length / (parentView->m_EditView - 30.0) * 1000.0);
			QRectF righttextrect =   rightTimeTip_->boundingRect();
			rightTimeTip_->setText(endtime.toString("m:s:z"));
			QPointF    pt  =   tem.topRight();
			rightTimeTip_->setPos(pt.x()-righttextrect.width()/2,pt.y()-righttextrect.height());
			QRectF lefttextrect =   leftTimeTip_->boundingRect();
			leftTimeTip_->setText(starttime.toString("m:s:z"));
			pt  =   tem.topLeft();
			leftTimeTip_->setPos(pt.x()-lefttextrect.width()/2,pt.y()-lefttextrect.height());

			ResetRect(m_boundRect);
			scene()->update();
		}
	}
	else                               //鼠标没有任何行为的移动
	{
		QGraphicsItem::mouseMoveEvent(event);
	}

//    int t=0;
//    if(cScale)
//    {
//        QPointF dis;
//        end=event->scenePos();
//        dis=end-start;
//        start=end;
//        switch(direction)
//        {
//        case 0:         //正常移动，拖拽
//        {
//              QPointF newPos    =   event->pos();
//              QPointF oldPos    =   event->lastPos();
//              QPointF computPos =   newPos-oldPos;
//              QPointF   curPos  =   pos();
//              setPos(curPos.x()+computPos.x(),curPos.y());
////            QRectF currect  =   m_boundRect;
////            update();
////            int i = scene()->collidingItems(this,Qt::ContainsItemBoundingRect).size();       //边界碰撞检测
////            if(i==1)        //没有发生碰撞，随便移动，不受限制
////            {
////                QGraphicsItem::mouseMoveEvent(event);
////            }
////            else if(i==0)   //现在已经是贴边的状态
////            {
////                QPointF subPoint    =   event->pos()-event->lastPos();
////                if(subPoint.y()<=0)
////                {
////                    QGraphicsItem::mouseMoveEvent(event);
////                }
////            }
//            break;
//        }
//        case 3:             //尾部缩放
//        {
//            m_boundRect.topRight().setX(event->scenePos().x());
//            QRectF tem  =   m_boundRect;
//            QRectF tem=QRectF(m_boundRect.x(),
//                              m_boundRect.y(),
//                              m_boundRect.width()+dis.x(),
//                              m_boundRect.height());

//            QRectF tem =    m_boundRect;
//            tem.setRight(event->pos().x());             //更新Rect右侧位置
//            this->ResetRect(tem);
//            this->scene()->update();                    //重绘，解决残影
////            update();
//            break;
//        }
//        case 7:             //头部缩放
//        {
////            QRectF tem=QRectF(m_boundRect.x(),
////                              m_boundRect.y(),
////                              m_boundRect.width()-dis.x(),
////                              m_boundRect.height());
//            QRectF tem =    m_boundRect;
//            tem.setLeft(event->pos().x());              //更新Rect左侧位置
//            this->ResetRect(tem);
//            this->scene()->update();                    //重绘，解决残影
//            break;
//        }
//        default:
//        {
//            //设置不让上下移动
//             //           m_boundRect.setX(event->screenPos().x());
//             //           this->setPos(m_boundRect.x(),m_boundRect.y());
//        }
//        }

//    }
}

//void CollidingRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
//{
//    cScale=false;
//    update();
//    QGraphicsItem::mouseReleaseEvent(event);
//}
