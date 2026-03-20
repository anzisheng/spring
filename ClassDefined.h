#ifndef CLASSDEFINED_H
#define CLASSDEFINED_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include <vector>
#include <QList>
#include <QPoint>
#include <QMultiMap>
#include <QTimeLine>

#pragma execution_character_set("utf-8")

struct AO
{
    QString ipaddr_;
    qreal   lowvoltage_;
    qreal   highvoltage_;
    qreal   maxvoltage_ =   10;
    qreal   minvoltage_ =   0;
    bool    enable_     =   false;
};
struct remotecontrol
{
    enum Mode{COMMON,DMX,NONE}; //工作模式
    Mode    WorkingMode =   Mode::NONE;
    qint16  CommonPort  =   8000;     //监听端口，缺省值从中牟水帘开始改为8800，之前是8000
    qint16  DMXPort     =   6000;     //监听端口
    QString DMXAddr     =   "192.168.1.14";    //DMX地址
    qint16  DataIndex   =   0;      //请求的数据起始位置
    qint16  DataLength  =   24;             //请求的数据长度
    QString Source      =   "";         //命令来源地址
};
//轨迹数据结构
class Orbit : public QObject
{
    Q_OBJECT
public:
    explicit Orbit()
    {
        id = 0;
        name = "";
        time_length = 0;
        delay_time = 0;
        reversal = false;
        reversal_output = false;
    }

public:
    int id;
    QString name;
    int time_length;
    int delay_time;
    bool reversal;
    bool reversal_output;
    QMap<int,int> show_orbit;
};


//DMX512设备
class DMX512 : public QObject
{
    Q_OBJECT
public:
    explicit DMX512()
    {
        id = 0;
        name = "";
        port = 0;
        ipAddress = "";
    }

public:
    int id;
    QString name;
    int port;
    QString ipAddress;
};

//水阀输出设备
class OutPutDevice : public QObject
{
    Q_OBJECT
public:
    explicit OutPutDevice()
    {
        id = 0;
        name = "";
        type = "";
        serialPorts = "";
        ipAddress = "";
        port = 6000;
        Address = "";
        isSixteenths = false;
        slotNum = 0;
        comment = "";
        bHasChange = false;
    }

public:
    int id;
    QString name;
    QString type;
    QString serialPorts;
    QString ipAddress;
    int     port;
    QString Address;
    bool isSixteenths;
    int slotNum;
    QString comment;
    QMap<int,int> m_mapping;		//水阀映射（电气设备ID-屏幕显示ID）
    QMap<int,int> m_route_state;	//该电气设备的有变化的通道的列表（通道号-值）
	bool bHasChange;				//数据有变化

	QMap<int, int> m_delay_ms;		//延迟值（电气设备ID-延迟值）
    //QMap<int, float> m_delay_ms;		//延迟值（电气设备ID-延迟值）

};

//设备信息
class DeviceInfo : public QObject
{
    Q_OBJECT
public:
    explicit DeviceInfo()
    {
        id = 0;
        elec_device_id = 0;
        output_device_id = 0;
        name = "";
        type = "";
        pos_x = 0;
        pos_y = 0;
        pos_z = 0;
        isDefault = false;
        out_put_value = 0;
		delayinms = 0;
    }

signals:

public slots:

public:
    int id;
    int elec_device_id;
    int output_device_id;
    QString name;
    QString type;
    int pos_x;
    int pos_y;
    int pos_z;
    bool isDefault;
    int out_put_value;	//实际的输出值，0-255之间
    float delayinms;
    //int delayinms;		//延时值，单位ms
};

class DMXState
{
public:
    explicit DMXState()
    {
        output_device_id = 0;
    }
public:
    int output_device_id;
    QMap<int,int> route_state;
};

