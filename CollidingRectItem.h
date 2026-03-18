#ifndef COLLIDINGRECTITEM_H
#define COLLIDINGRECTITEM_H

#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QCursor>
#include <QStyleOptionGraphicsItem>
#include <QPainterPath>
#include <QTimer>
#include "StaticValue.h"
#include "ElecDeviceRectItem.h"
#include "ClassDefined.h"
#include <QGraphicsSceneDragDropEvent>
#pragma execution_character_set("utf-8")
class CollidingRectItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    CollidingRectItem(int type,QRectF rect,QColor color,
                      QGraphicsItem* parent = 0);
    enum { Type = UserType + 1 };

    int type() const
    {
        // Enable the use of qgraphicsitem_cast with this item.
        return Type;
    }
    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);


    QPainterPath shape()const;
    bool contains(const QPointF &point) const override;
    void ResetRect(QRectF rect);
	void setscaleRect(QRectF rect);

    //表演编队、表演动作
    void setItemType(int iType);
    int  getItemType();

    void setID(int id);
    int getID();
    int getType();
    void setType(int iType);
    bool getIsShowing();
    void setIsShowing(bool bState);
    QMap<int,RouteInfo*> getRouteSetting();
    QList<QList<int>> getShowTeamsList();

	//阀常开
	void setData(int iOutputValue1);

	//数字量多米诺
//    void setData(int iKeepTime,int iDelayTime,
//                 int iPerialTime,int iLoopTimes);

//	void setData(int iKeepTime, int iDelayTime,
//		int iPerialTime, int iLoopTimes, int iOutputValue1, int iOutputValue2, int iStartDelay);

	void setData(int iKeepTime, int iDelayTime,
		int iPerialTime, int iLoopTimes, int iOutputValue1, int iOutputValue2, int iStartDelay, int iReverseFlag, int iFromLeft);

    void setData(QString qStrImagePath,int delayTime,int LoopTime,bool bIsYangWen );	//Image用

//	void setData(int clockstylesel, int delayTime, int LoopTime, bool bIsYangWen);		//Clock用
	void setData(int clockstylesel, int timeleadvalue, int delayTime, int LoopTime, bool bIsYangWen);	//Clock用

	//DMX
	void setData(int type, int orbit_id, int out_value1, int out_value2,
		int keep_time, int delay_time, int perial_time,
		int loop_time, int start_delay, int change_speed,
		int fade_in, int fade_out, int reverse_flag, int from_left);

    void setData(QMap<int,RouteInfo*> setting);
    void setTimes(float begin_time,float end_time);

private:
    void InitData();
    void change_device_state();
    void resizeImage2ShowTeam(QString device_list);
    QString getDrawText();
    void stopTimer();

public slots:

signals:

protected:
//    void dragMoveEvent(QGraphicsSceneDragDropEvent * event);

    //查看表演动作详细信息，并修改
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

private:

    QRectF m_boundRect;
    QRectF m_InnerRect;         //内层rect,用来判断鼠标进入边缘的
    QColor m_rectColor;
    bool cScale;
    QCursor* m_cursor;
    QPointF start;
    QPointF end;
    int direction;
    QGraphicsSimpleTextItem*    leftTimeTip_;   //左侧时间信息提示
    QGraphicsSimpleTextItem*    rightTimeTip_;  //右侧时间信息提示

public:
    float begin_time;   //表演动作的开始时间
    float end_time;     //表演动作的结束时间

    int id;				//表演动作在数据库中的ID号
    int m_type;			//常开、多米诺、图片,DMX512，时钟Clock，三维，动画，数学之美
    bool m_IsDeviceOn;	//判断设备是否已经和时间轴碰撞，true 已经碰撞，false 未碰撞

    int m_orbit_id;		//轨迹ID
    int m_out_value1;
    int m_out_value2;
    int m_len1,m_len2;
    int m_speed;
    int m_keep_time;
    int m_delay_time;
    int m_perial_time;
    int m_loop_time;
    int m_start_delay;
    int m_change_speed;

	int m_clockstylesel;	//时钟样式选择
	int m_timeleadvalue;	//时间提前量，单位ms

	int m_fade_in;
	int m_fade_out;

    QMap<int,RouteInfo*> m_route_setting; //DMX512设置

    QString m_ImagePath;   //加载的图片路径
    QImage new_image;      //根据编队电气阀进行缩放比之后的img对象；
    bool m_IsYangWen;      //是否是阳文

    QString m_show_team_list;   //表演编队列表
	QList<QList<int>> m_show_teams;   //表演编队列表
//	QList<QList<int>, int, QList<int>> m_show_teams;   //表演编队列表

    bool m_IsShowing;  //是否正在表演（当前时间小于最后表演时间，大于开始时间）

	//以下为添加
	int m_curr_loop_time;
	int m_reverse_flag;
	int m_from_left;

	char keep_time_flag[6000];		//保持中的标志
//	QList<char> keep_time_flag;		//保持中的标志
	int keep_time[6000];			//保持中的时间
//	QList<int> keep_time;			//保持中的时间

	char dmx_keep_time_flag[4][6000];   //dmx保持中的标志
	int dmx_keep_time[4][6000];		 //dmx保持中的时间
};

#endif // COLLIDINGRECTITEM_H
