#include "myitem.h"
#include "CommonFile.h"
#include "StaticValue.h"
#include "ClassDefined.h"



MyItem::MyItem(QRectF rect, QColor color, QGraphicsItem *parent)
{
    m_boundRect = rect;
    m_Color = color;
    //m_Show_Team_list = "0"; //anzs
}

QRectF MyItem::boundingRect() const
{
    return m_boundRect;
}

void MyItem::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{
    QRectF rec = boundingRect();
    QBrush Brush(m_Color);

    painter->fillRect(rec,Brush);
    painter->drawRect(rec);
}

void MyItem::setShowTeam(int id)
{
    this->id = id;
    ShowTeam *show_team = StaticValue::GetInstance()->
            m_show_team.value(id);

    m_Show_Team_list = StaticValue::GetInstance()->getShowTeamList(show_team);
}

