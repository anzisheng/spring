#include "TimeLineItem.h"

TimeLineItem::TimeLineItem()
{
    this->setFlag( QGraphicsItem::ItemIsMovable);
}

QRectF TimeLineItem::boundingRect() const
{
    return QRectF(0,0,100,1000);
}


void TimeLineItem::setLine(QLineF line)
{
    m_Line = line;
}

//void TimeLineItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    if(event->button() == Qt::LeftButton)
//    {
//        QPointF current_pos = scenePos();
//        QPointF press_pos = event->lastScenePos();
//        press_pos.setY(current_pos.ry());
//        setPos(press_pos);
//    }
//    QGraphicsItem::mousePressEvent(event);
//}

void TimeLineItem::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{
    painter->drawLine(m_Line);
}
