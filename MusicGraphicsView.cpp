#include "MusicGraphicsView.h"

#include <QApplication>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPoint>
#include <QPointF>
#include "CollidingRectItem.h"
#include "DominoSetting.h"
#include "CommonFile.h"
#include "SelectImageDialog.h"
#include "DMXLightDialog.h"

#include "ClockDialog.h"

#include "MusicWidget.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QtCore>
#include <algorithm>

MusicGraphicsView::MusicGraphicsView(QWidget *parent) :
QGraphicsView(parent)
{
	//    QGraphicsView::setDragMode(QGraphicsView::ScrollHandDrag);

	m_IsPlay = false;   //没有处于播放状态
	spaceKeyPressed = false; //判断回车键盘是不是按下
	bvirtualenterpress = false; //虚拟回车键
	isfrommouse = false; //判断是不是来自鼠标
	m_movableVerticalLine = nullptr;
	m_getcutsolidlines.clear();
	setDragMode(QGraphicsView::RubberBandDrag);
	CreateActions();
}

//鼠标按下的响应
void MusicGraphicsView::mousePressEvent(QMouseEvent * event)
{
	//只有鼠标左键选中场景矩形才能有选中item
 	if (event->button() == Qt::LeftButton)
	{		
		slotsetmovelinefocus(true);
		m_leftPressPos = event->pos();
		m_leftPressPendingMove = true;
	}
	else if (event->button() == Qt::RightButton)
	{
		m_triggerPos = event->pos();	//按下鼠标右键
		if ((m_triggerPos.x() < 150) || (m_triggerPos.y() < 60))
			return;

		QPoint global_pos = this->mapToGlobal(m_triggerPos);
		QList<QGraphicsItem *>  items_under_mouse = items(event->pos());
		if (items_under_mouse.size() == 0)         //选中视图空白部分的情况
			return;
		for (int i = 0; i < items_under_mouse.size(); i++)
		{
			CollidingRectItem* item = qgraphicsitem_cast<CollidingRectItem*>(items_under_mouse[i]);
			if (item != nullptr)   //说明item是场景矩形，而不是背景或者时间指针(当场景叠加时，总是选取最上层的)
			{
				QGraphicsView::mousePressEvent(event);
				m_RightClickItem = item;
				m_delete->setDisabled(false);
				m_Menu->exec(global_pos);
				return;
			}
		}

		m_delete->setDisabled(true);
		m_Menu->exec(global_pos);
		//选中轨道的情况
	}

	QGraphicsView::mousePressEvent(event);
}

void MusicGraphicsView::deleteShowAction(CollidingRectItem* pItem)
{
	int iActionID = pItem->getID();
	QString qStrSql = QString("delete from music_action where id = %1").arg(iActionID);
	QSqlQuery query;
	if (query.exec(qStrSql))
	{
		qStrSql = QString("delete from route where music_action_id = %1").arg(iActionID);
		if (query.exec(qStrSql))
		{
			qStrSql = QString("delete from music_mapping where music_action_id = %1").arg(iActionID);
			if (!query.exec(qStrSql))
			{
				qDebug() << "delete from music_mapping false!sql:%1" << qStrSql <<
					"error:" << query.lastError().text();
			}
			else
			{
				this->scene()->removeItem(pItem);
				StaticValue::GetInstance()->m_MusicActions.remove(iActionID);
			}
		}
		else
			qDebug() << "delete from route false!sql:%1" << qStrSql <<
			"error:" << query.lastError().text();
	}
	else
		qDebug() << "delete from music_action false!sql:%1" << qStrSql <<
		"error:" << query.lastError().text();
}

//add  by eagle for key logic, 先这里响应，在Item里面响应按键信息.
void MusicGraphicsView::keyPressEvent(QKeyEvent *event)
{
	if (Qt::Key_Delete == event->key())
	{
		if (!m_IsPlay)
		{
			QList<QGraphicsItem*> items = this->scene()->selectedItems();
			if (items.isEmpty())
			{
				return QGraphicsView::keyPressEvent(event);
			}

			bool removedLine = false;
			for (int i = 0; i < items.size(); ++i)
			{
				QGraphicsItem* currentItem = items.at(i);
				CutSolidVerticalLine* cutSolidLine = qgraphicsitem_cast<CutSolidVerticalLine*>(currentItem);
				if (cutSolidLine)
				{
					int index = m_cutsolidlines.indexOf(cutSolidLine);
					if (index != -1)
					{
						m_cutsolidlines.remove(index);
					}
					int loadIndex = m_getcutsolidlines.indexOf(cutSolidLine);
					if (loadIndex != -1)
					{
						m_getcutsolidlines.remove(loadIndex);
					}
					this->scene()->removeItem(cutSolidLine);
					delete cutSolidLine;
					removedLine = true;
					continue;
				}

				CollidingRectItem* item = qgraphicsitem_cast<CollidingRectItem*>(currentItem);
				if (item)
				{
					deleteShowAction(item);
				}
			}

			if (removedLine)
			{
				MusicWidget* music_info = qobject_cast<MusicWidget*>(this->parent());
				if (music_info)
				{
					music_info->savecutlines();
				}
				this->scene()->update();
			}

			//            QString qStrSql = QString("delete from music_action where id = %1").arg(id);
			//            QSqlQuery query;
			//            if(query.exec(qStrSql))
			//            {
			//                qStrSql = QString("delete from route where music_action_id = %1").arg(id);
			//                if(query.exec(qStrSql))
			//                {
			//                    qStrSql = QString("delete from music_mapping where music_action_id = %1").arg(id);
			//                    if(!query.exec(qStrSql))
			//                    {
			//                        qDebug()<<"delete from music_mapping false!sql:%1"<<qStrSql<<
			//                                  "error:"<<query.lastError().text();
			//                    }
			//                    else
			//                    {
			//                        this->scene()->removeItem(item);
			//                        StaticValue::GetInstance()->m_MusicActions.remove(id);
			//                    }
			//                }
			//                else
			//                    qDebug()<<"delete from route false!sql:%1"<<qStrSql<<
			//                              "error:"<<query.lastError().text();
			//            }
			//            else
			//                qDebug()<<"delete from music_action false!sql:%1"<<qStrSql<<
			//                          "error:"<<query.lastError().text();
		}
		else
		{
			QMessageBox::information(this, tr("提示"),
				tr("当前处于播放状态，因此不允许删除，请暂停后重试！"),
				QMessageBox::Ok);
		}
	}
	else if (Qt::Key_Return == event->key())
	{
 		foreach(CollidingRectItem* item, m_music_actions)
		{
			qDebug() << item->begin_time << item->end_time;
		}
		spaceKeyPressed = true;
		bvirtualenterpress = true; // 虚拟回车按下
	
	}
	else if (event->key() == Qt::Key_Space)
	{
		if (m_movableVerticalLine == nullptr )
		{
			
		}
		else
		{
			qDebug() << "MusicGraphicsView::keyPressEvent" << m_movableVerticalLine->getX();
			qint64 isignalpos = m_movableVerticalLine->getX(); // 60000;			
 			if (m_IsPlay == false)
			{
				emit signalsetmediaposition(isignalpos);
			}
			
		}
		
	}
	if (event->key() == Qt::Key_S && event->modifiers() == Qt::AltModifier) {
		// Your code for handling Alt + S. 实现再次填充方块在两个红线之间
		if (!m_IsPlay){
			QList<QGraphicsItem*> items = this->scene()->selectedItems();
			if (items.isEmpty())
			{
				return QGraphicsView::keyPressEvent(event);
			}
			///////////////
			int i = 0;      //尝试计算点击所在第几行
			i = m_triggerPos.y() / 30;
			i++;
			int hormid;     //计算垂直居中的y坐标值
			hormid = i * 30 - 5 - 20;    //20为场景矩形高度 5为上下边距
			//add by eagle for Debug action_rect = QRectF(m_triggerPos.x(), hormid, 30, 20);
			//第三个参数是宽度(这个才是我们的要计算的)，第四个是固定高度.
			qreal startx, width = 30;
			qreal iheight = 20;
			rescalewidth(&startx, &hormid, &width, &iheight);

			///////////////////////
		}
	}

	QGraphicsView::keyPressEvent(event);
}

