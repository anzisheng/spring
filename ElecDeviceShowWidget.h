#ifndef ELECDEVICESHOWWIDGET_H
#define ELECDEVICESHOWWIDGET_H

#include <QWidget>
#include <QGraphicsScene>
#include "ClassDefined.h"
#include "StaticValue.h"
#include <QMap>
#include <QList>
#include "ElecDeviceRectItem.h"
#pragma execution_character_set("utf-8")
namespace Ui {
class ElecDeviceShowWidget;
}

class ElecDeviceShowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ElecDeviceShowWidget(QWidget *parent = 0);
    ~ElecDeviceShowWidget();

    void setItemState(int id,bool bIsOn);

    void InitToolButton();

    void UpdataScene();

private:
    Ui::ElecDeviceShowWidget *ui;
    QGraphicsScene * m_scene;
    QMap<int,ElecDeviceRectItem*> m_elecDeviceMap;  //�����豸��ʾ�б�
signals:
    void RectItemClicked(int DevID);

};

#endif // ELECDEVICESHOWWIDGET_H
