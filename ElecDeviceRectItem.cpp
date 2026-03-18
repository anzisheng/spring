#include "ElecDeviceRectItem.h"
#include <QPointF>
#include <QMessageBox>
#include "StaticValue.h"

ElecDeviceRectItem::ElecDeviceRectItem(QGraphicsItem* parent)
{
    m_isOn = false;
    m_PosX = 0;
    m_PosY = 0;
    m_PosZ = 0;
    m_ID = 0;
    m_Name = "";
    m_Type = "";
//    setFlag(QGraphicsItem::ItemIsMovable,false);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
}

void ElecDeviceRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
//    QMessageBox::information(nullptr ,"info","你点到我了!");
    m_isOn  =! m_isOn;
    int device_id = m_ID;
    int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
    StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
    StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = m_isOn;
    emit EelecDevClicked(m_ID);
//    QMessageBox::information(nullptr,"info",QString("%1").arg(m_ID));
    update();
}

QRectF ElecDeviceRectItem::boundingRect() const
{
    return QRectF(m_PosX+10,m_PosY,20,20);
}

void ElecDeviceRectItem::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem *option,
                               QWidget *widget)
{
    QRectF bound_rect = boundingRect();
    QBrush brush(m_rectColor);
    if(m_isOn)
    {
        brush.setColor(Qt::red);
    }
    else
    {
        brush.setColor(Qt::gray);
    }

    if(m_Type.compare("DMX512") == 0)
    {	//滚动屏幕的时候会运行到这里
        int rect_width = bound_rect.width() / 3;
        int rect_height = bound_rect.height();
        QRectF one_rect(bound_rect.x(),bound_rect.y(),
                        rect_width,rect_height);
        QRectF two_rect(one_rect.x() + rect_width,bound_rect.y(),
                        rect_width,rect_height);
        QRectF three_rect(two_rect.x() + rect_width,bound_rect.y(),
                        rect_width,rect_height);
        painter->fillRect(one_rect,brush);
        painter->drawRect(one_rect);
        painter->fillRect(two_rect,brush);
        painter->drawRect(two_rect);
        painter->fillRect(three_rect,brush);
        painter->drawRect(three_rect);
    }
    else
    {
        painter->fillRect(bound_rect,brush);
        painter->drawRect(bound_rect);
    }

    QString qStrName = QString("%1%2").arg(m_ID).arg(m_Name);
    painter->setPen(QPen(QColor(Qt::black)));
    painter->drawText(QPointF(m_PosX+30+20,m_PosY+15),qStrName);
}

//void ElecDeviceRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
//{
//    update();
//    QGraphicsItem::mouseMoveEvent(event);
//}

void ElecDeviceRectItem::advance(int phase)
{
    if(!phase) return;

    QPointF location = this->pos();

    setPos(mapToParent(0,0));
}


void ElecDeviceRectItem::setID(int ID)
{
    m_ID = ID;
}

void ElecDeviceRectItem::setName(QString qStrName)
{
    m_Name = qStrName;
}

void ElecDeviceRectItem::setType(QString qStrType)
{
    m_Type = qStrType;
}

void ElecDeviceRectItem::setX(int posX)
{
    m_PosX = posX;
}

void ElecDeviceRectItem::setY(int posY)
{
    m_PosY = posY;
}

void ElecDeviceRectItem::setZ(int posZ)
{
    m_PosZ = posZ;
}

void ElecDeviceRectItem::setIsOn(bool bState)
{
    m_isOn = bState;
 //   this->setPos(this->pos());
}