//DMX多米诺和数字量多米诺共用
class RouteInfo : public QObject
{
    Q_OBJECT
public:
    explicit RouteInfo()
    {
        id = 0;
        type = 0;
        orbit_id = 0;
        out_value1 = 255;
        out_value2 = 0;
        keep_time = 0;
        delay_time = 0;
        perial_time = 1;
        loop_time = -1;
        start_delay = 0;
        change_speed = 0;
		fade_in = 0;
		fade_out = 0;

		reverse_flag = 0;
		from_left = 0;
		img_path = "";

		ClockStyle = 0;
		TimeLeadValue = 0;
    }

public:
    int id;        //通道ID
    int orbit_id;  //表演轨迹的ID
    int type;      //dmx的设置类型：on always track fromto
    int out_value1;
    int out_value2;
    int keep_time;
    int delay_time;
    int perial_time;
    int loop_time;
    int start_delay;
    int change_speed;
	int fade_in;
	int fade_out;

    qint8 len1,len2;
    qint32 speed;
    QString img_path;
    bool bIsYangWen;
    uint circle;    //彩虹周期

	int reverse_flag;
	int from_left;

	int ClockStyle;
	int TimeLeadValue;

//	char dmx_keep_time_flag[4][1000];   //保持中的标志
//	int dmx_keep_time[4][1000];		 //保持中的时间
};

//DMX512设置
class MusicAction : public QObject
{
    Q_OBJECT
public:
    explicit MusicAction()
    {
        id = 0;
        type = 0;
        begin_time = 0;
        end_time = 0;
    }

public:
    int id;
    int type;                //常开、多米诺、轨迹、dmx灯光
    float begin_time;       //表演动作在编曲内开始时间
    float end_time;         //表演动作在编曲内结束时间
    QMap<int,RouteInfo*> route_info;   //表演动作内的各个通道的动作
};

class QueryInfo : public QObject
{
    Q_OBJECT
public:
    explicit QueryInfo()
    {
        m_opened = false;
        m_fileOpened = 0;
        oldsel = 0;
        intDisMode = 0;
        bOutputStatus = 0;
        DisQueueLoopCurr = 0;
    }
public:
    bool m_opened;				//编曲文件已经打开的标志，如果为0，说明一个文件也没有，不允许运行和存储播放队列
    //new一个编曲不影响这个标志，还是0，存储一次也不变，只有打开编曲才会影响着这个标志！
    unsigned int m_fileOpened;	//打开的文件个数，也就是bottom表单的长度
    unsigned int oldsel;		//刚刚点击的一行的行号
    unsigned int intDisMode;	//播放方式，顺序、循环、随机，初始化为顺序播放
    //是否点击输出图标的标志，初始化为禁止输出
    bool bOutputStatus;			//0:禁止输出，1:允许输出
    int DisQueueLoopALL;		//播放队列循环总次数
    int DisQueueLoopCurr;		//当前播放队列循环次数
};


//编曲信息
class MusicInfo : public QObject
{
    Q_OBJECT
public:
    explicit MusicInfo()
    {
        id = 0;
        type = 0;
        name = "";
        music_path = "";
        time_length = 1.0;
    }

public:
    int id;
    int type;
    QString name;
    QString music_path;
    float time_length;
    QMultiMap<int,int> team_actions;  //编曲中编队和编队动作容器
};

//表演编队
class ShowTeam : public QObject
{
    Q_OBJECT
public:
    explicit ShowTeam()
    {
        id = 0;
        name = "";
        rowType = 0;
        minNum = 0;
        maxNum = 0;
        arrayNum = 0;
        repeckType = 0;
        continueNum = 0;
        blankNum = 0;
        isReverse = false;
        isRound = false;
        showTeamList = "";
    }

signals:

public slots:

public:
    int id;
    QString name;
    int rowType;
    int minNum;
    int maxNum;
    int arrayNum;
    int repeckType;
    int continueNum;
    int blankNum;
    bool isReverse;   //倒序
    bool isRound;   //往返
    QString showTeamList;
};

#endif // CLASSDEFINED_H
