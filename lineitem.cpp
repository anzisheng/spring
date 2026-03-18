#include <QDebug>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>
#include <QMessageBox>
#include <QTime>
#include "lineitem.h"

#pragma execution_character_set("utf-8")

LineItem::LineItem()
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    TI_ =   new QGraphicsSimpleTextItem;
    TI_->setText("wocaolgeDJ");
}

//void LineItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
//{
//    qDebug()<<"dragMoveing!";
//    QGraphicsRectItem::dragMoveEvent(event);

//}

//void LineItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
//{
//    qDebug()<<"hoverMoveEvent!";
//    QGraphicsRectItem::hoverMoveEvent(event);

//}

//void LineItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
//{
//    qDebug()<<"mouseMoveEvent!";

//    QGraphicsRectItem::mouseMoveEvent(event);
//}

void LineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
//    if(event->button()==Qt::LeftButton)
//    {
//        //取得当前位置

//        QList<QPair<qreal, QPointF> > oldlist   =   IA_->posList();

//        QPointF curpos  =   pos();

//        //从新设置动画，以及表演完成需要的时间

//        IA_->clear();
//        //从新设置时间线
//        IA_->timeLine()->setDuration(5000);

//        //时间线start
//        for(int i  = curpos.x();i<250;i++)
//        {
//            QPointF newpos(i,0);
//            IA_->setPosAt(i/(250.0-i),newpos);
//        }

//        QList<QPair<qreal, QPointF> > newlist   =   IA_->posList();

////        qDebug()<<IA_->timeLine()->state();
//        qDebug()<<IA_->timeLine()->startFrame();
//        qDebug()<<IA_->timeLine()->endFrame();

//        IA_->timeLine()->start();
//        //发送信号
//    }
//    else if (event->button()==Qt::RightButton)
//    {
////        timeline_->setStartFrame(200);
//        IA_->clear();
//        int curx    =   this->pos().x();
//        for(int i=curx;i<=200;i++)
//        {
//            IA_->setPosAt(i/curx,QPointF(i,0));
//        }
//        IA_->timeLine()->start();
//    }
    QGraphicsLineItem::mouseReleaseEvent(event);
}

QRectF LineItem::boundingRect()
{
//    QLineF  linerec =   line();

//    QRectF  linerect(linerec.x1(),linerec.y1(),linerec.x2(),linerec.y2());

    return QGraphicsLineItem::boundingRect();
}

void LineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
//    painter->drawLine(myline);



//    update(textrec);
    QGraphicsLineItem::paint(painter,option,widget);



    QLineF  myline  =   line();
    QTime time;
    time    = QTime::fromMSecsSinceStartOfDay(currenttime_);
	TI_->setText(time.toString("m:ss:zzz"));
	QRectF  textRect    =   TI_->boundingRect();		//播放线的时间值显示

//	int temp1 = myline.x1();
//	int temp2 = textRect.width() / 2;
//	int temp3 = myline.y1();
//	int temp4 = textRect.height();
	TI_->setPos(myline.x1()-textRect.width()/2,myline.y1()-textRect.height());	//时间值显示的位置
	//currentxpos_ = myline.x1() - textRect.width() / 2;
}

QVariant LineItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
	currentxpos_ = value.toPointF().x();

    if (change == ItemPositionChange)
    {
        return QPointF(value.toPointF().x(),pos().y());		
    }
    return QGraphicsLineItem::itemChange(change, value);
}

void LineItem::SetScene(QGraphicsScene *Scene)
{
    Scene_  =   Scene;
    Scene_->addItem(TI_);
    TI_->setParentItem(this);
}
