#ifndef MUSICGRAPHICSVIEW_H
#define MUSICGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QJsonValue>
#include <QMenu>
#include <QPainter>
#include <QImage>
#include <QStringList>
#include "CollidingRectItem.h"
#include "myitem.h"
#include <QRectF>
#include "CutSolidVerticalLine.h"
#include "MovableVerticalLine.h"

#pragma execution_character_set("utf-8")

class MusicGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:

    enum { Type = 65535 + 1 };
    int type() const
    {
        // Enable the use of qgraphicsitem_cast with this item.
        return Type;
    }
    explicit MusicGraphicsView(QWidget *parent = 0);

    void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent *event); //这里处理画线，释放的时候，修正一下Y坐标
	//void mouseDoubleClickEvent(QMouseEvent *event) override; // Add this method
	void repositemMouse(QMouseEvent *event); // 纠正鼠标选中item的Y坐标校验和X坐标
    //删除表演动作用
    void keyPressEvent(QKeyEvent * event);

    QVector<CollidingRectItem*> getCollidingItems();
    QVector<CutSolidVerticalLine*> getcutsolidlines();
    void InitMusicActions(int action_id,QRectF rect,
                          float begin_time,float end_time,MyItem* parent);

    void setPlayState(bool bState);
    bool m_IsPlay;
	bool spaceKeyPressed; //确认按了回车键
	bool bvirtualenterpress; // 虚拟回车按下
	bool isfrommouse;
public:
	void setCutSolidLines(const QString &filename); //用于加载保存的实线函数
	void rescalewidth(qreal *startx, int *hormid,qreal *width, qreal *heigh);
    //add by eagle for line task
	void setmovableVerticalLinetime(float ftimelength, qint64 imin, qint64 imax); // 设置参考线音乐总长度
public slots:
    void onDrawVerticalLineButtonClicked();
	void drawVerticalLine(float xPosition);
	void slotsetmovelinefocus(bool bset);
	void slotinitlinedata();

    //add end
private:
    void deleteShowAction(CollidingRectItem* pItem);
    void CreateActions();
    void InitMenuText();
    void addAlwaysOn(int id, QRectF rect,float begin_time,
                     float end_time,QMap<int,RouteInfo*> route_info, MyItem *parent);
    void addDomino(int id,QRectF rect,float begin_time,
                   float end_time,QMap<int,RouteInfo*> route_info,MyItem* parent);
    void addImage(int id,QRectF rect,float begin_time,
                  float end_time,QMap<int,RouteInfo*> route_info,MyItem* parent);
    void addDmx512(int id,QRectF rect,float begin_time,
                   float end_time,QMap<int,RouteInfo*> route_info,MyItem* parent);

	void addClock(int action_id, QRectF rect, float begin_time, float end_time,		//时钟
		QMap<int, RouteInfo*> route_info, MyItem *parent);

signals:
    void deleteAction();
	void signalsetmediaposition(qint64 position);
	void signalmousesetmediapos(qint64 position);

public slots:
    void always_on();
    void dominuo();
    void addImage();
    void changeLight();
	void addclock();		//时钟

    void delscence();
	void WidgetCopy();		//复制
	void WidgetPaste();		//粘贴

private:
    QMenu* m_Menu;
    QAction* m_Always;			//常开
    QAction* m_DoMino;			//多米诺
    QAction* m_AddImage;		//插入图片
    QAction* m_Lignt;			//DMX调光
	QAction* m_Clock;			//Clock		时钟
	QAction* m_Text;			//Text		文本
	QAction* m_Cartoon;			//Cartoon	卡通动画
	QAction* m_3D;				//3D		三维立体
	QAction* m_Math;			//math		数学之美(基于3D)

    QAction* m_delete;			//删除
	QAction* m_musicline;		//画线
	QAction* m_copy;			//复制
	QAction* m_paste;			//粘贴

    QPoint m_triggerPos;		//鼠标右键点击gloab坐标
    QPoint m_leftPressPos;		//鼠标左键按下位置
    bool m_leftPressPendingMove = false;
    CollidingRectItem* m_RightClickItem = nullptr;
    QVector<CollidingRectItem*> m_music_actions;    //编曲动作列表
    MovableVerticalLine *m_movableVerticalLine; // 参考线。只有一根
    QVector<CutSolidVerticalLine*> m_cutsolidlines; //存储所有的实线，默认红色
	QVector<CutSolidVerticalLine*> m_getcutsolidlines; //从文件读取的红线
	bool compareMovableVerticalLine(CutSolidVerticalLine* a, CutSolidVerticalLine* b); //比较线的X坐标，用于排序

public:		//用于复制粘贴的中间变量
	int mc_music_id;	//编曲号码，对应左侧树的编曲编号
	int mc_run_type;	//动作类型，runType{ ALWAYS_ON, DOMINO_RUN, IMAGE_SHOW, DMX512_SHOW };

	int mc_music_action_id;

	int mc_DMX512_TYPE;	//DMX512动作类型 { DMX512_ALWAYSON, DMX512_DOMINO, DMX512_TRACK, DMX512_FROMTO, DMX512_RAINBOW };

};

#endif // MUSICGRAPHICSVIEW_H
