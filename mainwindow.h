#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MusicGraphicsView.h"
#include "GraphicsSceneBase.h"
#include <QGraphicsRectItem>
#include <QJsonValue>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QTabWidget>
#include "QSubTreeView.h"
#include <QGraphicsLineItem>
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include <QLabel>
#include <QMap>
#include <QImage>
#include <QStringList>
#include "QTreeMode.h"
#include "ElecDeviceRectItem.h"
#include <QTimer>
#include "CommonFile.h"
#include "MusicWidget.h"
#include "CollidingRectItem.h"
#include "ElecDeviceShowWidget.h"
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QUrl>
#include "pingthread.h"

#include "winperformance.h"

#pragma execution_character_set("utf-8")
class ElecDeviceShowWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void newScene(QString qStrMusicName);

    QImage receive_image;
 //   QBuffer last_image_buffer;
    QByteArray last_image_array;
    QString  last_image_filename;

private:
 //   int nb_peer;//总共的小伙伴数目，用于确定client->index的取值
    QTimer* timingtimer_    =   nullptr;       //定制播放定时器，用于轮询定时播放任务
    bool    timingplay_     =   false;         //当前曲目否是定时播放状态
    QDateTime   timingend_;                   //本次定时播放任务的结束时间
    QUrl url_;
    QNetworkAccessManager qnam_;
    QNetworkReply *reply_;
    QNetworkReply *Imgreply_;
    QFile *webfile_;
    bool httpRequestAborted_;
    QNetworkReply *Taskreply_;

    QTimer* HTTPtimer_  =   nullptr;

    QTimer* HeartBeat_  =   nullptr;
    QUdpSocket  *HeartBeatSocket_    =   nullptr;

    bool JudgeArraySize(QByteArray datagram);
    void LoadControlStatus();               //从数据库中读取控制信息状态到rcstatus中
    void SaveControlStatus();               //保存远程控制信息状态和手机控制状态
    void ReadtimingMission();               //从数据库中读取定时任务
    void SavetimingMission();               //将定时任务保存到数据库
    int  CurrentJobID_  =   -1;
    void SetHTTPtimer();
    void GetImage(QString imgpath="http://39.100.46.158/hua.bmp");
    void StatusToHTTP(int jobid,int statuscode);
    void GetTaskFromHTTP();
    void closeEvent(QCloseEvent * event);
    void InitElecDeviceInfo();
    void thread_sleep(int iTime);
    void setvol(int voltage);
    unsigned char hex_asc(unsigned char ccc);
    unsigned int asc_hex(unsigned char ccc);

    void LoopSL32();
	void LoopSL40();
	void LoopCanMain();

    void LoopDo64Device();     //遍历do64电气阀设备
    void LoopDmx512Device();   //遍历dmx512灯光设备
    void SendHeartPackage();   //发送DMX心跳包
    void stopTimer();          //停止定时器
    void InitHeartContent();   //初始化给DMX512灯光设备发送的心跳包数据
    void InitOutPutData(int iType);   //初始化待输出的数据

    //表演动作结束后，结束表演动作
    void CloseDO64MusicAction(CollidingRectItem *item);
    void CloseDMX512MusicAction(CollidingRectItem *item);
    void clearDMX512State();
    void clearDO64State();
    void clearSL32();
	void clearSL40();

    //执行表演动作
    void InitAlwaysOn(CollidingRectItem* item);
    void InitDomino(CollidingRectItem* item);
    void InitImageShow(CollidingRectItem* item);
    void InitDMXShow(CollidingRectItem* item);
    void InitDMXDomino(CollidingRectItem *item, RouteInfo *route_info, QList<QList<int> > show_teams);
    void InitDMXOn(RouteInfo *route_info, QList<QList<int> > show_teams);
    void InitDMXRainbow(CollidingRectItem* item);
    void InitDMXFromTo(int route_id,int value1,int value2,int iDelayTime,
                       int change_speed,QList<QList<int>> show_teams,int iHeartTime);
    int getDeviceNumber(QString qStrType);   //获取某种类型的设备数量
    int getChangeStateNum();     //获取状态值改变的输出设备的数量

    void GetDmxData(); //获取DMX数据
    void SaveDmxDataToFile();  //保存数据到DMX文件中
    void SaveDmxHeartData();  //保存DMX心跳数据
    void SaveDMXHeaderData(); //保存DMX文件头

    void CreateListenSocket();  //创建监听端口
    void InitStatusBarWidget();       //初始化状态栏，以及内部组件
    void UpdateStatusBar();

    void RequestDMX();
    void PrepareHeartBeat();        //第三方心跳报文发送准备
