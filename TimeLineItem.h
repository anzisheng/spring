#ifndef TIMELINEITEM_H
#define TIMELINEITEM_H

#include <QtCore>
#include <QtGui>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>


class TimeLineItem : public QObject , public QGraphicsItem
{
    Q_OBJECT
public:
    TimeLineItem();

    QRectF boundingRect() const;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);


    void setLine(QLineF line);

 //   void mousePressEvent(QGraphicsSceneMouseEvent * event);
private:
    QLineF m_Line;
};

#endif // TIMELINEITEM_H
