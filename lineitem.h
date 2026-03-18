#ifndef LINEITEM_H
#define LINEITEM_H
#include <QGraphicsEllipseItem>
#include <QGraphicsItemAnimation>
#include <QGraphicsScene>
#pragma execution_character_set("utf-8")
class LineItem :public QGraphicsLineItem
{

public:
	enum { Type = UserType + 105 }; // Define the custom type
    LineItem();
	int type() const override { return Type; } // Override the type() function to return the custom type

//    virtual void    dragMoveEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;

//    virtual void    hoverMoveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;

//    virtual void    mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

    virtual void    mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

    virtual QRectF    boundingRect();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget) Q_DECL_OVERRIDE;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)    Q_DECL_OVERRIDE;

    void SetScene(QGraphicsScene* Scene);

    int currenttime_;       //当前表演时间，单位ms
	qreal currentxpos_;		//add by eagle for 当前时间线位置

private:


    QGraphicsScene* Scene_;
    qreal width_;

    qreal height_;

    QPointF topleft_;

    qreal penwidth_ =   1;

    QGraphicsItemAnimation* IA_;

    QGraphicsSimpleTextItem *TI_;
signals:
    void MoveFinished();        //移动完成

};

#endif // LINEITEM_H
