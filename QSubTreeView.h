#ifndef QSUBTREEVIEW_H
#define QSUBTREEVIEW_H

#include <QTreeView>
#include <QMouseEvent>
#include <QJsonValue>
#include "QTreeItem.h"
#include <QApplication>
#include <QMenu>
#include <QAction>
#pragma execution_character_set("utf-8")
class QSubTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit QSubTreeView(QWidget *parent = 0);

    void mouseDoubleClickEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void resetTreeView();

private:
    bool deleteOrbit(int id);
    bool deleteOutPutDevice(int id);
    bool deleteShowTeam(int id);
    bool deleteComp(int id);
    void clearMenu();


signals:
    void leftDoubleClicked(int,int);
    void addShowTeam(int show_team_id);//曲目中增加表演编队
    void addMusic();
    void addProgramControl();
    void addQueryMusic();
    void deleteCompSignal(QString);


public slots:
    void newDMX();
    void newSL32();
	void newSL40();
	void newCanMain();
	void newTTL();

    void newOrbit();
    void newOutPut();
    void newShowTeam();
    void deleteItem();
	void manualAssignmentId();
    void newMusic();
    void newProgramControl();
    void newQueryMusic();

private:
    QMenu* m_Menu;
    QAction* m_NewSL32;			//E-SL32_D21
	QAction* m_NewSL40;			//E-SL40_1.0
	QAction* m_NewCanMain;		//E-CAN-MAIN
	QAction* m_NewTTL;				//新建TTL长灯光设备

    QAction* m_NewOrbit;			//新建表演轨迹
    QAction* m_NewDMX;				//新建标准DMX512灯光设备
    QMenu* m_NewOutPut;				//新建输出设备
    QAction* m_NewOutPut_16;		//16路输出设备
    QAction* m_NewOutPut_32;		//32路输出设备
    QAction* m_NewOutPut_48;		//48路输出设备
    QAction* m_NewOutPut_64;		//64路输出设备

    QAction* m_NewshowTeam;			//新建表演编队
    QAction* m_NewMusic;			//新建编曲
    QAction* m_NewProgramControl;	//新建程控
    QAction* m_NewQueryMusic;		//新建表演序列
    QAction* m_Delete;				//删除
	QAction* m_reconnectMusicAction;//重新连接音乐

	QAction* m_manualAssignmentId;	//手工赋值id号

    QTreeItem* m_pCurrentItem;
};

#endif // QSUBTREEVIEW_H