//鼠标释放的响应
void MusicGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{	//鼠标左键释放
		slotsetmovelinefocus(true);
		if (m_leftPressPendingMove)
		{
			m_leftPressPendingMove = false;
			const int dragDistance = QApplication::startDragDistance();
			if ((event->pos() - m_leftPressPos).manhattanLength() < dragDistance && m_movableVerticalLine)
			{
				QPointF scenePoint = mapToScene(event->pos());
				MusicWidget* music_info = qobject_cast<MusicWidget*>(this->parent());
				qreal leftBound = 150.0;
				if (music_info)
				{
					leftBound = music_info->m_HeaderWidth;
				}
				qreal rightBound = this->sceneRect().right();
				qreal targetX = scenePoint.x();
				if (targetX < leftBound)
				{
					targetX = leftBound;
				}
				if (targetX > rightBound)
				{
					targetX = rightBound;
				}

				qreal newMx = (targetX + leftBound) / 2.0;
				qreal newPosX = (targetX - leftBound) / 2.0;
				m_movableVerticalLine->setX(newMx);
				m_movableVerticalLine->setPos(newPosX, 0.0);
				m_movableVerticalLine->setTimeText(newMx);
				m_movableVerticalLine->setFocus();
				this->scene()->update();
			}
		}

		QPointF scenePoint = mapToScene(event->pos());
		QGraphicsItem *selectedItem = scene()->itemAt(scenePoint, QTransform());
		if (selectedItem && selectedItem->type() == CutSolidVerticalLine::Type)
		{
			slotsetmovelinefocus(false);
			selectedItem->setFocus();
		}
		if (selectedItem && selectedItem->type() == CollidingRectItem::Type)
		{
			selectedItem->setFocus();
		}
	}
	QGraphicsView::mouseReleaseEvent(event);
}


//鼠标移动？
void MusicGraphicsView::repositemMouse(QMouseEvent *event)
{
	QPointF scenePoint = mapToScene(event->pos());
	QGraphicsItem *selectedItem = scene()->itemAt(scenePoint, QTransform());

	if (selectedItem && selectedItem->type() == MovableVerticalLine::Type)
	{
		qreal originalY = 0;
		qreal newX = selectedItem->pos().x() + scenePoint.x() - event->pos().x();
		qDebug() << "MusicGraphicsView::repositemMouse" << newX;
		selectedItem->setPos(newX, originalY); 
	}
	if (selectedItem && selectedItem->type() == CutSolidVerticalLine::Type)
	{
		qreal originalY = 0;
		qreal newX = selectedItem->pos().x() + scenePoint.x() - event->pos().x();
		qDebug() << "MusicGraphicsView::repositemMouse" << newX;
		selectedItem->setPos(newX, originalY);
	}
}
QVector<CollidingRectItem *> MusicGraphicsView::getCollidingItems()
{
    return m_music_actions;
}

QVector<CutSolidVerticalLine *> MusicGraphicsView::getcutsolidlines()
{
    return m_cutsolidlines;
}

void MusicGraphicsView::InitMusicActions(int action_id, QRectF rect,
	float begin_time, float end_time, MyItem *parent)
{
	MusicAction* music_action = StaticValue::GetInstance()->m_music_action[action_id];
	QMap<int, RouteInfo*> route_info = music_action->route_info;
	if (music_action->type == ALWAYS_ON)
	{
		addAlwaysOn(action_id, rect, begin_time, end_time, route_info, parent);
	}
	else if (music_action->type == DOMINO_RUN)
	{
		addDomino(action_id, rect, begin_time, end_time, route_info, parent);
	}
	else if (music_action->type == IMAGE_SHOW)
	{
		addImage(action_id, rect, begin_time, end_time, route_info, parent);
	}
	else if (music_action->type == DMX512_SHOW)
	{
		addDmx512(action_id, rect, begin_time, end_time, route_info, parent);
	}
	else if (music_action->type == CLOCK_SHOW)
	{
		addClock(action_id, rect, begin_time, end_time, route_info, parent);
	}
}

void MusicGraphicsView::setPlayState(bool bState)
{
    m_IsPlay = bState;
}
void MusicGraphicsView::setmovableVerticalLinetime(float ftimelength, qint64 imin, qint64 imax) // 设置参考线音乐总长度
{
	if (m_movableVerticalLine != nullptr)
	{
		qreal tl = ftimelength;
		m_movableVerticalLine->setMusiclength(tl);
		m_movableVerticalLine->setTimeText(tl);
	}
}

//从文件glw读取竖直红线数据
void MusicGraphicsView::setCutSolidLines(const QString &filename)
{
	QRectF sceneRect = this->sceneRect();

	MusicWidget* music_info = (MusicWidget*)this->parent();

	QFile file(filename);
	if (file.open(QIODevice::ReadOnly)) 
	{
		QDataStream in(&file);
		in.setVersion(QDataStream::Qt_5_6);
		int size;
		in >> size;
		for (int i = 0; i < size; i++) 
		{
			qreal x, y1, y2,time;
		//	in >> x >> y1 >> y2;		//原来读出来坐标
			in >> time;		//现在读出来时间，单位毫秒

		//	从time获得x, y1, y2。在1920分辨率下sceneRect.width()的值是1570，实际宽度1420
			x = time / (music_info->time_length * 1000.0) * (sceneRect.width()-150.0) + 150.0;
			y1 = 30.0 * 2;
			y2 =sceneRect.bottom();
			CutSolidVerticalLine* line = new CutSolidVerticalLine(x, y1, y2, time);
			line->setMusiclength(music_info->time_length);
			m_getcutsolidlines.append(line);
		}
	}
}

void MusicGraphicsView::rescalewidth(qreal *startx, int *hormid, qreal *width, qreal *heigh)
{
	int index1 = -1;
	int index2 = -1;
	float minDistance = std::numeric_limits<float>::max();

	// Find the two 分割线objects between which the mouse click occurred
	for (int i = 0; i < m_cutsolidlines.size() - 1; ++i) {
		float leftX = m_cutsolidlines[i]->getX();
		float rightX = m_cutsolidlines[i + 1]->getX();

		if (m_triggerPos.x() >= leftX && m_triggerPos.x() <= rightX) {
			float distance = rightX - leftX;
			if (distance < minDistance) {
				index1 = i;
				index2 = i + 1;
				minDistance = distance;
			}
		}
	}

	// Calculate the x-coordinate and width of the action_rect
	 *startx = m_triggerPos.x(); // Default x-coordinate
	 *width = 30; // Default width

	if (index1 != -1 && index2 != -1) {
		*startx = m_cutsolidlines[index1]->getX();
		*width = m_cutsolidlines[index2]->getX() - *startx;
	}
}

void MusicGraphicsView::onDrawVerticalLineButtonClicked()
{
	//spaceKeyPressed = true;
	//isfrommouse = true;
	//float xPosition = m_triggerPos.x(); // 您可以根据需要设置 xPosition 的值
	//drawVerticalLine(xPosition);
	// 这里本来是画线, 但是画线是键盘操作多，所以这里另作 - 填充此空格功能
	if (m_IsPlay)
	{
		QMessageBox::information(this, tr("提示"),
			tr("当前处于播放状态，因此不允许修改表演动作！"),
			QMessageBox::Ok);
		return;
	}

	QRectF rect;

	int i = 0;      //尝试计算点击所在第几行
	i = m_triggerPos.y() / 30;
	i++;
	int hormid;     //计算垂直居中的y坐标值
	hormid = i * 30 - 5 - 20;    //20为场景矩形高度 5为上下边距

	int index1 = -1;
	int index2 = -1;
	float minDistance = std::numeric_limits<float>::max();

	// Find the two 分割线objects between which the mouse click occurred
	for (int i = 0; i < m_cutsolidlines.size() - 1; ++i)
	{
		float leftX = m_cutsolidlines[i]->getX();
		float rightX = m_cutsolidlines[i + 1]->getX();

		if (m_triggerPos.x() >= leftX && m_triggerPos.x() <= rightX)
		{
			float distance = rightX - leftX;
			if (distance < minDistance)
			{
				index1 = i;
				index2 = i + 1;
				minDistance = distance;
			}
		}
	}
	// Calculate the x-coordinate and width of the action_rect
	qreal startx = m_triggerPos.x(); // Default x-coordinate
	qreal width = 30; // Default width

	if (index1 != -1 && index2 != -1)
	{
		startx = m_cutsolidlines[index1]->getX();
		width = m_cutsolidlines[index2]->getX() - startx;
	}

	//action_rect = QRectF(startx, hormid, width, 20);
	rect = QRectF(startx+1, hormid, width-1, 20);

	CollidingRectItem* rect_item = (CollidingRectItem*)this->itemAt(m_triggerPos);
	rect_item->setscaleRect(rect);
}

