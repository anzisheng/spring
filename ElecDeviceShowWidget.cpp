#include "ElecDeviceShowWidget.h"
#include "ui_ElecDeviceShowWidget.h"
#include <QColor>


ElecDeviceShowWidget::ElecDeviceShowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ElecDeviceShowWidget)
{
    ui->setupUi(this);
    if (ui->horizontalLayout)
    {
        ui->horizontalLayout->setContentsMargins(0, 0, 0, 0);
        ui->horizontalLayout->setSpacing(0);
    }

    m_scene = new QGraphicsScene(ui->graphicsView);
    m_scene->setBackgroundBrush(QColor(245, 245, 245));
    ui->graphicsView->setScene(m_scene);
    InitToolButton();
}

ElecDeviceShowWidget::~ElecDeviceShowWidget()
{
    delete ui;
}

//SL32的设备显示，也可以用于DO64，但是没办法用于DMX
void ElecDeviceShowWidget::setItemState(int id,bool bIsOn)
{
      m_elecDeviceMap[id]->setIsOn(bIsOn);
//      m_elecDeviceMap[id]->resetMatrix();
//      m_scene->advance();
//    m_scene->invalidate(m_scene->sceneRect());
}

void ElecDeviceShowWidget::InitToolButton()
{
    QMap<int,DeviceInfo*> device_map = StaticValue::GetInstance()->m_device_map;
    QList<int> keys = device_map.keys();
    int x = 0;
    int y = 40;
    for(int i = 0; i < keys.size(); ++i)
    {
        int id = keys.at(i);
        DeviceInfo* device_info = device_map.value(id);
        ElecDeviceRectItem* pItem = new ElecDeviceRectItem();
        connect(pItem, &ElecDeviceRectItem::EelecDevClicked,
                this, &ElecDeviceShowWidget::RectItemClicked);
        pItem->setID(id);
        pItem->setName(device_info->name);
        pItem->setType(device_info->type);
        pItem->setX(x);
        pItem->setY(y);
        pItem->setZ(device_info->pos_z);
        pItem->setIsOn(false);
        m_scene->addItem(pItem);
        m_elecDeviceMap[id] = pItem;
        if(x == 840)  //每行8个控件
        {
            y += 40;    //行间距为40
            x = 0;      //下一行从0开始计算
        }
        else
            x += 120;   //列间距是120
    }
}

void ElecDeviceShowWidget::UpdataScene()
{
    m_scene->update();
}
