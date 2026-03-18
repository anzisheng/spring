#ifndef GRAPHICSSCENEBASE_H
#define GRAPHICSSCENEBASE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPoint>
#include <QtCore>
#include <QtGui>
#include <QWidget>
#include "myitem.h"

class GraphicsSceneBase : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GraphicsSceneBase(QWidget *parent = 0);

 //   void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent);

public slots:


private:

    QString qStrMusicName;  //编曲音乐名
};

#endif // GRAPHICSSCENEBASE_H