//画时间竖红线，xPosition的值是
void MusicGraphicsView::drawVerticalLine(float xPosition)
{
 	qDebug() <<"当前传递的x" <<xPosition;
	if (bvirtualenterpress==false)
	{		
	}
	else
	{
		if (!spaceKeyPressed)
		{
			return;
		}
	}
	bvirtualenterpress = true;
	spaceKeyPressed = false;
	// 获取场景的边界
   	QRectF sceneRect = this->sceneRect();
	// 创建一个新的可移动竖线，从场景的顶部到底部

	MusicWidget* music_info = (MusicWidget*)this->parent();

	///init
	//add by eagle for 虚线开头和结尾添加到虚线Vector
    if ( m_cutsolidlines.size() == 0 )
	{
		
		if (m_getcutsolidlines.size() >= 3) //如果存储的size为0，就说明没有画线，这里不加载. 那么需要画头尾红线
		{
			int length = m_getcutsolidlines.size();
			for (int i = 0; i < length; i++)
			{
				CutSolidVerticalLine *line = m_getcutsolidlines.at(i);
				this->scene()->addItem(line);			//绘制已存储的红线
				// Store the new line in the QVector
				m_cutsolidlines.append(line);		
			}
		}
		else
		{
			// 线的起点坐标是Y1 是固定的，保持虚线一致 30*2 = 60
			CutSolidVerticalLine *line1 = new CutSolidVerticalLine(150, 30 * 2, sceneRect.bottom(),0);
			line1->setMusiclength(music_info->time_length);
			// 将竖线添加到场景中
			this->scene()->addItem(line1);

			// Store the new line in the QVector
			m_cutsolidlines.append(line1);

			// 线的起点坐标是Y1 是固定的，保持虚线一致 30*2 = 60
			CutSolidVerticalLine *line2 = new CutSolidVerticalLine(sceneRect.right(), 30 * 2, sceneRect.bottom(), music_info->time_length*1000);
			line2->setMusiclength(music_info->time_length);
			this->scene()->addItem(line2);
			// 将竖线添加到场景中
			//this->scene()->addItem(line2);

			// Store the new line in the QVector
			m_cutsolidlines.append(line2);
		}
		
		// 这条参考线.		
		m_movableVerticalLine = new MovableVerticalLine(150, 30 * 2, sceneRect.bottom());
		m_movableVerticalLine->setlineMaxX(sceneRect.right());
		connect(m_movableVerticalLine, &MovableVerticalLine::signalcutsolid, this, &MusicGraphicsView::drawVerticalLine);
		this->scene()->addItem(m_movableVerticalLine);
		// 将参考线添加到场景中
		// 设置竖线的焦点，以便它可以接收键盘事件
		m_movableVerticalLine->setFocus();
		
		return;
	}

	// 线的起点坐标是Y1 是固定的，保持虚线一致 30*2 = 60
	// 在参考线处按回车会执行到这里
    CutSolidVerticalLine *line;
	if (qIsFinite(xPosition) && isfrommouse == false)
	{
		float xplay = xPosition+150;

		//在1920屏幕下sceneRect.width()的值为1590,xPosition的范围是0-1420,xPositionRange的值是1420
		qreal xPositionRange = sceneRect.width() - 150;	//(1590-150)/2-10 = 1440/2-10 = 720-10 =710
		qreal time = xPosition / xPositionRange * music_info->time_length * 1000;	//毫秒

		line = new CutSolidVerticalLine(xplay, 30 * 2, sceneRect.bottom(), time);
	}
	else
	{
		//在1920屏幕下sceneRect.width()的值为1590,xPosition的范围是0-1420,xPositionRange的值是1420
		qreal xPositionRange = sceneRect.width() - 150;	//(1590-150)/2-10 = 1440/2-10 = 720-10 =710
		qreal time = m_triggerPos.x() / xPositionRange * music_info->time_length * 1000;	//毫秒

        line = new CutSolidVerticalLine(m_triggerPos.x(), 30 * 2, sceneRect.bottom(),time);
		isfrommouse = false;
	}
	line->setMusiclength(music_info->time_length);
	// 将竖线添加到场景中
	this->scene()->addItem(line);
	
	// Store the new line in the QVector
	m_cutsolidlines.append(line);
	
	// Sort the QVector by x-coordinate in ascending order
    std::stable_sort(m_cutsolidlines.begin(), m_cutsolidlines.end(),
        [](const CutSolidVerticalLine* a, const CutSolidVerticalLine* b) {
		return a->getX() < b->getX();
	});

    for (int i = 0; i < m_cutsolidlines.size(); ++i)
	{
        CutSolidVerticalLine* getline = m_cutsolidlines[i];
		qreal xValue = getline->getX();
		qDebug() << "Line " << i << " x value: " << xValue;
	}
	this->scene()->update();

	music_info->savecutlines();		//有新的红线，马上存储红线

	// 设置竖线的焦点，以便它可以接收键盘事件
    m_movableVerticalLine->setFocus();
}

void MusicGraphicsView::slotsetmovelinefocus(bool bset)
{
	//线只有画出来，才允许设置焦点 
	if (m_movableVerticalLine != NULL)
	{
		m_movableVerticalLine->setFocus();	
		qDebug() << "slotsetmovelinefocus called with bset:" << bset;
	}
}

void MusicGraphicsView::slotinitlinedata()
{
	// 这条参考线.
	// 获取场景的边界
	QRectF sceneRect = this->sceneRect();
	m_movableVerticalLine = new MovableVerticalLine(150, 30 * 2, sceneRect.bottom());
	connect(m_movableVerticalLine, &MovableVerticalLine::signalcutsolid, this, &MusicGraphicsView::drawVerticalLine);
	scene()->addItem(m_movableVerticalLine);
	// 将参考线添加到场景中
	// 设置竖线的焦点，以便它可以接收键盘事件
	m_movableVerticalLine->setFocus();
}

void MusicGraphicsView::CreateActions()
{
	m_Menu = new QMenu();

	m_Always = new QAction(m_Menu);
	m_DoMino = new QAction(m_Menu);
	m_AddImage = new QAction(m_Menu);
	m_Lignt = new QAction(m_Menu);
	m_Clock = new QAction(m_Menu);		//Clock		时钟
	m_Text	= new QAction(m_Menu);		//Text		文本文字
	m_Cartoon = new QAction(m_Menu);	//Cartoon	动画
	m_3D	= new QAction(m_Menu);		//3D		三维立体
	m_Math = new QAction(m_Menu);		//math		数学之美(基于3D)
	m_delete = new QAction(m_Menu);
	m_musicline = new QAction(m_Menu);
	m_copy = new QAction(m_Menu);
	m_paste = new QAction(m_Menu);
	InitMenuText();

	m_Menu->addAction(m_Always);
	m_Menu->addAction(m_DoMino);
	m_Menu->addAction(m_AddImage);
	m_Menu->addAction(m_Lignt);
	m_Menu->addAction(m_Clock);			//Clock		时钟
	m_Menu->addAction(m_Text);			//Text		文本文字
	m_Menu->addAction(m_Cartoon);		//Cartoon	动画
	m_Menu->addAction(m_3D);			//3D		三维立体
	m_Menu->addAction(m_Math);			//math		数学之美(基于3D)
	m_Menu->addAction(m_delete);
	m_Menu->addAction(m_musicline);
	m_Menu->addAction(m_copy);
	m_Menu->addAction(m_paste);

	connect(m_Always, SIGNAL(triggered()), this, SLOT(always_on()));
	connect(m_DoMino, SIGNAL(triggered()), this, SLOT(dominuo()));
	connect(m_AddImage, SIGNAL(triggered()), this, SLOT(addImage()));
	connect(m_Lignt, SIGNAL(triggered()), this, SLOT(changeLight()));

	connect(m_Clock, SIGNAL(triggered()), this, SLOT(addclock()));		//槽函数

	connect(m_delete, SIGNAL(triggered()), this, SLOT(delscence()));
	connect(m_musicline, SIGNAL(triggered()), this, SLOT(onDrawVerticalLineButtonClicked()));	
	connect(m_copy, SIGNAL(triggered()), this, SLOT(WidgetCopy()));
	connect(m_paste, SIGNAL(triggered()), this, SLOT(WidgetPaste()));
}

