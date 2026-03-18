#ifndef MYITEM_H
#define MYITEM_H

#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsItem>
#include <QRect>
#include <QColor>
#include <QStyleOptionGraphicsItem>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#pragma execution_character_set("utf-8")
//表演编队对象，只储存表演编队的信息
//class MyItem : public QObject, public QGraphicsItem //2017.8.4屏蔽的
class MyItem :  public QGraphicsObject
{
    Q_OBJECT
public:
    MyItem(QRectF rect,QColor color,QGraphicsItem* parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);

    void setShowTeam(int id);
    void setItemType(int iType);
    int  getItemType();


protected:
    void parse();

public:
    int id;
    QString m_Show_Team_list;

private:

    QRectF m_boundRect;
    QColor m_Color;

};

#endif // MYITEM_H