//    void readImageSet();
    void initPingThreadData(); //增加ping检测, add by eagle for task001

signals:

public slots:
    void HTTPtimerup();
    void addTeam(int show_team_id);
    void MouseLeftDoubleClicked(int,int); //显示编队和阀的具体属性
    void timeOut();
    void ControlTimer();
    void ResetStartTimer(int start_time);
    void AOtimerout();
    void looptimeOut();     //单曲循环时候的timer
    void HeartBeatTimeOut();    //第三方心跳报文
//    void receiveImgTimeOut(); //定时去读取ipad发送过来的图片




private slots:
    void timingtimer();
    void GetImgFinished();
    void send_status_finished();
    void Get_Task_finished();
    void on_start_triggered();
    void on_pause_triggered();              //继续按钮
    void on_stop_triggered();
	void on_stop_progrem_body();		//停止播放的实际操作移到该函数下
    void on_open_triggered();
    void on_save_triggered();
    void on_quit_triggered();
    void on_new_music_triggered();
    void on_new_program_control_triggered();
    void on_delete_comp_file(QString);

    void on_action_DMX_triggered();

    void on_action_Query_triggered();   //生成序列文件
    void on_Query_triggered();          //做序列播放

//    void on_action_DRAW_triggered();
//    void deleteActionEvent();
	void removeSubTab(int index);		//926打开多个编曲标签时可以关闭

 //   void on_action_loopsingle_triggered();
    void on_loopsingle_triggered();
    void on_setpicture_triggered();
//    void on_ontimesetclick();

    void newConnect();        //客户端和服务器端连接
    void processPendingDatagram();  //接收图片socket过程
    void processPendingDatagram2021();  //新版接收图片socket过程

    void on_action4test_triggered();

    void on_pointertime_triggered();
    void TimelineHasFinished();
    void CommonRecvCommand();         //收到网络指令
    void DMXRecvCommand();         //收到网络指令

    void on_actionRemotecontrol_triggered();

	void ElecDevItemClicked(int j);

    void on_action11_18_triggered();

    void on_actionjust4mobile_triggered();

    void on_GetMission_triggered(bool checked);

    void on_ontimeset_triggered(bool checked);

    void on_timinglist_triggered();

    void on_action_DRAW_triggered(bool checked);

    void on_heartbeatsetting_triggered(bool checked);
    void handlePingResult(double percentage); //add by eagle for ip check slot


private:
    AO AOset_;
    remotecontrol   rcstatus_;                      //远程控制状态信息
    QTimer* AOtimer_;
    QTimer*         DMXTimer_;                      //DMX发送订阅信息定时器
    QUdpSocket*     AOvolset_;
    QUdpSocket*     CommonSocket_;                  //远程受控监听常规端口
    QUdpSocket*     DMXSocket_;                     //远程受控监听常规端口
    QUdpSocket*     DMXRequest_;                     //远程受控监听常规端口
    qreal lowVoltage_,highvoltage_;                 //水阀变频低电压与高电压
    QStatusBar* mybar_;                             //状态栏，用来显示水阀的工作状态
    QLabel* statusLabel_;                           //添加在状态栏中的Label，用来显示远程控制信息
    QLabel* workingstatus_;                         //显示监听状态以及工作模式等信息
	QLabel* pingstatus_;							// 显示ping设备，目前多少台可以ping通.
    QByteArray ba;
    Ui::MainWindow *ui;
    QTabWidget* m_tabWidget;
    QSubTreeView* tree_view;
    QMap<int,ElecDeviceRectItem*> m_elecDeviceMap;  //电气设备显示列表

    ElecDeviceShowWidget* m_elec_show;     //电气设备和灯光设备展示窗体

    int screen_width;
    int screen_height;

    int tree_width;
    int tab_width;

    QLabel* m_StatueText;
    bool m_IsOutPutTimerOn;   //输出数据的定时器是否开启

	bool m_IsManuPlayFlag;   //屏幕菜单点击启动表演的标志