void MusicGraphicsView::InitMenuText()
{
	m_Always->setText(tr("常开"));
	m_DoMino->setText(tr("多米诺"));
	m_AddImage->setText(tr("水帘"));
	m_Lignt->setText(tr("DMX灯光"));

	m_Clock->setText(tr("时钟"));
	m_Text->setText(tr("文本文字"));
	m_Text->setDisabled(true);
	m_Cartoon->setText(tr("动画"));
	m_Cartoon->setDisabled(true);
	m_3D->setText(tr("三维立体"));
	m_3D->setDisabled(true);
	m_Math->setText(tr("数学图案"));
	m_Math->setDisabled(true);

	m_delete->setText(tr("删除场景"));
	m_musicline->setText("填充");
	m_copy->setText("复制");
	m_paste->setText("粘贴");
}

//新建常开动作
void MusicGraphicsView::addAlwaysOn(int action_id, QRectF rect, float begin_time, float end_time,
	QMap<int, RouteInfo*> route_info, MyItem* parent)
{
	if (route_info.size() > 0)
	{
		RouteInfo* route = route_info.value(0);

		CollidingRectItem* rect_item = new CollidingRectItem(ALWAYS_ON, rect,
			QColor(Qt::yellow),
			parent);
		rect_item->setID(action_id);
		rect_item->setZValue(1);
		rect_item->setTimes(begin_time, end_time);
		rect_item->setData(route->out_value1);

		this->scene()->addItem(rect_item);
		m_music_actions.append(rect_item);
	}
}

//新建多米诺动作
void MusicGraphicsView::addDomino(int action_id, QRectF rect, float begin_time, float end_time,
	QMap<int, RouteInfo*> route_info, MyItem *parent)
{
	if (route_info.size() > 0)
	{
		RouteInfo* route = route_info.value(0);
		CollidingRectItem* rect_item = new CollidingRectItem(DOMINO_RUN, rect,
			QColor(Qt::yellow),
			parent);
		rect_item->setID(action_id);
		rect_item->setZValue(1);
		rect_item->setTimes(begin_time, end_time);
		//add merge code for 0906
		rect_item->setData(route->keep_time,route->delay_time,
			route->perial_time, route->loop_time, route->out_value1, route->out_value2, route->start_delay, route->reverse_flag, route->from_left);

		this->scene()->addItem(rect_item);
		m_music_actions.append(rect_item);
	}
}

//新建DMX512动作
void MusicGraphicsView::addDmx512(int action_id, QRectF rect, float begin_time, float end_time,
	QMap<int, RouteInfo*> route_info, MyItem *parent)
{
	if (route_info.size() > 0)
	{
		CollidingRectItem* rect_item = new CollidingRectItem(DMX512_SHOW, rect,
			QColor(Qt::yellow),
			parent);
		rect_item->setID(action_id);
		rect_item->setZValue(1);
		rect_item->setTimes(begin_time, end_time);
		rect_item->setData(route_info);
		this->scene()->addItem(rect_item);
		m_music_actions.append(rect_item);
	}
}

//新建水帘图片表演
void MusicGraphicsView::addImage(int action_id, QRectF rect, float begin_time, float end_time,
	QMap<int, RouteInfo*> route_info, MyItem *parent)
{
	if (route_info.size() > 0)
	{
		RouteInfo* route = route_info.value(0);
		CollidingRectItem* rect_item = new CollidingRectItem(IMAGE_SHOW, rect,
			QColor(Qt::yellow),
			parent);
		rect_item->setID(action_id);
		rect_item->setZValue(1);
		rect_item->setTimes(begin_time, end_time);
		this->scene()->addItem(rect_item);
		rect_item->setData(route->img_path, route->delay_time,
			route->loop_time, route->bIsYangWen);
		m_music_actions.append(rect_item);
	}
}

//新建时钟表演，区分大小写，这个是打开编曲的时候调用，显示编曲块的内容
void MusicGraphicsView::addClock(int action_id, QRectF rect, float begin_time, float end_time,
	QMap<int, RouteInfo*> route_info, MyItem *parent)
{
	if (route_info.size() > 0)
	{
		RouteInfo* route = route_info.value(0);
		CollidingRectItem* rect_item = new CollidingRectItem(CLOCK_SHOW, rect,
			QColor(Qt::yellow),
			parent);
		rect_item->setID(action_id);
		rect_item->setZValue(1);
		rect_item->setTimes(begin_time, end_time);
		this->scene()->addItem(rect_item);
	//	rect_item->setData(route->img_path, route->delay_time,
	//		route->loop_time, route->bIsYangWen);
		//蚕食是从数据库读到route里面的数据
		rect_item->setData(route->ClockStyle, route->TimeLeadValue, route->delay_time, route->loop_time, route->bIsYangWen);	//Clock用
		m_music_actions.append(rect_item);
	}
}

