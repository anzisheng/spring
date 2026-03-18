#include "ElectToolButton.h"
#include <QPixmap>
#include <QIcon>

ElectToolButton::ElectToolButton(QWidget * parent) :
    QToolButton(parent)
{
//    m_Scroll = new QScrollArea(parent);
//    m_Scroll->setWidget(this);
//    m_Scroll->setAutoFillBackground(true);
//    m_Scroll->show();
}

void ElectToolButton::showEvent(QShowEvent * event)
{
    QPixmap pix_map("images\\nomal.png");
    this->setGeometry(m_PosX,m_PosY,40,40); //按钮的位置及大小
    this->setIcon(QIcon(pix_map));
    this->setIconSize(QSize(40,20));
    this->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    QString qStrText = QString("%1.%2").arg(m_ID).arg(m_Name);
    this->setText(qStrText);
}

void ElectToolButton::setID(int ID)
{
    m_ID = ID;
}

void ElectToolButton::setName(QString qStrName)
{
    m_Name = qStrName;
}

void ElectToolButton::setType(QString qStrType)
{
    m_Type = qStrType;
}

void ElectToolButton::setX(int posX)
{
    m_PosX = posX;
}

void ElectToolButton::setY(int posY)
{
    m_PosY = posY;
}

void ElectToolButton::setZ(int posZ)
{
    m_PosZ = posZ;
}
