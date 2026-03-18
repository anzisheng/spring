#include "CutSolidVerticalLine.h"
#include <QKeyEvent>
#include <QDebug>
#include <QGraphicsScene>
#include <QPainterPathStroker>
#include <QGuiApplication>
#include <QScreen>
#include <QStyleOptionGraphicsItem>
#include <QStyle>

CutSolidVerticalLine::CutSolidVerticalLine(qreal x, qreal y1, qreal y2, qreal time, QGraphicsItem *parent)
	: QGraphicsItem(parent), m_x(x), m_y1(y1), m_y2(y2), m_time(time)
{
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
}

QRectF CutSolidVerticalLine::boundingRect() const
{
	const qreal lineWidth = 6.0;
	const qreal halfWidth = lineWidth / 2.0;
	return QRectF(m_x - halfWidth, m_y1, lineWidth, m_y2 - m_y1);
}

void CutSolidVerticalLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    QPen pen;
	pen.setStyle(Qt::SolidLine);
	if (option && (option->state & QStyle::State_Selected))
	{
		pen.setColor(QColor(255, 215, 0));
		pen.setWidthF(2.0);
	}
	else
	{
		pen.setColor(Qt::red);
		pen.setWidthF(1.0);
	}
    painter->setPen(pen);
	painter->setRenderHint(QPainter::Antialiasing, true);

    painter->drawLine(m_x, m_y1, m_x, m_y2);
}

QPainterPath CutSolidVerticalLine::shape() const
{
    QPainterPath path;
    path.moveTo(m_x, m_y1);
    path.lineTo(m_x, m_y2);

    QPainterPathStroker stroker;
	stroker.setWidth(6.0);
    return stroker.createStroke(path);
}

//下面这段程序运行不到
void CutSolidVerticalLine::keyPressEvent(QKeyEvent *event)
{
	qreal Temp;
	QScreen *screen = QGuiApplication::primaryScreen();
	QRect screenGeometry;
	if (screen) {
		screenGeometry = screen->geometry();
	}
	QRect screenRect = screenGeometry;
	qreal width = screenRect.width();

	if (event->modifiers() & Qt::ControlModifier) // Check if the Ctrl key is pressed
		{
		if (event->key() == Qt::Key_Left)
		{
			if (m_x <= 150)  //不允许虚线负相对原点
			{
				m_x = 150;
			}
			else
			{
				m_x -= 1;
				this->moveBy(-1, 0);

				Temp = (((width*1.0) / 6.0) * 5.0) - 150.0 - 30.0;
				m_time = (m_x-150.0) / Temp * m_timelength *1000.0;
			}
			update();

			qDebug() << "CutSolidVerticalLine::keyPressEvent" << m_x;		
		}
		else if (event->key() == Qt::Key_Right)
		{
			m_x += 1;
			this->moveBy(1, 0);

			Temp = (((width*1.0) / 6.0) * 5.0) - 150.0 - 30.0;
			m_time = (m_x - 150.0) / Temp * m_timelength *1000.0;

			qDebug() << "CutSolidVerticalLine::keyPressEvent" << m_x;
			update();
		}
	}
	this->scene()->update();
    QGraphicsItem::keyPressEvent(event);
}

//void CutSolidVerticalLine::keyReleaseEvent(QKeyEvent *event)
//{
//    if (event->key() == Qt::Key_Right)
//    {
//        qDebug() << "CutSolidVerticalLine::keyReleaseEvent";
//        update();
//    }
//}
QVariant CutSolidVerticalLine::itemChange(GraphicsItemChange change, const QVariant &value)
{

    if (change == ItemPositionChange)
    {
		QPointF newPos = value.toPointF();
		newPos.setX(0); // Keep the X value constant
		newPos.setY(0); // Keep the Y value constant
		return QGraphicsItem::itemChange(change, newPos);
		//qDebug() << "CutSolidVerticalLine::itemChange1";
    }
    else if (change == ItemPositionHasChanged)
    {
		
		qDebug() << "CutSolidVerticalLine::itemChange2";
    }

    return QGraphicsItem::itemChange(change, value);
}
qreal CutSolidVerticalLine::getX() const
{
    return m_x;
}

qreal CutSolidVerticalLine::gety1() const
{
	return m_y1;
}

qreal CutSolidVerticalLine::gety2() const
{
    return m_y2;
}

qreal CutSolidVerticalLine::gettime() const
{
	return m_time;
}

void CutSolidVerticalLine::setX(qreal x) 
{
    m_x = x;
}

void CutSolidVerticalLine::sety1(qreal y1)
{
    m_y1 = y1;
}

void CutSolidVerticalLine::sety2(qreal y2)
{
    m_y2 = y2;
}

void CutSolidVerticalLine::settime(qreal time)
{
	m_time = time;
}
void CutSolidVerticalLine::setMusiclength(float ftime_length)
{
	m_timelength = ftime_length;
}