//ALWAYS_ON
void MusicGraphicsView::always_on()
{
	if (m_IsPlay)
	{
		QMessageBox::information(this, tr("提示"),
			tr("当前处于播放状态，因此不允许修改表演动作！"),
			QMessageBox::Ok);
		return;
	}
	//这里只要判断一下Y1的值小于 30*2 就说明是音乐标题，返回就不会崩溃.
	if (m_triggerPos.y() < 60)
	{
		return;
	}
	//crash fixed end
	//先获取编队的ID
	MyItem* show_team_item = (MyItem*)this->itemAt(m_triggerPos);
	//MyItem* show_team_item = reinterpret_cast <MyItem*>(this->itemAt(m_triggerPos));
	if (show_team_item)
	{
		QRectF action_rect;
		int show_team_id = show_team_item->id;
		if (show_team_id == 0)       //id = 0时就不在数据库中添加数据了吗? 为啥会有这种判断
		{
			CollidingRectItem* rect_item = (CollidingRectItem*)this->itemAt(m_triggerPos);
			action_rect = rect_item->boundingRect();
			show_team_item = (MyItem*)rect_item->parent();
			show_team_id = show_team_item->id;
			if (rect_item)
			{
				deleteShowAction(rect_item);
			}
		}
		else
		{	
			//            MusicWidget* music_info = (MusicWidget*)this->parent();
			//            music_info->m_NextLinePosY-2-20
			int i = 0;      //尝试计算点击所在第几行
			i = m_triggerPos.y() / 30;
			i++;
			int hormid;     //计算垂直居中的y坐标值
			hormid = i * 30 - 5 - 20;    //20为场景矩形高度 5为上下边距
			//add by eagle for Debug action_rect = QRectF(m_triggerPos.x(), hormid, 30, 20);
			//第三个参数是宽度(这个才是我们的要计算的)，第四个是固定高度.
			//add by eagle
			// 这里增加2个逻辑。 1. 增加起始线， 2. m_movableVerticalLines 每次都要做一次排序插入
			//
			qreal startx, width = 30;
			qreal iheight = 20;
			rescalewidth(&startx, &hormid, &width, &iheight);
			action_rect = QRectF(startx, hormid, width, iheight);
		
			//add end	
		}

		if (show_team_id != 0)
		{
			//获取当前编曲的ID
			MusicWidget* music_info = (MusicWidget*)this->parent();
			int music_id = music_info->id;

			//获得编曲块的begin_time和end_time
			float begin_time = (action_rect.x() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
			float end_time = (action_rect.x() + action_rect.width() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
			begin_time *= 1000.0;
			end_time *= 1000.0;

			QSqlQuery query;
			QString qStrSql = QString("insert into music_action(type,begin_time,end_time)" "values(%1,%2,%3)").
				arg(ALWAYS_ON).arg(begin_time).arg(end_time);
			if (!query.exec(qStrSql))
			{
				qDebug() << "insert music_action false!sql:" << qStrSql <<
					" error:" << query.lastError().text();
				return;
			}
			qStrSql = QString("select distinct last_insert_rowid() from music_action");
			if (!query.exec(qStrSql) || !query.next())
			{
				qDebug() << "select last_insert_rowid from music_action false!sql:"
					<< qStrSql << " error:" << query.lastError().text();
				return;
			}
			int music_action_id = query.value(0).toInt();
			qStrSql = QString("insert into route(route_id,music_action_id,type,out_value1) values(0,%1,%2,255)").	//改为255
				arg(music_action_id).arg(ALWAYS_ON);
			if (!query.exec(qStrSql))
			{
				qDebug() << "insert into route false!sql:"
					<< qStrSql << " error:" << query.lastError().text();
				return;
			}
			qStrSql = QString("insert into music_mapping values(%1,%2,%3)").
				arg(music_id).arg(show_team_id).arg(music_action_id);
			if (!query.exec(qStrSql))
			{
				qDebug() << "insert music_mapping false!sql:"
					<< qStrSql << " error:" << query.lastError().text();
				return;
			}
			//新建常开动作
			CollidingRectItem* rect_item = new CollidingRectItem(ALWAYS_ON, action_rect,
				QColor(Qt::yellow),
				show_team_item);
			rect_item->setID(music_action_id);
			rect_item->setZValue(1);
			rect_item->setData(255);

			this->scene()->addItem(rect_item);
			m_music_actions.append(rect_item);
		}
	}
	else
		QMessageBox::information(this, tr("提示"),
		tr("未找到编队信息"), QMessageBox::Ok);
}

//DOMINO
void MusicGraphicsView::dominuo()
{
	if (m_IsPlay)
	{
		QMessageBox::information(this, tr("提示"),
			tr("当前处于播放状态，因此不允许修改表演动作！"),
			QMessageBox::Ok);
		return;
	}

	DominoSetting domino;
	domino.setModal(true);
	if (QDialog::Accepted == domino.exec())
	{
		//先获取编队的ID
		MyItem* show_team_item = (MyItem*)this->itemAt(m_triggerPos);	//m_triggerPos鼠标点击点//坐标，返回编队信息(id号等）
		// MyItem* show_team_item = reinterpret_cast <MyItem*>(this->itemAt(m_triggerPos));
		//show_team_item->m_Show_Team_list = "0";
		if (show_team_item)
		{	//有效的id号，不是0
			QRectF rect;
			int show_team_id = show_team_item->id;		//获得编队的id号
			if (show_team_id == 0)
			{
				CollidingRectItem* rect_item =
					(CollidingRectItem*)this->itemAt(m_triggerPos);
				rect = rect_item->boundingRect();
				show_team_item = (MyItem*)rect_item->parent();
 				show_team_id = show_team_item->id;
				if (rect_item)
				{
					deleteShowAction(rect_item);
				}
			}
			else
			{
				int i = 0;      //尝试计算点击所在第几行
				i = m_triggerPos.y() / 30;
				i++;
				int hormid;     //计算垂直居中的y坐标值
				hormid = i * 30 - 5 - 20;    //20为场景矩形高度 5为上下边距
				
				//add by eagle
				// 这里增加2个逻辑。 1. 增加起始线， 2. m_movableVerticalLines 每次都要做一次排序插入
				//
				int index1 = -1;
				int index2 = -1;
				float minDistance = std::numeric_limits<float>::max();

				// Find the two 分割线objects between which the mouse click occurred
				for (int i = 0; i < m_cutsolidlines.size() - 1; ++i) {
					float leftX = m_cutsolidlines[i]->getX();
					float rightX = m_cutsolidlines[i + 1]->getX();

					if (m_triggerPos.x() >= leftX && m_triggerPos.x() <= rightX) {
						float distance = rightX - leftX;
						if (distance < minDistance) {
							index1 = i;
							index2 = i + 1;
							minDistance = distance;
						}
					}
				}

				// Calculate the x-coordinate and width of the action_rect
				qreal startx = m_triggerPos.x(); // Default x-coordinate
				qreal width = 30; // Default width

				if (index1 != -1 && index2 != -1) {
					startx = m_cutsolidlines[index1]->getX();
					width = m_cutsolidlines[index2]->getX() - startx;
				}

				//action_rect = QRectF(startx, hormid, width, 20);
				rect = QRectF(startx, hormid, width, 20);		//获得鼠标点击区域的方块大小和坐标
				//add end 
			}

			if (show_team_id != 0)
			{
				//获取当前编曲的ID
				MusicWidget* music_info = (MusicWidget*)this->parent();
				int music_id = music_info->id;

				//获得编曲块的begin_time和end_time
				float begin_time = (rect.x() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
				float end_time = (rect.x() + rect.width() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
				begin_time *= 1000.0;
				end_time *= 1000.0;

				QSqlQuery query;
			//	QString qStrSql = QString("insert into music_action(type,begin_time,end_time)"		//写入type到music_action，只写入了type，没有写begin_time，end_time
			//		"values(%1,%2,%3)").arg(DOMINO_RUN).arg(begin_time).arg(end_time);				//目前是点击保存的时候才会存储begin_time，end_time

			//	QString qStrSql = QString("insert into music_action values(%1,%2,%3)").
			//		arg(DOMINO_RUN).arg(begin_time).arg(end_time);

				QString qStrSql = QString("insert into music_action(type,begin_time,end_time)" "values(%1,%2,%3)").
					arg(DOMINO_RUN).arg(begin_time).arg(end_time);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_action false!sql:" << qStrSql <<
						" error:" << query.lastError().text();
					return;
				}

				qStrSql = QString("select distinct last_insert_rowid() from music_action");	//找到最后位置？
				if (!query.exec(qStrSql) || !query.next())
				{
					qDebug() << "select last_insert_rowid from music_action false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}
				int music_action_id = query.value(0).toInt();	//获得music_action_id号
				qStrSql = QString("insert into route(route_id,music_action_id,type,keep_time,"
					"delay_time,perial_time,loop_time,start_delay,out_value1,out_value2,reverse_flag,from_left) values(0,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11)").
					arg(music_action_id).arg(DOMINO_RUN).
					arg(domino.m_iKeepTime).arg(domino.m_iDelayTime).
					arg(domino.m_iPerialTime).arg(domino.m_iLoopTimes).arg(domino.m_iStartDelay).
					arg(domino.m_iOutputValue1).arg(domino.m_iOutputValue2).
					arg(domino.m_iReverseFlag).arg(domino.m_iFromLeft);
				if (!query.exec(qStrSql))						//写入route
				{
					qDebug() << "insert music_action false!sql:" << qStrSql <<
						" error:" << query.lastError().text();
					return;
				}
				qStrSql = QString("insert into music_mapping values(%1,%2,%3)").
					arg(music_id).arg(show_team_id).arg(music_action_id);
				if (!query.exec(qStrSql))						//写入music_mapping
				{
					qDebug() << "insert music_mapping false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}

				//屏幕显示
				CollidingRectItem* rect_item = new CollidingRectItem(DOMINO_RUN, rect,
					QColor(Qt::yellow),
					show_team_item);
				rect_item->setID(music_action_id);
				rect_item->setZValue(1);
				rect_item->setData(domino.m_iKeepTime, domino.m_iDelayTime,
					domino.m_iPerialTime, domino.m_iLoopTimes, domino.m_iOutputValue1, domino.m_iOutputValue2, domino.m_iStartDelay,
					domino.m_iReverseFlag, domino.m_iFromLeft);
				this->scene()->addItem(rect_item);
				m_music_actions.append(rect_item);
			}
		}
	}
	else
		QMessageBox::information(this, tr("提示"),
		tr("未找到编队信息"), QMessageBox::Ok);
}

//IMAGE，添加图片的对话框操作
void MusicGraphicsView::addImage()
{
	if (m_IsPlay)
	{
		QMessageBox::information(this, tr("提示"),
			tr("当前处于播放状态，因此不允许修改表演动作！"),
			QMessageBox::Ok);
		return;
	}
	SelectImageDialog image_sel;		//启动选择图片的对话框
	image_sel.setModal(true);
	if (QDialog::Accepted == image_sel.exec())
	{
		//先获取编队的ID
		MyItem* show_team_item = (MyItem*)this->itemAt(m_triggerPos);
		if (show_team_item)
		{
			QRectF rect;
			int show_team_id = show_team_item->id;
			if (show_team_id == 0)
			{
				CollidingRectItem* rect_item =
					(CollidingRectItem*)this->itemAt(m_triggerPos);
				rect = rect_item->boundingRect();
				show_team_item = (MyItem*)rect_item->parent();
				show_team_id = show_team_item->id;
				if (rect_item)
				{
					deleteShowAction(rect_item);
				}
			}
			else
			{
				int i = 0;      //尝试计算点击所在第几行
				i = m_triggerPos.y() / 30;
				i++;
				int hormid;     //计算垂直居中的y坐标值
				hormid = i * 30 - 5 - 20;    //20为场景矩形高度 5为上下边距
				//add by eagle
				// 这里增加2个逻辑。 1. 增加起始线， 2. m_movableVerticalLines 每次都要做一次排序插入
				//
				int index1 = -1;
				int index2 = -1;
				float minDistance = std::numeric_limits<float>::max();

				// Find the two 分割线objects between which the mouse click occurred
				for (int i = 0; i < m_cutsolidlines.size() - 1; ++i) {
					float leftX = m_cutsolidlines[i]->getX();
					float rightX = m_cutsolidlines[i + 1]->getX();

					if (m_triggerPos.x() >= leftX && m_triggerPos.x() <= rightX) {
						float distance = rightX - leftX;
						if (distance < minDistance) {
							index1 = i;
							index2 = i + 1;
							minDistance = distance;
						}
					}
				}

				// Calculate the x-coordinate and width of the action_rect
				qreal startx = m_triggerPos.x(); // Default x-coordinate
				qreal width = 30; // Default width

				if (index1 != -1 && index2 != -1) {
					startx = m_cutsolidlines[index1]->getX();
					width = m_cutsolidlines[index2]->getX() - startx;
				}

				//action_rect = QRectF(startx, hormid, width, 20);
				rect = QRectF(startx, hormid, width, 20);
				//add end
				//rect = QRectF(m_triggerPos.x(), hormid, 30, 20);
			}
			if (show_team_id != 0)
			{
				//获取当前编曲的ID
				MusicWidget* music_info = (MusicWidget*)this->parent();
				int music_id = music_info->id;

				//获得编曲块的begin_time和end_time
				float begin_time = (rect.x() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
				float end_time = (rect.x() + rect.width() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
				begin_time *= 1000.0;
				end_time *= 1000.0;

				QSqlQuery query;
				QString qStrSql = QString("insert into music_action(type,begin_time,end_time)" "values(%1,%2,%3)").
					arg(IMAGE_SHOW).arg(begin_time).arg(end_time);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_action false!sql:" << qStrSql <<
						" error:" << query.lastError().text();
					return;
				}
				qStrSql = QString("select distinct last_insert_rowid() from music_action");
				if (!query.exec(qStrSql) || !query.next())
				{
					qDebug() << "select last_insert_rowid from music_action false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}
				int music_action_id = query.value(0).toInt();
				qStrSql = QString("insert into route(route_id,music_action_id,type,delay_time,loop_time,img_path,yang_wen)"
					" values(0,%1,%2,%3,%4,'%5',%6)").arg(music_action_id).
					arg(IMAGE_SHOW).arg(image_sel.m_DelayTime).
					arg(image_sel.m_LoopTime).arg(image_sel.m_ImagePath).
					arg(image_sel.m_IsYangWen);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert route false!sql:" << qStrSql <<
						" error:" << query.lastError().text();
					return;
				}
				qStrSql = QString("insert into music_mapping values(%1,%2,%3)").
					arg(music_id).arg(show_team_id).arg(music_action_id);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_mapping false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}


				CollidingRectItem* rect_item = new CollidingRectItem(IMAGE_SHOW, rect,
					QColor(Qt::yellow),
					show_team_item);
				rect_item->setID(music_action_id);
				rect_item->setZValue(1);
				rect_item->setData(image_sel.m_ImagePath, image_sel.m_DelayTime,
					image_sel.m_LoopTime, image_sel.m_IsYangWen);

				if (image_sel.m_LoopTime != -1)
				{
#if 0
					//delaytime*height*times
					qDebug() << rect_item->m_show_teams.size();
					image_sel.new_image = image_sel.m_image.scaledToWidth(rect_item->m_show_teams.size());       //计算缩放后的图像
					int height = image_sel.new_image.height();
					int delaytime = StaticValue::GetInstance()->str_ipaimageDelayTime.toInt();
					int looptimes = image_sel.m_LoopTime;

					//预期的图片表演时间
					qreal estimatedtime = height*delaytime*looptimes;

					int width = (estimatedtime / 1000.0)* (music_info->m_EditView - 30) / music_info->time_length;

					qDebug() << estimatedtime / 1000.0;
					qDebug() << music_info->m_EditView - 30;
					qDebug() << music_info->time_length;

					rect.setWidth(width);
					rect_item->ResetRect(rect);                 //更新长度
#endif
				}

				this->scene()->addItem(rect_item);

				//float begin_time = (pos_x - m_HeaderWidth)/m_balance*1000;
				//float end_time = begin_time + width/m_balance*1000;
				m_music_actions.append(rect_item);
			}
		}
	}
	else
		QMessageBox::information(this, tr("提示"),
		tr("未找到编队信息"), QMessageBox::Ok);
}

//DMX512
void MusicGraphicsView::changeLight()
{
	if (m_IsPlay)
	{
		QMessageBox::information(this, tr("提示"),
			tr("当前处于播放状态，因此不允许修改表演动作！"),
			QMessageBox::Ok);
		return;
	}
	DMXLightDialog dmx_light;
	dmx_light.setModal(true);
	if (QDialog::Accepted == dmx_light.exec())
	{
		//先获取编队的ID
		MyItem* show_team_item = (MyItem*)this->itemAt(m_triggerPos);
		if (show_team_item)
		{
			QRectF rect;
			int show_team_id = show_team_item->id;
			if (show_team_id == 0)
			{
				CollidingRectItem* rect_item =
					(CollidingRectItem*)this->itemAt(m_triggerPos);
				rect = rect_item->boundingRect();
				show_team_item = (MyItem*)rect_item->parent();
				show_team_id = show_team_item->id;
				if (rect_item)
				{
					deleteShowAction(rect_item);
				}
			}
			else
			{
				int i = 0;      //尝试计算点击所在第几行
				i = m_triggerPos.y() / 30;
				i++;
				int hormid;     //计算垂直居中的y坐标值
				hormid = i * 30 - 5 - 20;    //20为场景矩形高度 5为上下边距
				//add by eagle
				// 这里增加2个逻辑。 1. 增加起始线， 2. m_movableVerticalLines 每次都要做一次排序插入
				//
				int index1 = -1;
				int index2 = -1;
				float minDistance = std::numeric_limits<float>::max();

				// Find the two 分割线objects between which the mouse click occurred
				for (int i = 0; i < m_cutsolidlines.size() - 1; ++i) {
					float leftX = m_cutsolidlines[i]->getX();
					float rightX = m_cutsolidlines[i + 1]->getX();

					if (m_triggerPos.x() >= leftX && m_triggerPos.x() <= rightX) {
						float distance = rightX - leftX;
						if (distance < minDistance) {
							index1 = i;
							index2 = i + 1;
							minDistance = distance;
						}
					}
				}

				// Calculate the x-coordinate and width of the action_rect
				qreal startx = m_triggerPos.x(); // Default x-coordinate
				qreal width = 30; // Default width

				if (index1 != -1 && index2 != -1) {
					startx = m_cutsolidlines[index1]->getX();
					width = m_cutsolidlines[index2]->getX() - startx;
				}

				//action_rect = QRectF(startx, hormid, width, 20);
				rect = QRectF(startx, hormid, width, 20);
				//add end
				//rect = QRectF(m_triggerPos.x(), hormid, 30, 20);
			}

			if (show_team_id != 0)
			{
				//获取当前编曲的ID
				MusicWidget* music_info = (MusicWidget*)this->parent();
				int music_id = music_info->id;

				//获得编曲块的begin_time和end_time
				float begin_time = (rect.x() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
				float end_time = (rect.x() + rect.width() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
				begin_time *= 1000.0;
				end_time *= 1000.0;

				QSqlQuery query;
				QString qStrSql = QString("insert into music_action(type,begin_time,end_time)" "values(%1,%2,%3)").
					arg(DMX512_SHOW).arg(begin_time).arg(end_time);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_action false!sql:" << qStrSql <<
						" error:" << query.lastError().text();
					return;
				}
				qStrSql = QString("select distinct last_insert_rowid() from music_action");
				if (!query.exec(qStrSql) || !query.next())
				{
					qDebug() << "select last_insert_rowid from music_action false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}
				int music_action_id = query.value(0).toInt();
								QList<int> keys = dmx_light.m_route_setting.keys();
			//	QList<int> keys = dmx_light.user_route_setting.keys();
                for(int i = 0 ; i < keys.size(); ++i)
                {
                    int route_id = keys.at(i);
					RouteInfo* dmx_set = dmx_light.m_route_setting[route_id];
				//	RouteInfo* dmx_set = dmx_light.user_route_setting[route_id];

                    qStrSql = QString("insert into route(route_id,music_action_id,type,out_value1,out_value2,keep_time,delay_time,perial_time,loop_time,start_delay,orbit_id,change_speed,len1,len2,speed,fade_in,fade_out,reverse_flag,from_left)"
                                      "values(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19)").arg(route_id).arg(music_action_id).arg(dmx_set->type).arg(dmx_set->out_value1).arg(dmx_set->out_value2).
                            arg(dmx_set->keep_time).arg(dmx_set->delay_time).arg(dmx_set->perial_time).arg(dmx_set->loop_time).arg(dmx_set->start_delay).arg(dmx_set->orbit_id).arg(dmx_set->change_speed).
                            arg(dmx_set->len1).arg(dmx_set->len2).arg(dmx_set->speed).
							arg(dmx_set->fade_in).arg(dmx_set->fade_out).arg(dmx_set->reverse_flag).arg(dmx_set->from_left);
					if (!query.exec(qStrSql))
					{
						qDebug() << "select last_insert_rowid from music_action false!sql:"
							<< qStrSql << " error:" << query.lastError().text();
						break;
					}
				}
				qStrSql = QString("insert into music_mapping values(%1,%2,%3)").
					arg(music_id).arg(show_team_id).arg(music_action_id);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_mapping false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}

				CollidingRectItem* rect_item = new CollidingRectItem(DMX512_SHOW, rect,
					QColor(Qt::yellow),
					show_team_item);
				rect_item->setID(music_action_id);
				rect_item->setZValue(1);
				rect_item->setData(dmx_light.m_route_setting);
			//	rect_item->setData(dmx_light.user_route_setting);
				this->scene()->addItem(rect_item);
				m_music_actions.append(rect_item);
			}
		}
		else
			QMessageBox::information(this, tr("提示"),
			tr("未找到编队信息"), QMessageBox::Ok);
	}
}

//Clock时钟对应的槽函数，区分大小写
void MusicGraphicsView::addclock()
{
	if (m_IsPlay)
	{
		QMessageBox::information(this, tr("提示"),
			tr("当前处于播放状态，因此不允许修改表演动作！"),
			QMessageBox::Ok);
		return;
	}
	ClockDialog clock_sel;
	clock_sel.setModal(true);
	if (QDialog::Accepted == clock_sel.exec())
	{
		//先获取编队的ID
		MyItem* show_team_item = (MyItem*)this->itemAt(m_triggerPos);
		if (show_team_item)
		{
			QRectF rect;
			int show_team_id = show_team_item->id;
			if (show_team_id == 0)
			{
				CollidingRectItem* rect_item =
					(CollidingRectItem*)this->itemAt(m_triggerPos);
				rect = rect_item->boundingRect();
				show_team_item = (MyItem*)rect_item->parent();
				show_team_id = show_team_item->id;
				if (rect_item)
				{
					deleteShowAction(rect_item);
				}
			}
			else
			{
				int i = 0;      //尝试计算点击所在第几行
				i = m_triggerPos.y() / 30;
				i++;
				int hormid;     //计算垂直居中的y坐标值
				hormid = i * 30 - 5 - 20;    //20为场景矩形高度 5为上下边距
				//add by eagle
				// 这里增加2个逻辑。 1. 增加起始线， 2. m_movableVerticalLines 每次都要做一次排序插入
				//
				int index1 = -1;
				int index2 = -1;
				float minDistance = std::numeric_limits<float>::max();

				// Find the two 分割线objects between which the mouse click occurred
				for (int i = 0; i < m_cutsolidlines.size() - 1; ++i) {
					float leftX = m_cutsolidlines[i]->getX();
					float rightX = m_cutsolidlines[i + 1]->getX();

					if (m_triggerPos.x() >= leftX && m_triggerPos.x() <= rightX) {
						float distance = rightX - leftX;
						if (distance < minDistance) {
							index1 = i;
							index2 = i + 1;
							minDistance = distance;
						}
					}
				}

				// Calculate the x-coordinate and width of the action_rect
				qreal startx = m_triggerPos.x(); // Default x-coordinate
				qreal width = 30; // Default width

				if (index1 != -1 && index2 != -1) {
					startx = m_cutsolidlines[index1]->getX();
					width = m_cutsolidlines[index2]->getX() - startx;
				}

				//action_rect = QRectF(startx, hormid, width, 20);
				rect = QRectF(startx, hormid, width, 20);
				//add end
				//rect = QRectF(m_triggerPos.x(), hormid, 30, 20);
			}
			if (show_team_id != 0)
			{
				//获取当前编曲的ID
				MusicWidget* music_info = (MusicWidget*)this->parent();
				int music_id = music_info->id;

				//获得编曲块的begin_time和end_time
				float begin_time = (rect.x() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
				float end_time = (rect.x() + rect.width() - music_info->m_HeaderWidth)*music_info->time_length / (music_info->m_EditView - 30.0);
				begin_time *= 1000.0;
				end_time *= 1000.0;

				QSqlQuery query;
				QString qStrSql = QString("insert into music_action(type,begin_time,end_time)" "values(%1,%2,%3)").
					arg(CLOCK_SHOW).arg(begin_time).arg(end_time);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_action false!sql:" << qStrSql <<
						" error:" << query.lastError().text();
					return;
				}
				qStrSql = QString("select distinct last_insert_rowid() from music_action");
				if (!query.exec(qStrSql) || !query.next())
				{
					qDebug() << "select last_insert_rowid from music_action false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}

				//给sql数据库的route项增加一列ClockStyle，缺省值是0，类型是int
				//	qStrSql = QString("alter table route add column ClockStyle int not null default 0");
				//给sql数据库的route项增加一列TimeLeadValue，缺省值是0，类型是int
				//	qStrSql = QString("alter table route add column TimeLeadValue int not null default 0");

				int music_action_id = query.value(0).toInt();

				qStrSql = QString("insert into route(route_id,music_action_id,type,delay_time,loop_time,ClockStyle,yang_wen,TimeLeadValue)"
					" values(0,%1,%2,%3,%4,'%5',%6,%7)").
					arg(music_action_id).			//1.music_action_id
				//	arg(IMAGE_SHOW).				//2.type
					arg(CLOCK_SHOW).				//2.type
					arg(clock_sel.m_DelayTime).		//3.delay_time
					arg(clock_sel.m_LoopTime).		//4.loop_time
				//	arg(clock_sel.m_ImagePath).		//5.img_path
					arg(clock_sel.m_ClockStyleSel).	//5.ClockStyle
					arg(clock_sel.m_IsYangWen).		//6.yang_wen
					arg(clock_sel.m_TimeLeadValue);	//7.TimeLeadValue
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert route false!sql:" << qStrSql <<
						" error:" << query.lastError().text();
					return;
				}
				qStrSql = QString("insert into music_mapping values(%1,%2,%3)").
					arg(music_id).arg(show_team_id).arg(music_action_id);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_mapping false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}

				CollidingRectItem* rect_item = new CollidingRectItem(CLOCK_SHOW, rect,
					QColor(Qt::yellow),
					show_team_item);
				rect_item->setID(music_action_id);
				rect_item->setZValue(1);
				rect_item->setData(clock_sel.m_ClockStyleSel, clock_sel.m_TimeLeadValue, clock_sel.m_DelayTime,
					clock_sel.m_LoopTime, clock_sel.m_IsYangWen);

				if (clock_sel.m_LoopTime != -1)
				{
#if 0
					//delaytime*height*times
					qDebug() << rect_item->m_show_teams.size();
					image_sel.new_image = image_sel.m_image.scaledToWidth(rect_item->m_show_teams.size());       //计算缩放后的图像
					int height = image_sel.new_image.height();
					int delaytime = StaticValue::GetInstance()->str_ipaimageDelayTime.toInt();
					int looptimes = image_sel.m_LoopTime;

					//预期的图片表演时间
					qreal estimatedtime = height*delaytime*looptimes;

					int width = (estimatedtime / 1000.0)* (music_info->m_EditView - 30) / music_info->time_length;

					qDebug() << estimatedtime / 1000.0;
					qDebug() << music_info->m_EditView - 30;
					qDebug() << music_info->time_length;

					rect.setWidth(width);
					rect_item->ResetRect(rect);                 //更新长度
#endif
				}

				this->scene()->addItem(rect_item);

				//float begin_time = (pos_x - m_HeaderWidth)/m_balance*1000;
				//float end_time = begin_time + width/m_balance*1000;
				m_music_actions.append(rect_item);
			}
		}
	}
	else
		QMessageBox::information(this, tr("提示"),
		tr("未找到编队信息"), QMessageBox::Ok);
}

//复制
void MusicGraphicsView::WidgetCopy()
{
	if (m_IsPlay)
	{
		QMessageBox::information(this, tr("提示"),
			tr("当前处于播放状态，因此不允许修改表演动作！"),
			QMessageBox::Ok);
		return;
	}

	QRectF rect;

	int i = 0;      //尝试计算点击所在第几行
	i = m_triggerPos.y() / 30;
	i++;
	int hormid;     //计算垂直居中的y坐标值
	hormid = i * 30 - 5 - 20;    //20为场景矩形高度 5为上下边距

	//add by eagle
	// 这里增加2个逻辑。 1. 增加起始线， 2. m_movableVerticalLines 每次都要做一次排序插入
	//
	int index1 = -1;
	int index2 = -1;
	float minDistance = std::numeric_limits<float>::max();

	// Find the two 分割线objects between which the mouse click occurred
	for (int i = 0; i < m_cutsolidlines.size() - 1; ++i)
	{
		float leftX = m_cutsolidlines[i]->getX();
		float rightX = m_cutsolidlines[i + 1]->getX();

		if (m_triggerPos.x() >= leftX && m_triggerPos.x() <= rightX)
		{
			float distance = rightX - leftX;
			if (distance < minDistance)
			{
				index1 = i;
				index2 = i + 1;
				minDistance = distance;
			}
		}
	}
	// Calculate the x-coordinate and width of the action_rect
	qreal startx = m_triggerPos.x(); // Default x-coordinate
	qreal width = 30; // Default width

	if (index1 != -1 && index2 != -1) 
	{
		startx = m_cutsolidlines[index1]->getX();
		width = m_cutsolidlines[index2]->getX() - startx;
	}

	//action_rect = QRectF(startx, hormid, width, 20);
	rect = QRectF(startx, hormid, width, 20);

	CollidingRectItem* rect_item = (CollidingRectItem*)this->itemAt(m_triggerPos);

	//add end

/*
//	DominoSetting domino;
//	domino.setModal(true);
//	if (QDialog::Accepted == domino.exec())
//	{
		//先获取编队的ID
		MyItem* show_team_item = (MyItem*)this->itemAt(m_triggerPos);
		if (show_team_item)
		{
			QRectF rect;
			int show_team_id = show_team_item->id;
			if (show_team_id == 0)
			{
				CollidingRectItem* rect_item =
					(CollidingRectItem*)this->itemAt(m_triggerPos);
				rect = rect_item->boundingRect();
				show_team_item = (MyItem*)rect_item->parent();
				show_team_id = show_team_item->id;
				if (rect_item)
				{
					deleteShowAction(rect_item);
				}
			}
			else
			{
				int i = 0;      //尝试计算点击所在第几行
				i = m_triggerPos.y() / 30;
				i++;
				int hormid;     //计算垂直居中的y坐标值
				hormid = i * 30 - 5 - 20;    //20为场景矩形高度 5为上下边距

				//add by eagle
				// 这里增加2个逻辑。 1. 增加起始线， 2. m_movableVerticalLines 每次都要做一次排序插入
				//
				int index1 = -1;
				int index2 = -1;
				float minDistance = std::numeric_limits<float>::max();

				// Find the two 分割线objects between which the mouse click occurred
				for (int i = 0; i < m_cutsolidlines.size() - 1; ++i) {
					float leftX = m_cutsolidlines[i]->getX();
					float rightX = m_cutsolidlines[i + 1]->getX();

					if (m_triggerPos.x() >= leftX && m_triggerPos.x() <= rightX) {
						float distance = rightX - leftX;
						if (distance < minDistance) {
							index1 = i;
							index2 = i + 1;
							minDistance = distance;
						}
					}
				}

				// Calculate the x-coordinate and width of the action_rect
				qreal startx = m_triggerPos.x(); // Default x-coordinate
				qreal width = 30; // Default width

				if (index1 != -1 && index2 != -1) {
					startx = m_cutsolidlines[index1]->getX();
					width = m_cutsolidlines[index2]->getX() - startx;
				}

				//action_rect = QRectF(startx, hormid, width, 20);
				rect = QRectF(startx, hormid, width, 20);
				//add end 
			}

			if (show_team_id != 0)
			{
				//获取当前编曲的ID
				MusicWidget* music_info = (MusicWidget*)this->parent();
				mc_music_id = music_info->id;

			//	QVector<CollidingRectItem*> items = m_MusicView->getCollidingItems();

			//	MusicWidget* music_info = (MusicWidget*)this->parent();
			//	mc_music_id = music_info->getMusicActions;

				//然后获得点中的编曲块的Music_action_id，动作序列号，存储在music_mapping里面，忽略music_mapping里面的MUSIC_ID和Show_team_id
			//	mc_music_action_id = music_info->getMusicID();
			//	mc_run_type = 

				QSqlQuery query;
				QString qStrSql = QString("insert into music_action(type)"
					" values(%1)").arg(DOMINO_RUN);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_action false!sql:" << qStrSql <<
						" error:" << query.lastError().text();
					return;
				}
				qStrSql = QString("select distinct last_insert_rowid() from music_action");
				if (!query.exec(qStrSql) || !query.next())
				{
					qDebug() << "select last_insert_rowid from music_action false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}
				int music_action_id = query.value(0).toInt();
				qStrSql = QString("insert into route(route_id,music_action_id,type,keep_time,"
					"delay_time,perial_time,loop_time,start_delay,out_value1,out_value2,reverse_flag,from_left) values(0,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11)").
					arg(music_action_id).arg(DOMINO_RUN).
					arg(domino.m_iKeepTime).arg(domino.m_iDelayTime).
					arg(domino.m_iPerialTime).arg(domino.m_iLoopTimes).arg(domino.m_iStartDelay).
					arg(domino.m_iOutputValue1).arg(domino.m_iOutputValue2).
					arg(domino.m_iReverseFlag).arg(domino.m_iFromLeft);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_action false!sql:" << qStrSql <<
						" error:" << query.lastError().text();
					return;
				}
				qStrSql = QString("insert into music_mapping values(%1,%2,%3)").
					arg(music_id).arg(show_team_id).arg(music_action_id);
				if (!query.exec(qStrSql))
				{
					qDebug() << "insert music_mapping false!sql:"
						<< qStrSql << " error:" << query.lastError().text();
					return;
				}

				CollidingRectItem* rect_item = new CollidingRectItem(DOMINO_RUN, rect,
					QColor(Qt::yellow),
					show_team_item);
				rect_item->setID(music_action_id);
				rect_item->setZValue(1);
				rect_item->setData(domino.m_iKeepTime, domino.m_iDelayTime,
					domino.m_iPerialTime, domino.m_iLoopTimes, domino.m_iOutputValue1, domino.m_iOutputValue2, domino.m_iStartDelay,
					domino.m_iReverseFlag, domino.m_iFromLeft);
				this->scene()->addItem(rect_item);
				m_music_actions.append(rect_item); 
			}
	//	}
	}
	else
		QMessageBox::information(this, tr("提示"),
		tr("未找到编队信息"), QMessageBox::Ok);  */
}

//粘贴
void MusicGraphicsView::WidgetPaste()
{

}

void MusicGraphicsView::delscence()
{
	Q_ASSERT(m_RightClickItem != nullptr);
    deleteShowAction(m_RightClickItem);
}

bool MusicGraphicsView::compareMovableVerticalLine(CutSolidVerticalLine *a, CutSolidVerticalLine *b)
{
 return a->getX() < b->getX();
}
