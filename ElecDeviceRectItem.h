#ifndef ELECDEVICERECTITEM_H
#define ELECDEVICERECTITEM_H

#include <QGraphicsItem>
#include <QRectF>
#include <QColor>
#include <QPainter>
#include <QBrush>
#include <QStyleOptionGraphicsItem>
#include <QtGui>
#include <QtCore>
#include <QWidget>
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPointF>
#pragma execution_character_set("utf-8")
//class ElecDeviceRectItem : public QObject,public QGraphicsItem    //2017.8.4 这里注释乱了，不知道以前写的什么
class ElecDeviceRectItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit ElecDeviceRectItem(QGraphicsItem* parent = 0);



    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);
//    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void advance(int phase);

    void setID(int ID);
    void setName(QString qStrName);
    void setType(QString qStrType);
    void setX(int posX);
    void setY(int posY);
    void setZ(int posZ);
    void setIsOn(bool bState);

private:
    QRectF m_boundRect;
    QColor m_rectColor;
    bool m_isOn;

    int m_ID;
    QString m_Name;
    QString m_Type;
    int m_PosX;
    int m_PosY;
    int m_PosZ;
signals:
    void EelecDevClicked(int i);
};

#endif // ELECDEVICERECTITEM_H
