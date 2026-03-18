#include "GraphicsSceneBase.h"

#include <QMessageBox>
#include <QPoint>
#include <QPen>
#include <QBrush>
#include <QRect>
#include <QGraphicsRectItem>
#include <QMessageBox>
#include <QFont>


GraphicsSceneBase::GraphicsSceneBase(QWidget *parent):
    QGraphicsScene(parent)
{

}

//void GraphicsSceneBase::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
//{
//    if(mouseEvent->button() == Qt::LeftButton)
//    {

//    }
//    QGraphicsScene::mousePressEvent(mouseEvent);
//}
