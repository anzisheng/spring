#ifndef GRAPHICSVIEWMENU_H
#define GRAPHICSVIEWMENU_H

#include <QMenu>
#include <QAction>

class GraphicsViewMenu : public QMenu
{
    Q_OBJECT
public:
    explicit GraphicsViewMenu(QWidget *parent = 0);

private:
    void CreateActions();
    void InitMenuText();

signals:
    void NewPlay();

public slots:

private:
    QAction* m_NewPlay;        //新建表演动作
    QAction* m_NewPlayFullRow; //新建表演动作充满整段
    QAction* m_StickUpPlay;    //粘贴表演动作
    QAction* m_Block;          //分段锁定
    QAction* m_Attribute;      //属性

};

#endif // GRAPHICSVIEWMENU_H
