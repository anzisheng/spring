#include "GraphicsViewMenu.h"

GraphicsViewMenu::GraphicsViewMenu(QWidget *parent) :
    QMenu(parent)
{
    CreateActions();
}

void GraphicsViewMenu::CreateActions()
{
    m_NewPlay = new QAction(this);
    m_NewPlayFullRow = new QAction(this);
    m_Block = new QAction(this);
    m_Attribute = new QAction(this);
    m_StickUpPlay = new QAction(this);

    InitMenuText();

    this->addAction(m_NewPlay);
    this->addAction(m_NewPlayFullRow);
    this->addAction(m_StickUpPlay);
    this->addAction(m_Block);
    this->addSeparator();
    this->addAction(m_Attribute);
    QObject::connect(m_NewPlay, SIGNAL(triggered()), this, SIGNAL(NewPlay()));
}

void GraphicsViewMenu::InitMenuText()
{
    m_NewPlay->setText(tr("新建表演动作"));
    m_NewPlayFullRow->setText(tr("新建表演动作(充满当前分段)"));
    m_Block->setText(tr("分段锁定"));
    m_Attribute->setText(tr("属性"));
    m_StickUpPlay->setText(tr("粘贴表演动作"));
}
