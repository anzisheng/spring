#include "MovableVerticalLine.h"
#include <QKeyEvent>
#include <QDebug>
#include <QGraphicsScene>


MovableVerticalLine::MovableVerticalLine(qreal x, qreal y1, qreal y2, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_x(x), m_y1(y1), m_y2(y2)
{
	setFlag(QGraphicsItem::ItemIsFocusable, true);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);////QGraphicsItem::ItemSendsGeometryChanges使itemChange()函数有效。
	setAcceptHoverEvents(true);
	//textmusictimeitem = new QGraphicsSimpleTextItem(this);
	//textmusictimeitem->setPos(m_x, m_y1/2); // Adjust the position as needed
	//textmusictimeitem->setText("0:00");
	
	
	textmusictimeitem1 = new QGraphicsSimpleTextItem(this);
	textmusictimeitem1->setPos(m_x, m_y1 / 2 + 20); // Adjust the position as needed
	textmusictimeitem1->setText("0:00");
}

QRectF MovableVerticalLine::boundingRect() const
{
//	qreal margin = 2;
//	return QRectF(m_x - 1 - margin, m_y1 - margin, 2 + 2 * margin, m_y2 - m_y1 + 2 * margin);
	return QRectF(m_x, m_y1, 0, m_y2 - m_y1);
}

void MovableVerticalLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
	QPen pen;
	pen.setStyle(Qt::DashLine);
	painter->setPen(pen);
    painter->drawLine(m_x, m_y1, m_x, m_y2);
}

void MovableVerticalLine::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left)
    {		
		//qDebug() << "MovableVerticalLine::keyPressEvent" << m_x;
		if (m_x <= 150)  //不允许虚线负相对原点
		{
			m_x = 150;
		}
		else
		{
			m_x -= 1;
			this->moveBy(-1, 0);
			setTimeText(m_x);
		}
        update(); 
    }
	else if (event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_P)
	{
		//qDebug() << "MovableVerticalLine::keyPressEvent" << m_x;
		if (m_x <= 150)  //不允许虚线负相对原点
		{
			m_x = 150;
		}
		else
		{
			m_x -= 10;
			this->moveBy(-10, 0);
			setTimeText(m_x);
		}
		update();
	}
	else if (event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_O)
	{
		//qDebug() << "MovableVerticalLine::keyPressEvent" << m_x;
		if (m_x <= 150)  //不允许虚线负相对原点
		{
			m_x = 150;
		}
		else
		{
			m_x += 10;
			this->moveBy(+10, 0);
			setTimeText(m_x);
		}
		update();
	}
	else if (event->key() == Qt::Key_Right)
	{
		if (m_x < (maxx + 150))
		{
			m_x += 1;
			this->moveBy(1, 0);

			setTimeText(m_x);
		//	qDebug() << "MovableVerticalLine::keyPressEvent" << m_x;
		}
		update();
    }
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        // 固定位置后，取消焦点以防止进一步的键盘事件
        //setFlag(QGraphicsItem::ItemIsFocusable, false);
        clearFocus();
		//qDebug() << "MovableVerticalLine::keyPressEvent: 在虚线的位置画实线" << m_x;;
		//emit signalcutsolid(m_x);
		emit signalcutsolid((m_x-150)*2);
		event->accept(); // Accept the event to prevent it from propagating to the main UI
    }
	else if (event->key() == Qt::Key_Escape) //ESC 虚线回到相对原点
	{
		if (m_x >= 150)
		{
			m_x =  150;
			setPos(0, y());		//setPos的x的参数是0-maxx，0在起点，maxx在终点，和m_x是不一样的

			setTimeText(m_x);
		//	qDebug() << "MovableVerticalLine::keyPressEvent" << m_x;
		}		
		update();
	}
	else if (event->key() == Qt::Key_End) //END 虚线到终点位置
	{
	//	if (m_x <= (maxx + 150))
	//	{
			m_x = maxx + 150;
			setPos(maxx, y());	//setPos的x的参数是0-maxx，0在起点，maxx在终点，和m_x是不一样的

			setTimeText(m_x);
		//	qDebug() << "MovableVerticalLine::keyPressEvent" << m_x;
	//	}
		update();
	}
	else
	{
		QGraphicsItem::keyPressEvent(event);
	}
	
	this->scene()->update();
    
}
//void MovableVerticalLine::keyReleaseEvent(QKeyEvent *event)
//{
//	if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
//	{
//		qDebug() << "MovableVerticalLine::keyReleaseEvent" ;
//		update();
//	}
//}
QVariant MovableVerticalLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionChange)
	{
		//QPointF newPos = value.toPointF();
		//newPos.setY(y());
		//return newPos;
	}
	else if (change == ItemPositionHasChanged)
	{
		
	}

    return QGraphicsItem::itemChange(change, value);
}
qreal MovableVerticalLine::getX() const
{
     return m_x;
}

qreal MovableVerticalLine::gety1() const
{
	return m_y1;
}

qreal MovableVerticalLine::gety2() const
{
    return m_y2;
}

void MovableVerticalLine::setX(qreal posx)
{
	m_x = posx;
}

void MovableVerticalLine::sety1(qreal posy1)
{
	m_y1 = posy1;
}
void MovableVerticalLine::sety2(qreal posy2)
{
	m_y2 = posy2;
}
void MovableVerticalLine::setTimeText(qreal musicLength)
{
//	int maxx = (m_maxx-150)/2-10;
//	int temp = scene()->width();	//屏幕1280宽度的时候，值1573；屏幕1920宽度的时候，值不变
	qreal positionRatio = (m_x - 150) / maxx;
//	qreal musicTime = 2*positionRatio * m_timelength; //场景坐标要*2 坐标才对上
	qreal musicTime = positionRatio * m_timelength; //场景坐标要*2 坐标才对上
	//textmusictimeitem->setText(QString::number(musicTime));
	//textmusictimeitem->setPos(m_x, m_y1-20);


	int minutes = static_cast<int>(musicTime) / 60;
	int seconds = (static_cast<int>(musicTime) % 60);
	int milliseconds = static_cast<int>((musicTime - static_cast<int>(musicTime)) * 1000);

	QString timeText = QString("%1:%2.%3")
		.arg(minutes, 2, 10, QChar('0'))
		.arg(seconds, 2, 10, QChar('0'))
		.arg(milliseconds, 3, 10, QChar('0'));

	textmusictimeitem1->setText(timeText);
	textmusictimeitem1->setPos(m_x-80, m_y1 - 25);
}

void MovableVerticalLine::setMusiclength(float ftime_length)
{
	m_timelength = ftime_length;
}

void MovableVerticalLine::setlineMaxX(qreal posmaxx)
{
	m_maxx = posmaxx;
	maxx = (m_maxx - 150) / 2;
}
qreal MovableVerticalLine::getlineMaxX(void)
{
	return m_maxx;
}