//	QTimer* m_Timer;          //5毫秒心跳包
	Turing_Win_PerformanceTimer* m_Timer;

//	QMap<int,int> m_CurrentDeviceState;   //电气阀设备信息有变化才会发送
    char m_Buffer[20];			//DO64数据包
    char m_DmxBuffer[2000];		//dmx数据包，改为2000长度
    char m_HeartBuffer[20];     //DMX心跳包，改为20长度

    double m_PlayTimeCount;     //Play运行时长的计时器
	double m_HeartCount;		//仅用于心跳包的发送定时

    int m_CurrentMusicID;   //当前播放编曲的ID
    QMap<int,CollidingRectItem*> m_CurrentMusicActions;  //当前编曲的所有表演动作

    QMap<int,DeviceInfo*> m_do64State;


    //以下为存储DMX文件时所使用的变量
    int DmxHeartCount;	//以10ms为单位
    int m_intDoPoint;		//曲条从第0条开始
//    int m_intDoTimeFromStart;		//曲子运行了的时间长度，以ms为单位
    int m_intRecordSnNumAll;		//总记录数

    QFile m_original_file;      //原始文件的对象
    QFile m_decrypt_file;       //加密文件的对象
    LONG lOffset,lActual;		//这三个变量可能全工程都可以公用,因为是顺序执行的
    char* m_pDMX;
    char t_buf[ETH_LEN];
    int m_MusicType;            //当前保存DMX文件的编曲类型  0 编曲  1 程控
    QString m_MusicName;            //当前保存的DMX文件音乐名称；

    QueryInfo m_query;          //表演编队的序列

    QMap<int,int> m_dmx_state;  //当前表演动作的状态设备号-值（DMX512的话就是通道号-值）
    int m_DO64Num;              //do64板子的数量
    int m_SL32Num;              //SL32板子的数量
	int m_SL40Num;              //SL40板子的数量
    int m_DMX512Num;            //DMX512板子的数量；

    QUdpSocket udp_socket;      //发送的socket


//    QUdpSocket *receiver;       //接收的socket
    QTcpServer *server;
    QTcpSocket *tcpSocket;
//    void newConnect();        //客户端和服务器端连接
    void desConect();         //断开连接

 //   void displayError(QAbstractSocket::SocketError);  //显示错误
    int clientindex;        //1对N模式，客户端数量

    QPixmap receive_pixmap;
    QImage temp_img;
    int ipadimage_currtenttime;		//当有数据来的时候，获得当前时间，把当前
									//时间当成begin_time,开始计数

	BOOL m_isloopMusic;
	QTimer* m_loopTimer;			//单曲循环时的timer，去读取是否一遍结束的状态

	//接收ipad发图片数据的变量
	BOOL m_CtrlModeFlag;			//收到ipad图片的标志
	QTimer* m_receiveTimer;			//接收图片数据的timer
    void save_receive_img(QByteArray datagram);
    void save_receive_img2021(QByteArray datagram);		//新版保存图片函数
//	char m_imgBuffer[600];			//图片数据包

	int TimePeriod;		//彩虹渐变的变化的周期，3200ms。TimePeriod = (512/m_intRunStep)*m_intRunSpeed;
	int m_intRunStep;	//彩虹变化的台阶,4
	int m_intRunSpeed;	//彩虹变化的速度，25ms
	int m_intRunInt;	//灯与灯之间的时间延迟的时间，单位毫秒
	unsigned int TimeHaveRun;		//已经运行的时间
	int RunPoint;			//运行的指针

    PingThread *m_pingThread; //add by eagle for ip check thread
	QThread *m_thread;//add by eagle for ip check thread
};

#endif // MAINWINDOW_H
