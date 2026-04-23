//-----------------------------------------------------------------------------------------
//2021-11-16	把写QUE和Dmx文件的里面的中文统一为英文，避免了中文编码长度的问题
//				存储还需要继续修改，增加除图片以外的动作的存储部分

//				修改了 Rainbow 的代码部分，后续还需要修改，需要深入研究
//				待解决问题1：DMX动作为Rainbow的时候，计算机运行正常，写文件在SD卡运行不正常，但是track动作正常
//				缺省的DMX文件名称改为sovotek

//				待解决问题2：监听、电气设备、变频器动态值、message'box信息等等都变成了乱码

//2021-11-18	修改了Rainbow，增加了WRGB的特殊处理 initdmxrainbow

//2021-11-20	修改了软件台头，现在是每次编译的代码都不一样，将来写到数据库或者config里面去

//2025-07-14	把m_HeartCount替换为m_PlayTimeCount
//-----------------------------------------------------------------------------------------
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileHelper.h"
#include <QSplitter>
#include <QListView>
#include <QTextEdit>
#include <QTreeView>
#include <QStyleFactory>
#include <QVBoxLayout>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtGui>
#include "QSubTreeView.h"
#include "StaticValue.h"
#include "QOutPutDialog.h"
#include "QShowTeamsDialog.h"
#include "QDeviceInfoDialog.h"
#include "CommonFile.h"
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include "myitem.h"
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QtDebug>
#include "config.h"
#include "ClassDefined.h"
#include "OrbitDialog.h"
#include <QFile>
#include <QFileDialog>
#include "ProgramControl.h"
#include "drawtabledialog.h"
#include "ipadimagesetdialog.h"
#include "ontimesetdialog.h"
#include "addquedialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include "ElecDeviceShowWidget.h"
#include <QtCore>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTime>
#include "LockCode.h"
#include <QtCore>
#include <string>
#include "MusicGraphicsView.h"
#include "MusicWidget.h"
#include <QDateTime>
#include <QString>
#include <QSettings>
#include <algorithm>

#include <QJsonDocument>
#include <QJsonValue>
#include "ouputViewer.h"
#include "aosetting.h"
#include "remotedlg.h"

#include "playtiming.h"
#include "pingthread.h"
#include <QLabel>
#include "senddatathread.h"

#pragma execution_character_set("utf-8")
using namespace std;
//#define  img_heigth    192
//#define  img_weigth    100
#define  ipad_port     60000

#define IF_TWIST (StaticValue::GetInstance()->m_SysConfig.value("TWIST").toInt() == 1)
//{
//#define if(IF_TWIST) （#define TWIST_IMAGE 1) else (#define TWIST_IMAGE 0)
//}

const QString mobiledomain  =   "http://www.thfountain.com";
//const QString mobiledomain  =   "http://39.100.46.158";
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
//    QLoggingCategory::setFilterRules("*.debug=true");
//    qDebug()<<"wocaolegeDJ";
//    qWarning()<<"warning!FBI warning !";
    ui->setupUi(this);
//    this->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint|Qt::WindowFullscreenButtonHint);
    this->setWindowFlags(Qt::WindowFullscreenButtonHint);

//	this->setWindowFlags(windowFlags()&~(Qt::WindowCloseButtonHint |
//							 Qt::WindowMinMaxButtonsHint));


    //1 从数据库中读取AO设置信息
    qDebug()<<"read aosetting infomation!";
    QSqlQuery   query;
    QString qStrSql = QString("select * from AOsetting");
    if(query.exec(qStrSql))
    {
        if(query.next())
        {
            AOset_.ipaddr_  = query.value(0).toString();
            AOset_.lowvoltage_ = query.value(1).toReal();
            AOset_.highvoltage_= query.value(2).toReal();
            AOset_.enable_  = query.value(3).toBool();
        }
    }
    else
    {
        QMessageBox::information(this,"警告","无法读取AO变频数据!");
        QMainWindow::close();
    }

    AOvolset_   =       new QUdpSocket;
	m_DO64Num = 0;      //DO64的数量
	m_SL32Num = 0;      //DO32的数量
	m_SL40Num = 0;      //SL40的数量
	m_DMX512Num = 0;    //MCON的数量

	m_PlayTimeCount = 0;
    m_CurrentMusicID = 0;
    m_IsOutPutTimerOn = false;

	m_IsManuPlayFlag = false;

//--------------------------------------------------------------------------
//彩虹渐变
	m_intRunStep=4;		//彩虹变化的台阶,4
	m_intRunSpeed=25;	//彩虹变化的速度，25ms
	m_intRunInt=500;	//灯与灯之间的时间延迟的时间，单位毫秒

	TimeHaveRun=0;		//已经运行的时间
	RunPoint=0;			//运行的指针

	TimePeriod = (512/m_intRunStep)*m_intRunSpeed;
//--------------------------------------------------------------------------

    m_query.m_opened = true;
    m_query.m_fileOpened = 1;
    m_query.oldsel = 1;
    m_query.intDisMode = 1;
    m_query.bOutputStatus = 1;
    m_query.DisQueueLoopALL = -1;
    m_query.DisQueueLoopCurr = 0;

    connect(this,SIGNAL(sendData(QByteArray)),
            this,SLOT(sendMessage(QByteArray)));

    StaticValue::GetInstance()->ZeroMemoryEx(m_Buffer,sizeof(m_Buffer));
    StaticValue::GetInstance()->ZeroMemoryEx(m_DmxBuffer,sizeof(m_DmxBuffer));
    StaticValue::GetInstance()->ZeroMemoryEx(t_buf,sizeof(t_buf));

    m_StatueText = new QLabel(this);
    m_StatueText->setText(tr("编曲"));
    ui->statusBar->addPermanentWidget(m_StatueText,0);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen ? screen->geometry() : QRect(0, 0, 1920, 1080);
    screen_width = screenRect.width();
    screen_height = screenRect.height();
    this->setMinimumSize(QSize(screen_width,screen_height));
    this->showMaximized();
//    this->showFullScreen();
    tree_width = screen_width/6;
    tab_width = screen_width*5/6;

    QWidget *main_window = new QWidget();
    this->setCentralWidget(main_window);

    tree_view = new QSubTreeView(main_window);
    tree_view->setObjectName("leftTreeView");
    QStyle *treeStyle = QStyleFactory::create("windows");
    if (treeStyle)
    {
        tree_view->setStyle(treeStyle);
    }
    tree_view->setAlternatingRowColors(true);
    tree_view->setUniformRowHeights(true);
    tree_view->setAnimated(true);
    tree_view->setIndentation(18);
    tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree_view->setSelectionMode(QAbstractItemView::SingleSelection);
    tree_view->setMinimumWidth(tree_width);
    m_tabWidget = new QTabWidget(main_window);
    m_tabWidget->setMinimumWidth(tab_width);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(tree_view);
    layout->addWidget(m_tabWidget);
    layout->setSpacing(1);
    main_window->setLayout(layout);
    main_window->show();

    InitElecDeviceInfo();

    InitHeartContent();

    tree_view->resetTreeView();

    //时间轴 - add by eagle for 高精度定时器支持
/*	m_Timer = new QTimer(this);
	m_Timer->setTimerType(Qt::PreciseTimer);
    connect(m_Timer,SIGNAL(timeout()),this,SLOT(timeOut()));
    m_Timer->start(TIME_RES);  */

	//时间轴 - add by eagle for 高精度定时器支持
	m_Timer = new Turing_Win_PerformanceTimer();
	//m_Timer->setTimerType(Qt::PreciseTimer);
	//connect(m_Timer, SIGNAL(timeout()), this, SLOT(timeOut()));
	connect(m_Timer, SIGNAL(timeout()), this, SLOT(timeOut()));
	m_Timer->start(TIME_RES);

    m_loopTimer=new QTimer(this);
    connect(m_loopTimer,SIGNAL(timeout()),this,SLOT(looptimeOut()));

//    m_receiveTimer=new QTimer(this);
//    connect(m_receiveTimer,SIGNAL(timeout()),this,SLOT(receiveImgTimeOut()));

    AOtimer_    =   new QTimer(this);
//	connect(AOtimer_,&QTimer::timeout,this,&AOtimerout);
	connect(AOtimer_,SIGNAL(timeout()),this,SLOT(AOtimerout()));
    AOtimer_->start(50);

    connect(tree_view,SIGNAL(leftDoubleClicked(int,int)),
            this,SLOT(MouseLeftDoubleClicked(int,int)));
    connect(tree_view,SIGNAL(addShowTeam(int)),
            this,SLOT(addTeam(int)));
    connect(tree_view,SIGNAL(addMusic()),
            this,SLOT(on_new_music_triggered()));
    connect(tree_view,SIGNAL(addProgramControl()),
			this,SLOT(on_new_program_control_triggered()));
    connect(tree_view,SIGNAL(deleteCompSignal(QString)),
            this,SLOT(on_delete_comp_file(QString)));

    connect(tree_view,SIGNAL(addQueryMusic()),
            this,SLOT(on_Query_triggered()));

	connect(m_tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(removeSubTab(int)));	//926关闭打开的编曲标签


    server = new QTcpServer(this);
//    nb_peer= 0;
//    bool state_server = server->listen(QHostAddress::Any,ipad_port);      //监听60000端口
    bool cres   =   connect(server,SIGNAL(newConnection()),this,SLOT(newConnect()));


//    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(desConect()),QT::DirectConnection);
//    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagram()));
//    connect(tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),
//             this,SLOT(displayError(QAbstractSocket::SocketError)));


//    readImageSet();

    clearDO64State();
    clearSL32();
	clearSL40();
    clearDMX512State();

    m_isloopMusic = 0;
    m_CtrlModeFlag = 0;

    ipadimage_currtenttime=0;

    DMXRequest_ =   new QUdpSocket(this);

    connect(DMXRequest_, SIGNAL(readyRead()),this, SLOT(DMXRecvCommand()));

    DMXTimer_   =   new QTimer(this);

//	connect(DMXTimer_,&QTimer::timeout,this,&RequestDMX);
	connect(DMXTimer_,SIGNAL(timeout),this,SLOT(RequestDMX));

    CreateListenSocket();
    StatusToHTTP(0,0);                 //向web服务器发送水帘运行状态
    SetHTTPtimer();                     //设定web服务器轮询请求定时器
    ReadtimingMission();                //读取定时任务数据库
    PrepareHeartBeat();
    LoadControlStatus();                //从数据中加载控制模式以及参数
    InitStatusBarWidget();              //根据控制模式信息初始化状态栏
	//add by eagle for ping check
	//initPingThreadData();
	///add end
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
void MainWindow::readImageSet()
{
    QSettings *configIniRead = new QSettings("imageset.ini", QSettings::IniFormat);
       //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
    QString m_strImageW = configIniRead->value("/size/first").toString();
    QString m_strImageH = configIniRead->value("/size/second").toString();

       //读入入完成后删除指针
    delete configIniRead;

    str_ipaimageW=m_strImageW;
    str_ipaimageH=m_strImageH;

}
*/
void MainWindow::newScene(QString qStrMusicName)
{

}



void MainWindow::LoadControlStatus()
{

    QSqlQuery create("CREATE table IF NOT EXISTS  `ControlInfo` ( `ControlMode` INTEGER, `CommonPort` INTEGER, `DmxPort` INTEGER, `DmxAddr` TEXT, `DataIndex` INTEGER, `DataLen` INTEGER, `Mobile` INTEGER , `HeartBeat` INTEGER ,`Loop` INTEGER )");
    create.exec();

    QSqlQuery query("SELECT * FROM ControlInfo");
    query.next();
    rcstatus_.WorkingMode   =   static_cast<remotecontrol::Mode>(query.value(0).toInt());
    rcstatus_.CommonPort    =   query.value(1).toInt();
    rcstatus_.DMXPort       =   query.value(2).toInt();
    rcstatus_.DMXAddr       =   query.value(3).toString();
    rcstatus_.DataIndex     =   query.value(4).toInt();
    rcstatus_.DataLength    =   query.value(5).toInt();
    bool mobile             =   query.value(6).toBool();
    bool heart              =   query.value(7).toBool();
    bool loop               =   query.value(8).toBool();

    if(mobile)
    {
        ui->GetMission->setChecked(true);
        on_GetMission_triggered(true);
    }
    else
    {
        ui->GetMission->setChecked(false);
        on_GetMission_triggered(false);
    }
    if(heart)
    {
        ui->heartbeatsetting->setChecked(true);
        HeartBeat_->start(1000);
    }
    else
    {
        ui->heartbeatsetting->setChecked(false);
        HeartBeat_->stop();
    }
    if(loop)
    {
        ui->loopsingle->setChecked(true);
        m_isloopMusic   =   1;
    }
    else
    {
        ui->loopsingle->setChecked(false);
        m_isloopMusic   =   0;
    }
}

void MainWindow::SaveControlStatus()
{
//    enum Mode{COMMON,DMX,NONE}; //工作模式
//    Mode    WorkingMode =   Mode::COMMON;
//    qint16  CommonPort  =   8000;     //监听端口
//    qint16  DMXPort     =   6000;     //监听端口
//    QString DMXAddr     =   "192.168.1.14";    //DMX地址
//    qint16  DataIndex   =   0;      //请求的数据起始位置
//    qint16  DataLength  =   24;             //请求的数据长度
//    QString Source      =   "";         //命令来源地址
    QSqlQuery   query;
    query.exec("delete from ControlInfo");       //先清空表，避免数据重复、冲突
    QString qStrSql = QString("insert into ControlInfo(ControlMode,CommonPort,DmxPort,DmxAddr,DataIndex,DataLen,Mobile,HeartBeat,Loop) values"
                              "('%1','%2','%3','%4','%5','%6','%7','%8','%9')").
            arg(rcstatus_.WorkingMode).
            arg(rcstatus_.CommonPort).
            arg(rcstatus_.DMXPort).
            arg(rcstatus_.DMXAddr).
            arg(rcstatus_.DataIndex).
            arg(rcstatus_.DataLength).
            arg(ui->GetMission->isChecked()).                               //保存获取任务状态
            arg(ui->heartbeatsetting->isChecked()).                          //保存心跳报文发送状态
            arg(ui->loopsingle->isChecked());                               //保存单曲循环状态
    query.exec(qStrSql);
}

void MainWindow::ReadtimingMission()
{
    //如果缺少表，就创建一个
    QSqlQuery create("CREATE table IF NOT EXISTS  `Timing` ( `Start` TEXT, `End` TEXT, `Name` TEXT, `ControlMode` TEXT )");
    create.exec();

    QSqlQuery query("SELECT * FROM Timing");
    while (query.next())
    {
        QString start = query.value(0).toString();
        QString end   = query.value(1).toString();
        QString name  = query.value(2).toString();
        QString mode  = query.value(3).toString();
        timinginfo ti;
        ti.start_   =   QDateTime::fromString(start);
        ti.end_     =   QDateTime::fromString(end);
        ti.musicname_   =   name;
        ti.ControlMode_ =   mode;
        playtiming::timinglist_.insert(ti.start_.date(),ti);
    }
}

void MainWindow::SavetimingMission()
{
    QSqlQuery   query;
    query.exec("delete from Timing");       //先清空表，避免数据重复、冲突
    for (QMultiMap<QDate, timinginfo>::const_iterator itr = playtiming::timinglist_.cbegin();
         itr != playtiming::timinglist_.cend();
         ++itr)
    {
        QString qStrSql = QString("insert into Timing(Start,End,Name,ControlMode) values('%1','%2','%3','%4')").
                arg(itr.value().start_.toString()).
                arg(itr.value().end_.toString()).
                arg(itr.value().musicname_).
                arg(itr.value().ControlMode_);
        query.exec(qStrSql);
    }
}

void MainWindow::SetHTTPtimer()
{
    HTTPtimer_  =   new QTimer;
    connect(HTTPtimer_,&QTimer::timeout,this,&MainWindow::HTTPtimerup);
}

void MainWindow::GetImage(QString imgpath)
{
    Imgreply_ = qnam_.get(QNetworkRequest(imgpath));
    connect(Imgreply_, &QNetworkReply::finished, this, &MainWindow::GetImgFinished);
}

void MainWindow::StatusToHTTP(int jobid,int statuscode)
{
    QString tmp =   m_CtrlModeFlag==1?"YES":"NO";
    QString statusstr;
    switch (statuscode) {
    case 0:
        statusstr   =   "start";
        break;
    case 1:
        statusstr   =   "stop";
        break;
    case 2:
        statusstr   =   "begin";
        qDebug()<<"this is the begin of the show!"<<"___jobid___"<<CurrentJobID_<<"is ctrl mode? "<<tmp;
        if(m_CtrlModeFlag)
        {
            //可能是重复的任务，或是正在执行手机任务,只需要等待本次执行结束，而不需要再次发送begin消息
            return;
        }
        break;
    case 3:
        statusstr   =   "end";
        qDebug()<<"this is the end of the show!";
        break;
    default:
        break;
    }
    url_    =   mobiledomain+ "/fount/api/v1/job/status";
    QUrlQuery queryinfo;
    QList<QPair<QString,QString>> pairkv;
    pairkv.push_back(QPair<QString,QString>("fountain_code","fount1"));
    pairkv.push_back(QPair<QString,QString>("job_id",QString("%1").arg(jobid)));
    pairkv.push_back(QPair<QString,QString>("status",statusstr));
    queryinfo.setQueryItems(pairkv);

    QNetworkRequest statusRequest(url_);
    statusRequest.setRawHeader("Content-Type", "application/json");

    QJsonObject json;

    json.insert("fountain_code","fount1");

    json.insert("job_id",QString("%1").arg(jobid));

    json.insert("status",statusstr);

    reply_ = qnam_.post(statusRequest,QJsonDocument(json).toJson());
//    connect(reply_, &QNetworkReply::finished, this, &MainWindow::send_status_finished);
}

void MainWindow::GetTaskFromHTTP()
{
	if (m_PlayTimeCount == 0)
    {
        //还没有播放，无需请求手机任务
    }
    else
    {
        if(CurrentJobID_==-1)           //当前没有正在执行的任务
        {
            url_    =   mobiledomain+"/fount/api/v1/job?fountain_code=fount1";
            Taskreply_  =   qnam_.get(QNetworkRequest(url_));
            connect(Taskreply_, &QNetworkReply::finished, this, &MainWindow::Get_Task_finished);
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	// Stop the thread, add by eagle for close event
	/*
	if (m_pingThread)
	{
		emit m_pingThread->stopPing();//通知线程，不在往主界面发送任何消息和ping数据
		//这里还需要，等待线程自己清理，所以不要直接delete 也不要赋值为ptrnull，会引起崩溃，wait也自动处理

		// Stop the event loop and wait for the thread to finish
		m_thread->quit();
	}
	*/


    //2软件退出 通知web服务器
    setvol(0);                              //退出软件初始化变频状态，置0
    clearDO64State();
    clearSL32();
	clearSL40();
    clearDMX512State();
    StatusToHTTP(0,1);                 //向web服务器发送水帘运行状态
	exit(0);
}

void MainWindow::InitHeartContent()
{
    //准备心跳包数据
    StaticValue::GetInstance()->
            ZeroMemoryEx(m_HeartBuffer, sizeof(m_HeartBuffer));
    m_HeartBuffer[0]='D';		//D	包头
    m_HeartBuffer[1]='X';		//X
    m_HeartBuffer[2]=0x60;		//CMD=0X60	命令行

    char t=0;
    for(int i=0;i<3;i++)
        t=t+m_HeartBuffer[i];
    m_HeartBuffer[3]=t;
}


//add show team
void MainWindow::addTeam(int show_team_id)
{

    MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
    current_widget->addItem(show_team_id);
    int music_id = current_widget->getMusicID();
    QString qStrSql = QString("insert into music_mapping(music_id,"
                              "show_team_id,music_action_id) values(%1,%2,-1)").
            arg(music_id).arg(show_team_id);
    QSqlQuery query;
    if(!query.exec(qStrSql))
    {
        qDebug() <<"insert music false!sql:"<<qStrSql<<
                   " error:"<<query.lastError().text();
        return ;
    }
}

void MainWindow::InitElecDeviceInfo()
{
    m_elec_show = new ElecDeviceShowWidget();
	m_tabWidget->addTab(m_elec_show, tr("电气设备"));
//	connect(m_elec_show,&ElecDeviceShowWidget::RectItemClicked,this,ElecDevItemClicked);
	connect(m_elec_show,SIGNAL(ElecDeviceShowWidget::RectItemClicked()),this,SLOT(ElecDevItemClicked));
	m_3D_show = new ElecDeviceShowWidget();
	m_tabWidget->addTab(m_3D_show, tr("3D 显示"));
}


//enum TreeType
//{OUTPUT_DEVICE,ELECTRIC_DEVICE,SHOWTEAMS_DEVICE,ORBIT_DEVICE,OUTLINE_DEVICE};

//修改数据以后,双击鼠标，无法到达这个函数中,也就是无法获取双击消息，或是前置鼠标处理消息有问题
void MainWindow::MouseLeftDoubleClicked(int iType,int itemID)
{
    if(OUTPUT_DEVICE == iType)
    {
        QOutPutDialog dlg(itemID);
        dlg.setModal(true);
        if(QDialog::Accepted == dlg.exec())
           tree_view->resetTreeView();
    }
    else if(ELECTRIC_DEVICE == iType)
    {
        QDeviceInfoDialog dlg(itemID);
        dlg.setModal(true);
        if(QDialog::Accepted == dlg.exec())
            tree_view->resetTreeView();
    }
    else if(SHOWTEAMS_DEVICE == iType)
    {
        QShowTeamsDialog dlg(itemID);
        dlg.setModal(true);
        if(QDialog::Accepted == dlg.exec())
            tree_view->resetTreeView();
    }
    else if(ORBIT_DEVICE == iType)
    {
        OrbitDialog dlg(itemID);
        dlg.setModal(true);
        if(QDialog::Accepted == dlg.exec())
            tree_view->resetTreeView();
    }
    else if(OUTLINE_DEVICE == iType)
    {

    }
    else if(COMP_DEVICE == iType)  //双击编曲文件
    {
		//add by eagle for music path re-link
        MusicInfo* music = StaticValue::GetInstance()->m_music_info[itemID];
		if (music->music_path.isEmpty() == false)
		{
			QFile file(music->music_path);
			if (!file.exists())
			{
				QString newMusicPath = QFileDialog::getOpenFileName(nullptr, QObject::tr("Select Music File"), "", QObject::tr("Music Files (*.mp3 *.wav)"));
				if (!newMusicPath.isEmpty())
				{
					// Update the music_path in the MusicInfo object and the database
					music->music_path = newMusicPath;
					StaticValue::GetInstance()->updateMusicPath(itemID, newMusicPath);
				}
			}
		}
		//add end for music path re-link
        int index = 0;
        int iNum = m_tabWidget->count();
        for(;index < iNum;++index)
        {
            if(m_tabWidget->tabText(index).compare(music->name) == 0)
            {
                m_tabWidget->setCurrentIndex(index);
                break;
            }
        }
        if(index == iNum)           //需要新建
        {

            int view_height = m_tabWidget->rect().height();
            int view_width = m_tabWidget->rect().width();

			//add by eagle for spectrum 
			// 创建新的 spectrumwidget
			spectrumwidget* newWidget = new spectrumwidget();
			int newWidgetHeight = 90;
			newWidget->setFixedSize(view_width, newWidgetHeight);


            MusicWidget* musicTab = new MusicWidget(m_tabWidget);
			if (music->music_path.isEmpty()==false){
				float  szleg = musicTab->getMusicLengthbypath(music->music_path)*1.0 / 1000.0;
				music->time_length = szleg;
			}
			
            int index = m_tabWidget->addTab(musicTab,music->name);
            m_tabWidget->setCurrentIndex(index);

			//add by eagle 双击逻辑的波形绘制
            // 在 newWidget 上添加 QLabel，显示文字 "音乐波形"
            // 设置 newWidget 的背景颜色为灰色
			QString szlabel = tr("音乐：%1\n时长：%2 秒")
				.arg(music->name)
				.arg(QString::number(music->time_length));
			newWidget->setLabel(szlabel);
           // newWidget->setStyleSheet("background-color: gray;");
            // 将 newWidget 添加到 musicTab 下方
            musicTab->addWidgetBelow(newWidget);
			//add end	

            connect(musicTab,SIGNAL(controler()),this,SLOT(ControlTimer()));
            connect(musicTab,SIGNAL(resetStartTime(int)),this,SLOT(ResetStartTimer(int)));
		//	connect(musicTab,&MusicWidget::TimelineFinished,this,&TimelineHasFinished);
			connect(musicTab,SIGNAL(MusicWidget::TimelineFinished),this,SLOT(TimelineHasFinished));
            musicTab->resize(QSize(view_width,view_height));
            musicTab->InitData(music);

			m_tabWidget->setTabsClosable(true);		//926给编曲标签加上关闭按钮
			
        }
    }
    else if(QUEUE_DEVICE == iType)
    {

    }
    //tree_view->setFocus();
    this->repaint();
}

void MainWindow::thread_sleep(int iTime)
{
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<iTime)
        QCoreApplication::processEvents();
}

void MainWindow::setvol(int voltage)
{
    char k  =   0;
    QByteArray  ComByte;  //指令序列
    ComByte.append("AH",2);
    ComByte.append(k);
    for(int i=0;i<16;i++)
    {
        ComByte.append(voltage);
    }
    Q_ASSERT(ComByte.size()==19);
    unsigned char t=0;
    for(int i=0;i<19;i++)
        t+=ComByte[i];
    ComByte.append(t);
    AOvolset_->writeDatagram(ComByte,QHostAddress(AOset_.ipaddr_),6000);
}

unsigned char MainWindow::hex_asc(unsigned char ccc)
{
    if(ccc<=9)
        return ccc+'0';
    else
        return ccc+'A'-10;
}

unsigned int MainWindow::asc_hex(unsigned char ccc)
{
    if((ccc>='0')&&(ccc<='9'))
        return ccc-'0';
    else if((ccc>='a')&&(ccc<='z'))
        return ccc-'a'+0x0a;
    else if((ccc>='A')&&(ccc<='Z'))
        return ccc-'A'+0x0a;
    else
        return 0;
}

void MainWindow::LoopSL32()
{
	//QDateTime currentTime = QDateTime::currentDateTime();
	//qDebug()<<"void MainWindow::LoopSL32() Start -========================" << currentTime.toString("yyyy-MM-dd hh:mm:ss.zzz");

    int i,j;
    int temp;
    int ck;
    int ct=2;	
	//QUdpSocket udp_dmnsocketonly;

    QMap<int,OutPutDevice*> out_put_device_list =
            StaticValue::GetInstance()->m_output_device;
    QMap<int,OutPutDevice*>::iterator iter = out_put_device_list.begin();

    for(;iter != out_put_device_list.end();++iter)
    {
        OutPutDevice* out_put_device = iter.value();
        if(out_put_device->bHasChange && out_put_device->type.compare("SL32") == 0)
        {
            QBitArray   arrayData(out_put_device->m_mapping.size(),false);              //二进制待发数据
            QString qStrIPAddress = out_put_device->ipAddress;
            int qStrPort = out_put_device->port;
            QMap<int,int> device_list = out_put_device->m_mapping;
            QList<int> keys = device_list.keys();
            QList<int> value_id =   device_list.values();

			QByteArray SLData(8, 0);		//数据包长度8，HEAD(3)+DATA(4)+CHECK(1)
            SLData[0] = 'D';
            SLData[1] = 'L';
            SLData[2] = '3';
            ct=3;

            int require_byte    =   out_put_device->m_mapping.size()/8;            //计算设备的控制报文所占用的字节数
            for(j = require_byte-1; j >= 0;j--)
            {
                temp=0;
                for(i = 7; i >= 0; i--)
                {
                    int device_id = device_list[keys.at(j*8+i)];
                    int istate = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;

                    ck =0x01 <<i;
				//	if( 1 == istate)
					if (istate>=128)
                    {
                        temp = temp | ck;
                    }
					if(device_id != 0)		//送屏幕显示
						m_elec_show->setItemState(device_id,istate);
                }
                SLData[ct+require_byte-j-1]     =   temp;
            }
            unsigned char check = 0;
            for(i=0;i<SLData.size()-1;i++)
            {
                check = check+SLData[i];
            }
            SLData[SLData.size()-1]   = check;

			// Create the sendDataThread instance only once
#if 0
			static SendDataThread sendDataThread;
			if (!sendDataThread.isRunning())
			{
				sendDataThread.start();
			}

			// Use the sendData method to send data
			sendDataThread.sendData(SLData, qStrIPAddress, qStrPort);
#endif				
			//}
			/////////////////////////////////////
           udp_socket.writeDatagram(SLData,SLData.size(),QHostAddress(qStrIPAddress),qStrPort);			
           out_put_device->bHasChange = false;
        }	
    }
	//currentTime = QDateTime::currentDateTime();
	//qDebug() << "void MainWindow::LoopSL32() End-========================" << currentTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
}

void MainWindow::LoopSL40()
{
	//QDateTime currentTime = QDateTime::currentDateTime();
	//qDebug()<<"void MainWindow::LoopSL32() Start -========================" << currentTime.toString("yyyy-MM-dd hh:mm:ss.zzz");

	int i, j;
	int temp;
	int ck;
	int ct = 2;
	//QUdpSocket udp_dmnsocketonly;

	QMap<int, OutPutDevice*> out_put_device_list = StaticValue::GetInstance()->m_output_device;
	QMap<int, OutPutDevice*>::iterator iter = out_put_device_list.begin();

	for (; iter != out_put_device_list.end(); ++iter)
	{
		OutPutDevice* out_put_device = iter.value();
		if (out_put_device->bHasChange && out_put_device->type.compare("SL40") == 0)
		{
			QBitArray   arrayData(out_put_device->m_mapping.size(), false);              //二进制待发数据
			QString qStrIPAddress = out_put_device->ipAddress;
			int qStrPort = out_put_device->port;
			QMap<int, int> device_list = out_put_device->m_mapping;
			QList<int> keys = device_list.keys();
			QList<int> value_id = device_list.values();

			QByteArray SLData(9, 0);	//数据包长度9，HEAD(3)+DATA(5)+CHECK(1)
			SLData[0] = 'D';
			SLData[1] = 'L';
			SLData[2] = '4';
			ct = 3;

			int require_byte = out_put_device->m_mapping.size() / 8;            //计算设备的控制报文所占用的字节数
			for (j = require_byte - 1; j >= 0; j--)
			{
				temp = 0;
				for (i = 7; i >= 0; i--)
				{
					int device_id = device_list[keys.at(j * 8 + i)];
					int istate = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;

					ck = 0x01 << i;
					//	if( 1 == istate)
					if (istate >= 128)
					{
						temp = temp | ck;
					}
					if (device_id != 0)		//送屏幕显示
						m_elec_show->setItemState(device_id, istate);
				}
				SLData[ct + require_byte - j - 1] = temp;
			}
			unsigned char check = 0;
			for (i = 0; i<SLData.size() - 1; i++)
			{
				check = check + SLData[i];
			}
			SLData[SLData.size() - 1] = check;

			// Create the sendDataThread instance only once
#if 0
			static SendDataThread sendDataThread;
			if (!sendDataThread.isRunning())
			{
				sendDataThread.start();
			}

			// Use the sendData method to send data
			sendDataThread.sendData(SLData, qStrIPAddress, qStrPort);
#endif				
			//}
			/////////////////////////////////////
			udp_socket.writeDatagram(SLData, SLData.size(), QHostAddress(qStrIPAddress), qStrPort);
			out_put_device->bHasChange = false;
		}
	}
	//currentTime = QDateTime::currentDateTime();
	//qDebug() << "void MainWindow::LoopSL32() End-========================" << currentTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
}

void MainWindow::LoopCanMain()
{
	int i, j;
	int temp;
	int ck;
	int ct = 2;

	QMap<int, OutPutDevice*> out_put_device_list =	StaticValue::GetInstance()->m_output_device;
	QMap<int, OutPutDevice*>::iterator iter = out_put_device_list.begin();

	for (; iter != out_put_device_list.end(); ++iter)
	{
		OutPutDevice* out_put_device = iter.value();
		if (out_put_device->bHasChange && out_put_device->type.compare("CanMain") == 0)
		{
			QBitArray   arrayData(out_put_device->m_mapping.size(), false);              //二进制待发数据
			QString qStrIPAddress = out_put_device->ipAddress;
			int qStrPort = out_put_device->port;
			QMap<int, int> device_list = out_put_device->m_mapping;
			QList<int> keys = device_list.keys();
			QList<int> value_id = device_list.values();

			QByteArray SLData(106, 0);	//数据包长度106，HEAD(5)+DATA(100)+CHECK(1)
			SLData[0] = 'C';	//包头CL1，连续40路CAN板卡
			SLData[1] = 'L';
			SLData[2] =  1;
			SLData[3] = 0;		//起始板卡号
			SLData[4] = 20;		//板卡数量

			ct = 5;
			int require_byte = out_put_device->m_mapping.size() / 8;            //计算设备的控制报文所占用的字节数
			for (j = require_byte - 1; j >= 0; j--)
			{
				temp = 0;
				for (i = 7; i >= 0; i--)
				{
					int device_id = device_list[keys.at(j * 8 + i)];
					int istate = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;

					ck = 0x01 << i;
					//	if( 1 == istate)
					if (istate >= 128)
					{
						temp = temp | ck;
					}
					if (device_id != 0)		//送屏幕显示
						m_elec_show->setItemState(device_id, istate);
				}
				SLData[ct + require_byte - j - 1] = temp;
			}
			unsigned char check = 0;
			for (i = 0; i<SLData.size() - 1; i++)
			{
				check = check + SLData[i];
			}
			SLData[SLData.size() - 1] = check;

			// Create the sendDataThread instance only once
#if 0
			static SendDataThread sendDataThread;
			if (!sendDataThread.isRunning())
			{
				sendDataThread.start();
			}

			// Use the sendData method to send data
			sendDataThread.sendData(SLData, qStrIPAddress, qStrPort);
#endif				
			//}
			/////////////////////////////////////
			udp_socket.writeDatagram(SLData, SLData.size(), QHostAddress(qStrIPAddress), qStrPort);
			out_put_device->bHasChange = false;
		}
	}
	//currentTime = QDateTime::currentDateTime();
	//qDebug() << "void MainWindow::LoopSL32() End-========================" << currentTime.toString("yyyy-MM-dd hh:mm:ss.zzz");

//	以下这段工作不正常
/*	out_put_device_list = StaticValue::GetInstance()->m_output_device;
	int k = out_put_device_list.size();
	iter = out_put_device_list.begin();
	for (; iter != out_put_device_list.end(); ++iter)
	{
		OutPutDevice* out_put_device = iter.value();
	//	if (out_put_device->bHasChange && out_put_device->type.compare("DMX512") == 0)
		if (out_put_device->bHasChange && out_put_device->type.compare("CanMain") == 0)
		{
			QString qStrIpAddress = out_put_device->ipAddress;
			int port = out_put_device->port;
			QMap<int, int> dmx_state = out_put_device->m_route_state;
			QMap<int, int>::iterator dmx_iter = dmx_state.begin();
			int length = dmx_state.size();
			int minRouteID = dmx_iter.key();
			m_DmxBuffer[0] = 'D';
			m_DmxBuffer[1] = 'X';
			m_DmxBuffer[2] = 0x44;

			m_DmxBuffer[3] = (minRouteID & 0x0300) >> 8;	//data_ppp MSB，0到511，最大511
			m_DmxBuffer[4] = (minRouteID & 0x00ff);		//data_ppp LSB
			//data_ppp是有数据的起始通道

			m_DmxBuffer[5] = (length & 0x0300) >> 8;	//data_len MSB，1到512，最大512
			m_DmxBuffer[6] = (length & 0x00ff);		//data_len LSB
			//data_len 是有数据的通道长度
			//e.g.从4通道开始到11通道有输出，则起始通道是4							//长度是8

			for (int i = 0; i<length; i++, dmx_iter++)	//数据
				m_DmxBuffer[7 + i] = dmx_iter.value();

			unsigned char t = 0;
			for (int i = 0; i<(length + 7); i++)  	//和校验
				t = t + m_DmxBuffer[i];
			m_DmxBuffer[length + 7] = t;

			udp_socket.writeDatagram(m_DmxBuffer, length + 8,
				QHostAddress(qStrIpAddress), port);
			//发送完改变的数据包，即将其设置为未改变状态，防止重复发相同的数据
			QByteArray tmparray(m_DmxBuffer);
			qDebug() << tmparray;
			out_put_device->bHasChange = false;
		}
	} */
}


//时基为5ms，不是很准确，约5.026ms
void MainWindow::timeOut()
{
    if(m_IsOutPutTimerOn)  //播放的编曲是否被暂停，如果被点暂停，则停止送输出
    {
        InitOutPutData(PROGRAMCONTROL_TYPE);	//获得输出的数据
        LoopDo64Device();		//组织DO64的数据包并输出
        LoopSL32();
		LoopSL40();
        LoopDmx512Device();		//组织DMX512的数据包并输出
		LoopCanMain();

        //心跳包一直存在
		m_PlayTimeCount += TIME_RES;
		TimeHaveRun += TIME_RES;		//已经运行的时间

        MusicWidget* current_widget = qobject_cast<MusicWidget *>(m_tabWidget->currentWidget());

        if(current_widget!=0)
        {
			current_widget->UpdataTime(m_PlayTimeCount);		//显示播放线顶上的时间值
			//qDebug() << "MainWindow::timeOut()" << m_HeartCount;
        }

        else          //电气设备选项卡
        {
            ElecDeviceShowWidget* Elec = qobject_cast<ElecDeviceShowWidget *>(m_tabWidget->currentWidget());
            Elec->UpdataScene();
        }
//        MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();

//        int curIndex    =    m_tabWidget->currentIndex();

    }
//	else
//		m_HeartCount += TIME_RES;		//2021-11-15增加该语句，解决不运行编曲的时候频繁发送心跳包的问题

	m_HeartCount += TIME_RES;		//2021-11-15增加该语句，解决不运行编曲的时候频繁发送心跳包的问题
    if(((int)m_HeartCount) % 1000 == 0)
    {
       SendHeartPackage();
    }
}

//计算输出的数据
void MainWindow::InitOutPutData(int iType)
{
    QMap<int,CollidingRectItem*>::iterator iter;
    for(iter = m_CurrentMusicActions.begin();iter != m_CurrentMusicActions.end();++iter)
    {
        CollidingRectItem* item = iter.value();
        int type = item->getType();     //type是编曲的命令种类
		if (m_PlayTimeCount > item->begin_time && m_PlayTimeCount < item->end_time) // >=  -> >
        {   //
            item->setIsShowing(true);
            if(ALWAYS_ON == type)
            {
                InitAlwaysOn(item);
            }
            else if(DOMINO_RUN == type)
            {
                InitDomino(item);	//多米诺	
            }
            else if(IMAGE_SHOW == type)
            {
                InitImageShow(item);
            }
            else if(DMX512_SHOW == type)
            {
                InitDMXShow(item);
            }
//             qDebug()<<"working";
        }
		else if (m_PlayTimeCount > item->end_time || m_PlayTimeCount < item->begin_time)
        {
//             qDebug()<<"white space!!!";

            //如果当前表演动作刚才正在表演，现在超出最后表演时间，就将表演动作结束,同时，下次循环，不在遍历其动作
            if(item->getIsShowing())
            {
                if(DMX512_SHOW == type)
                {
                    CloseDMX512MusicAction(item);
                    if(PROGRAMCONTROL_TYPE == iType)
                        LoopDmx512Device();
                }
                else
                {
                    CloseDO64MusicAction(item);
                    if(PROGRAMCONTROL_TYPE == iType)
                        LoopDo64Device();
                }
                item->setIsShowing(false);
            }
        }
    }
        for(iter = m_CurrentMusicActions.begin();iter != m_CurrentMusicActions.end();++iter)
        {
            CollidingRectItem* item = iter.value();
            if(item->getIsShowing())
            {
                return;
            }
        }
        if(CurrentJobID_!=-1)
        {
            //此时在空白处，向服务器下发end信号，停止之前的表演状态
            m_CtrlModeFlag=0;
            StatusToHTTP(CurrentJobID_,3);
            CurrentJobID_   =   -1;
        }
}

void MainWindow::CloseDO64MusicAction(CollidingRectItem* item)
{
    for(int i = 0; i < item->m_show_teams.size(); ++i)
    {
        QList<int> lists = item->m_show_teams.at(i);
        for(int j = 0; j < lists.size(); ++j)
        {
            int device_id = lists.at(j);
            int out_put_value = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
            if(out_put_value > 0)
            {
                int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
                StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
                StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
            }
        }
    }
}

void MainWindow::CloseDMX512MusicAction(CollidingRectItem* item)
{
    //return;
    QMap<int,RouteInfo*> routes = item->getRouteSetting();
    QMap<int,RouteInfo*>::iterator iter;
    for(iter = routes.begin();iter != routes.end();++iter)
    {
        int route_index = iter.key();
        for(int j = 0; j < item->m_show_teams.size(); ++j)
        {
            //设备列表
            QList<int> device_list = item->m_show_teams.at(j);
            for(int k = 0; k < device_list.size(); ++k)
            {
                //获取设备号
                int device_id = device_list.at(k);
                int out_put_device_id = StaticValue::GetInstance()->
                        m_device_map[device_id]->output_device_id;
                OutPutDevice* out_put_Device = StaticValue::GetInstance()->
                        m_output_device[out_put_device_id];

                //通过设备号获取通道号列表
                QList<int> route_id_list = StaticValue::GetInstance()->
                        m_route_mapping.values(device_id);
                std::sort(route_id_list.begin(),route_id_list.end());
                //通道ID
                Q_ASSERT(route_id_list.size() != 0);    //断言列表大小不为0
                int route_id = route_id_list.at(route_index);
                int route_state = StaticValue::GetInstance()->
                        m_output_device[out_put_device_id]->m_route_state[route_id];
                //                if(route_state > 0)
                //                {

                //                }
                out_put_Device->bHasChange = true;
                out_put_Device->m_route_state[route_id] = 0;
                qDebug() << "close DMX512!";
            }
        }
    }
}

void MainWindow::clearDO64State()
{
    //将DO64输出设备置空
    QMap<int,OutPutDevice*> out_put_device = StaticValue::GetInstance()->m_output_device;
    QList<int> keys = out_put_device.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        OutPutDevice* output_device = out_put_device[keys.at(i)];
        QString type = output_device->type;
        if(type.compare("DO64Ethenet") == 0)
        {
            QMap<int,int> device_list = output_device->m_mapping;
            QList<int> device_keys = device_list.keys();
            for(int i = 0; i < device_keys.size();++i)
            {
                output_device->bHasChange = true;
                int device_id = device_list[device_keys.at(i)];
                StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
            }
        }
    }
    LoopDo64Device();
}

void MainWindow::clearSL32()
{
    //将SL32输出设备置空
    QMap<int,OutPutDevice*> out_put_device = StaticValue::GetInstance()->m_output_device;
    QList<int> keys = out_put_device.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        OutPutDevice* output_device = out_put_device[keys.at(i)];
        QString type = output_device->type;
        if(type.compare("SL32") == 0)
        {
            QMap<int,int> device_list = output_device->m_mapping;
            QList<int> device_keys = device_list.keys();
            for(int i = 0; i < device_keys.size();++i)
            {
                output_device->bHasChange = true;
                int device_id = device_list[device_keys.at(i)];
                StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
            }
        }
    }
    LoopSL32();
}

void MainWindow::clearSL40()
{
	//将SL40输出设备置空
	QMap<int, OutPutDevice*> out_put_device = StaticValue::GetInstance()->m_output_device;
	QList<int> keys = out_put_device.keys();
	for (int i = 0; i < keys.size(); ++i)
	{
		OutPutDevice* output_device = out_put_device[keys.at(i)];
		QString type = output_device->type;
		if (type.compare("SL40") == 0)
		{
			QMap<int, int> device_list = output_device->m_mapping;
			QList<int> device_keys = device_list.keys();
			for (int i = 0; i < device_keys.size(); ++i)
			{
				output_device->bHasChange = true;
				int device_id = device_list[device_keys.at(i)];
				StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
			}
		}
	}
	LoopSL32();
}

void MainWindow::clearDMX512State()
{
    //将DO64输出设备置空
    QMap<int,OutPutDevice*> out_put_device = StaticValue::GetInstance()->m_output_device;
    QList<int> keys = out_put_device.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        OutPutDevice* output_device = out_put_device[keys.at(i)];
        QString type = output_device->type;
        if(type.compare("DMX512") == 0)
        {
            output_device->bHasChange = true;
            for(int j = 0 ; j < 512; ++j)
            {
                output_device->m_route_state[j] = 0;
            }
        }
    }
    LoopDmx512Device();
    //将DO64输出设备置空
    QMap<int,OutPutDevice*> out_put_device2 = StaticValue::GetInstance()->m_output_device;
    QList<int> keys2 = out_put_device2.keys();
    for(int i = 0; i < keys2.size(); ++i)
    {
        OutPutDevice* output_device = out_put_device2[keys2.at(i)];
        QString type = output_device->type;
        if(type.compare("DMX512") == 0)
        {
            output_device->m_route_state.clear();
        }
    }
}

void MainWindow::InitAlwaysOn(CollidingRectItem* item)
{
    for(int i = 0; i < item->m_show_teams.size(); ++i)
    {
        QList<int> lists = item->m_show_teams.at(i);
        for(int j = 0; j < lists.size(); ++j)
        {
            int device_id = lists.at(j);
            int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
            if(device_state <= 0)
            {
                int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
                StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
				StaticValue::GetInstance()->m_device_map[device_id]->out_put_value =  255;
            }
        }
    }
}

//SUN移植
void MainWindow::InitDomino(CollidingRectItem* item)
{
	//m_HeartCount是总运行时刻
	//item->begin_time是本段编曲的起始时刻
	//iHeartTime是本段编曲段内的时刻，本段编曲运行了的时间长度
	int iHeartTime = m_PlayTimeCount - item->begin_time;
	QList<QList<int>> show_teams = item->m_show_teams;	//获得本段编曲的编队号码
	int loop_times = item->m_loop_time;		//循环次数
	int delay_time = item->m_delay_time;	//延迟时间
	int keep_time = item->m_keep_time;		//保持时间
	int perial_time = item->m_perial_time;	//波浪周期时间

	int out_value1 = item->m_out_value1;	//输出值1
	int out_value2 = item->m_out_value2;	//输出值2
	int start_delay= item->m_start_delay;	//启动延迟

	int reverse_flag = item->m_reverse_flag;//反向的标志，1：反向，：不反向
	int from_left = item->m_from_left;	//左起的标志，1：左起，0：填充

	int total_perial_time = delay_time*show_teams.size();		//周期时间为延迟时间乘以表演编队的组的个数
	
	if(0==from_left)
		iHeartTime = iHeartTime + total_perial_time;

	iHeartTime = iHeartTime - start_delay;

	iHeartTime = (iHeartTime / 5) * 5;	//归一化时间

	int iTempTime;			//暂存一下本段编曲运行了的时间长度
	int curr_point = 0;

	if (delay_time < 0)		//延迟时间不能为负
		return;

	if (delay_time == 0)
	{	//延迟时间为0，全关全开运行
		//是不是要结束
		//屏蔽的这段压根执行不到，条件永远不成立
	/*	item->m_curr_loop_time = iHeartTime / perial_time;		//当前循环了多少个周期
		if (loop_times != -1 && item->m_curr_loop_time >= loop_times)	//无限循环和超出循环次数时
		{
			item->m_curr_loop_time = 0;

			stopTimer();	//清除输出数据并发送到板卡，将来改为仅更新数据，最后统一比对发送

			for (int i = 0; i < show_teams.size(); i++)
			{
				//并且把保持的标志清除
				item->keep_time_flag[i] = 0;
				item->keep_time[i] = 0;

				//把所有内部数据都清除
				QList<int> show_device_list = show_teams.at(i);
				for (int k = 0; k < show_device_list.size(); ++k)
				{	//对编队的一个组的所有设备进行操作，都无效
					int device_id = show_device_list.at(k);
					StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
				}
			}
			return;
		} */

		//计算是否要关闭
		for (int j = 0; j < show_teams.size(); j++)
		{
			//所有有效的分组计时
			if (1 == item->keep_time_flag[j])
				item->keep_time[j] = item->keep_time[j] + 5;

			//如果计时超过保持时间，就关闭
			if (item->keep_time[j] >= keep_time)
			{
				//如果超时就关闭
				QList<int> close_device_list = show_teams.at(j);
				for (int k = 0; k < close_device_list.size(); ++k)
				{	//对编队的一个组的所有设备进行操作，都无效
					int device_id = close_device_list.at(k);
					int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
					if (device_state != out_value2)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
					{
						int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
						StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
						StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value2;
					}
				}

				//超时的分组不再计时
				item->keep_time_flag[j] = 0;
				item->keep_time[j] = 0;
			}
		}

		//把所有有效的分组都输出
		for (int i = 0; i < show_teams.size(); ++i)
		{
			if (1 == item->keep_time_flag[i])
			{
				QList<int> show_device_list = show_teams.at(i);
				for (int k = 0; k < show_device_list.size(); ++k)
				{	//对编队的一个组的所有设备进行操作，都无效
					int device_id = show_device_list.at(k);
					int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
					if (device_state != out_value1)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
					{
						int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
						StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
						StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value1;
					}
				}
			}
		}

		//延迟时间是打开新组的时间
		//只在等于延迟时间的时候新组才会有效
		int time_temp_set = iHeartTime % perial_time;
		if (time_temp_set == 0)
		{
			//所有的组都有效
			for (int i = 0; i < show_teams.size(); ++i)
			{
				item->keep_time[i] = 0;
				item->keep_time_flag[i] = 1;

				QList<int> show_device_list = show_teams.at(i);
				for (int k = 0; k < show_device_list.size(); ++k)
				{	//对编队的一个组的所有设备进行操作，都无效
					int device_id = show_device_list.at(k);
					int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
					if (device_state <= 0)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
					{
						int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
						StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
						StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value1;
					}
				}
			}
		}
	}
	else
	{
		if (total_perial_time <= perial_time)	//如果理论周期时间等于实际设置的周期时间
		{
			//是不是要结束
		/*	item->m_curr_loop_time = iHeartTime / perial_time;		//当前循环了多少个周期
			if (loop_times != -1 && item->m_curr_loop_time >= loop_times)	//无限循环和超出循环次数时
			{
				item->m_curr_loop_time = 0;

				stopTimer();	//清除输出数据并发送到板卡，将来改为仅更新数据，最后统一比对发送

				for (int i = 0; i < show_teams.size(); i++)
				{
					//并且把保持的标志清除
					item->keep_time_flag[i] = 0;
					item->keep_time[i] = 0;

					//把所有内部数据都清除
					QList<int> show_device_list = show_teams.at(i);
					for (int k = 0; k < show_device_list.size(); ++k)
					{	//对编队的一个组的所有设备进行操作，都无效
						int device_id = show_device_list.at(k);
						StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
					}
				}
				return;
			} */

			//计算是否要关闭
			for (int j = 0; j < show_teams.size(); j++)
			{
				if (reverse_flag)
					curr_point = show_teams.size() - j -1;
				else
					curr_point = j;

				if (curr_point>=0)
				{
					//所有有效的分组计时
					if (1 == item->keep_time_flag[curr_point])
						item->keep_time[curr_point] = item->keep_time[curr_point] + 5;

					//如果计时超过保持时间，就关闭
					if (item->keep_time[curr_point] >= keep_time)
					{
						//如果超时就关闭
						QList<int> close_device_list = show_teams.at(curr_point);
						for (int k = 0; k < close_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = close_device_list.at(k);
							int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
							if (device_state != out_value2)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
							{
								int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value2;
							}
						}

						//超时的分组不再计时
						item->keep_time_flag[curr_point] = 0;
						item->keep_time[curr_point] = 0;
					}
				}
			}

			//把所有有效的分组都输出
			for (int i = 0; i < show_teams.size(); ++i)
			{
				if (reverse_flag)
					curr_point = show_teams.size() - i -1;
				else
					curr_point = i;

				if (curr_point >= 0)
				{
					if (1 == item->keep_time_flag[curr_point])
					{
						QList<int> show_device_list = show_teams.at(curr_point);
						for (int k = 0; k < show_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = show_device_list.at(k);
							int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
							if (device_state != out_value1)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
							{
								int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value1;
							}
						}
					}
				}
			}

			//延迟时间是打开新组的时间
			//只在等于延迟时间的时候新组才会有效
			int time_temp_set = iHeartTime % perial_time;
			if ((time_temp_set%delay_time) == 0)
			{
				//计算那个分组处于有效期
				int donoff_point = time_temp_set / delay_time;

				if (reverse_flag)
					curr_point = show_teams.size() - donoff_point -1;
				else
					curr_point = donoff_point;
				if (curr_point >= 0)
				{
					if ((item->keep_time_flag[curr_point] == 0) && (curr_point < show_teams.size()))
					{
						item->keep_time[curr_point] = 0;
						item->keep_time_flag[curr_point] = 1;

						//让这个分组有效，并输出
						QList<int> show_device_list = show_teams.at(curr_point);
						for (int k = 0; k < show_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = show_device_list.at(k);
							int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
							if (device_state != out_value1)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
							{
								int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value1;
							}
						}
					}
				}
			}
		}
		else if (total_perial_time > perial_time)	//如果理论周期时间大于实际设置的周期时间（追逐）
		{
			//是不是要结束
		/*	item->m_curr_loop_time = iHeartTime / total_perial_time;		//当前循环了多少个周期
			if (loop_times != -1 && item->m_curr_loop_time >= loop_times)	//无限循环和超出循环次数时
			{
				item->m_curr_loop_time = 0;

				stopTimer();	//清除输出数据并发送到板卡，将来改为仅更新数据，最后统一比对发送

				for (int i = 0; i < show_teams.size(); i++)
				{
					//并且把保持的标志清除
					item->keep_time_flag[i] = 0;
					item->keep_time[i] = 0;

					//把所有内部数据都清除
					QList<int> show_device_list = show_teams.at(i);
					for (int k = 0; k < show_device_list.size(); ++k)
					{	//对编队的一个组的所有设备进行操作，都无效
						int device_id = show_device_list.at(k);
						StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
					}
				}
				return;
			} */

			//计算是否要关闭
			for (int j = 0; j < show_teams.size(); j++)
			{
				if (reverse_flag)
					curr_point = show_teams.size() - j -1;
				else
					curr_point = j;

				if (curr_point >= 0)
				{
					//所有有效的分组计时
					if (1 == item->keep_time_flag[curr_point])
						item->keep_time[curr_point] = item->keep_time[curr_point] + 5;

					//如果计时超过保持时间，就关闭
					if (item->keep_time[curr_point] >= keep_time)
					{
						//如果超时就关闭
						QList<int> close_device_list = show_teams.at(curr_point);
						for (int k = 0; k < close_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = close_device_list.at(k);
							int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
							if (device_state != out_value2)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
							{
								int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value2;
							}
						}

						//超时的分组不再计时
						item->keep_time_flag[curr_point] = 0;
						item->keep_time[curr_point] = 0;
					}
				}
			}

			//把所有有效的分组都输出
			for (int i = 0; i < show_teams.size(); ++i)
			{
				if (reverse_flag)
					curr_point = show_teams.size() - i -1;
				else
					curr_point = i;

				if (curr_point >= 0)
				{
					if (1 == item->keep_time_flag[curr_point])
					{
						QList<int> show_device_list = show_teams.at(curr_point);
						for (int k = 0; k < show_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = show_device_list.at(k);
							int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
							if (device_state != out_value1)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
							{
								int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value1;
							}
						}
					}
				}
			}

			//		int time_temp_set = iHeartTime % perial_time; //是否到了设置的周期时间
			if ((iHeartTime%delay_time) == 0)
			{	//-1循环的情况
				if (loop_times == -1)
				{
					int ttt = 0;
					do{
						iTempTime = (iHeartTime % (perial_time*total_perial_time)) - ttt*perial_time;
					//	iTempTime = (iHeartTime % total_perial_time) - ttt*perial_time;

						if (iTempTime >= 0)
						{
							int donoff_point = iTempTime / delay_time;

							if (reverse_flag)
								curr_point = show_teams.size() - donoff_point -1;
							else
								curr_point = donoff_point;

							if (curr_point >= 0)
							{
								if ((item->keep_time_flag[curr_point] == 0) && (curr_point < show_teams.size()))
								{
									item->keep_time[curr_point] = 0;
									item->keep_time_flag[curr_point] = 1;

									//让这个分组有效，并输出
									QList<int> show_device_list = show_teams.at(curr_point);
									for (int k = 0; k < show_device_list.size(); ++k)
									{	//对编队的一个组的所有设备进行操作，都无效
										int device_id = show_device_list.at(k);
										int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
										if (device_state != out_value1)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
										{
											int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
											StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
											StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value1;
										}
									}
								}
							}
						}
						ttt++;
					} while (iTempTime >= 0);
				}

				//不是无限循环
				if (loop_times != -1)
				{
					int ttt = 0;
					do{
						//	iTempTime = (iHeartTime % (perial_time*total_perial_time)) - ttt*perial_time;
						iTempTime = iHeartTime - ttt*perial_time;

						if (iTempTime >= 0)
						{
							int donoff_point = iTempTime / delay_time;

							if (reverse_flag)
								curr_point = show_teams.size() - donoff_point -1;
							else
								curr_point = donoff_point;

							if (curr_point >= 0)
							{
								if ((item->keep_time_flag[curr_point] == 0) && (curr_point < show_teams.size()))
								{
									item->keep_time[curr_point] = 0;
									item->keep_time_flag[curr_point] = 1;

									//让这个分组有效，并输出
									QList<int> show_device_list = show_teams.at(curr_point);
									for (int k = 0; k < show_device_list.size(); ++k)
									{	//对编队的一个组的所有设备进行操作，都无效
										int device_id = show_device_list.at(k);
										int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
										if (device_state != out_value1)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
										{
											int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
											StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
											StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value1;
										}
									}
								}
							}
						}
						ttt++;
					} while ((iTempTime >= 0) && (ttt < loop_times));
				}
			}
		}
	}
}
//扭曲图像
void MainWindow::InitImageShow(CollidingRectItem* item)
{
//#ifdef  OLD_InitImageShow
//	int tt = 0;
//#endif
#if INIT_IMAGE_SHOW==3
	int loop_times, delay_time;
	QSize img_size;					//
	//	int iHeartTime = m_HeartCount - item->begin_time;
	int iHeartTime;

	QList<QList<int>> show_teams = item->m_show_teams;	//当前表演编队
	QImage last_image;
	QSize sendimagesize;
	int sendimagelooptimes;
	int sendimage_perialtime;			//苹果图片的一次掉落时间

	if (m_CtrlModeFlag == 0)
	{
		//不是收到了ipad发送的图片，正常表演
		iHeartTime = m_PlayTimeCount - item->begin_time;	//
		loop_times = item->m_loop_time;
		delay_time = item->m_delay_time;
		img_size = item->new_image.size();
		last_image = item->new_image;
	}
	else
	{
		//收到了ipad发送的图片，ipad互动
		//		if(CurrentJobID_!=-1)
		//		{
		//			qDebug()<<"prepare to end the mission!!!!!!!!!!!!!!!!!!!!!!!!!!";
		//			m_CtrlModeFlag=0;
		//			StatusToHTTP(CurrentJobID_,3);
		//			CurrentJobID_   =   -1;
		//			//之前有正在表演的手机图片，需要先结束掉
		//		}
		iHeartTime = m_PlayTimeCount - ipadimage_currtenttime;		//ipadimage_currtenttime是获得ipad图片的时刻
		//		sendimagelooptimes = 5;
		//		delay_time = 10;
		sendimagelooptimes = StaticValue::GetInstance()->str_ipaimageTimes.toInt();		//掉落的次数
		delay_time = StaticValue::GetInstance()->str_ipaimageDelayTime.toInt();			//间隔时间
		last_image.loadFromData(last_image_array);						//获得图片数据
		last_image = last_image.scaledToWidth(show_teams.size());		//缩放图片，宽度适配编组数量

		sendimagesize = last_image.size();			//适配尺寸后图片的大小，应该是包含了宽度和高度信息？
		//	img_size = last_image.size();
		img_size = sendimagesize;					//和非ipad的普通图片一致起来，普通图片的长宽等尺寸信息也在img_size变量内
		sendimage_perialtime = delay_time * (sendimagesize.height() - 1);	//ipad图片的一次掉落的所用时间
	}

	//以下操作普通图片和ipad图片是一样的
	int perial_time = delay_time * (img_size.height() - 1);	//一个完整图片掉落的时长

	//	int row_num = (iHeartTime%perial_time) / delay_time;	//图片的当前行 
	int current_padimage_looptime = iHeartTime / (delay_time * (sendimagesize.height() - 1));		//ipad图片周期，已经掉落的次数
	int current_loop_time = iHeartTime / (delay_time * (img_size.height() - 1));					//判断走了多少个周期
	if (m_CtrlModeFlag == 0)
	{
		//普通普片的情况
		if (current_loop_time >= loop_times && loop_times != -1)
		{
			stopTimer();
			return;
		}
	}
	else
	{
		//ipad的情况
		qDebug() << "ipad图片周期" << current_padimage_looptime << "判断走了多少个周期" << current_loop_time;
		if (current_padimage_looptime >= sendimagelooptimes && sendimagelooptimes != -1)
		{
			//远程控制图片播放完毕
			m_CtrlModeFlag = 0;			//清除ipad有图片的标志
			StatusToHTTP(CurrentJobID_, 3);
			CurrentJobID_ = -1;
			//	ipadimage_currtenttime=item->begin_time;
			//	stopTimer();
			return;
		}
	}

	//	int row_num = (iHeartTime%perial_time) / delay_time;	//图片的当前行
	//	int show_num = img_size.height() - row_num - 1;		//当前输出的图片的行号

	//	if (iHeartTime % delay_time <= TIME_RES)
	//	{
			//每timeout一次就执行到这里一次，这个if没有啥用，屏蔽了没啥变化，估计对比值大于TIME_RES才有用
		//	if (m_CtrlModeFlag == 1)
		//	{
				//	qDebug()<<"outer loop";
		//		int i = 100;
		//		++i;
				//	qDebug()<<img_size;
		//	}
	const int   Valve_Num = StaticValue::GetInstance()->m_SysConfig.value("VALVE_NUM").toInt();//120;
	float MAX = 0.0f;
			//当前一行内，宽度上的点循环
	for (int i = 0; i < img_size.width(); ++i)       //第二遍循环挂掉
	{
		QList<int> device_list = show_teams.at(i);

		//根据图片的像素点，找到自己编队内的设备号
		for (int k = 0; k < device_list.size(); ++k)	//device_list.size()的值是1，只对应一个像素点，可能多米诺跑动的会大于1
		{
			DeviceInfo* device_info = new DeviceInfo();
			//找到i像素点对应的水阀ID
			int device_id = device_list.at(k);		//设备号
			int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;	//输出值
			
			
			////anzs code for delay time for each valve
			const float Arc_Length = 3.0f;
			const float Arc_Height = 2.0f;


			const float Tank_Height = 0.36;
			const float g = 9.8;
			float flope = .0f;
			const float deltaX = Arc_Length / (Valve_Num);
			float height = .0f;

			//弧形水箱，可以确定最高处在中间阀门处，即Valve_Num / 2处，延时应为0；
			flope = (Arc_Height - Tank_Height) / (Arc_Length / 2); // 斜率，用斜线拟合弧形 deltaY/deltaX
			height = Tank_Height + flope * deltaX * (Valve_Num / 2);
			float t = sqrt(2 * height / g);//自由落体到地面的时间 t = sqrt(2*height/g)
			MAX = round(t * 1000);
			device_info->delayinms = 0;// round(t * 1000);



			if (device_info->id <= Valve_Num / 2)
			{
				flope = (Arc_Height - Tank_Height) / (Arc_Length / 2); // 斜率，用斜线拟合弧形 deltaY/deltaX
				height = Tank_Height + flope * deltaX * (device_info->id);
			}
			else
				if (device_info->id > Valve_Num / 2)
				{
					flope = (Tank_Height - Arc_Height) / (Arc_Length / 2);
					height = Arc_Height + flope * deltaX * (device_info->id - Valve_Num / 2);
				}


			t = sqrt(2 * height / g);//自由落体到地面的时间 t = sqrt(2*height/g)

			//t = 0;// for 扭曲图像

			device_info->delayinms = round(t * 1000);//i+1;										//创建的时候缺省的延时值
			//device_info->delayinms = round(t * 1000)*1000;//i+1;										//为看效果，放大延迟量
			device_info->delayinms = MAX - device_info->delayinms;




			//out_put_device->m_delay_ms[i] = 0;// device_info->delayinms;			//延时值，缺省初值都是0

			
			
			float PixelDelayInms = device_info->delayinms;// StaticValue::GetInstance()->m_device_map[device_id]->delayinms;	//延时值
			//anzs for test 为了清晰可见
			//PixelDelayInms *= 10;
			//这段延迟时间内，像素数值方向移动的个数



			double deltaY = 0.5 * 9.8 * (PixelDelayInms/1000) * (PixelDelayInms/1000);
			int CurrentPixelTime = iHeartTime - PixelDelayInms;
			if (CurrentPixelTime >= 0)
			{
				int row_num = (CurrentPixelTime % perial_time) / delay_time;	//图片的当前行
				int show_num = img_size.height() - row_num - 1;				//当前输出的图片的行号，不是从0行输出，是从最大行开始输出，图片最底部
				if (CurrentPixelTime % delay_time <= TIME_RES)
				{
					if (QColor(last_image.pixel(k, show_num)) == Qt::black)		//感觉这里的i应该是变量k
					{
						//图形像素点为黑，图形非空白处的意思
						if (item->m_IsYangWen)
						{	//阳文
							if (device_state <= 0)   //此动作在关闭时，才将其打开，并改变设备状态，如果处于打开状态，那么不处理
							{
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;			//设置对应的设备为开
								int output_device_id = StaticValue::GetInstance()->
									m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;	//数据有变化
							}
						}
						else
						{	//阴文
							if (device_state > 0)
							{
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
								int output_device_id = StaticValue::GetInstance()->
									m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
							}
						}
					}
					//	else if(QColor(item->new_image.pixel(i,show_num)) == Qt::white)
					else if (QColor(last_image.pixel(k, show_num)) == Qt::white)
					{
						//图片像素点为白，空白处的意思
						if (item->m_IsYangWen)
						{	//阳文
							if (device_state > 0)
							{
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
								int output_device_id = StaticValue::GetInstance()->
									m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
							}
						}
						else
						{	//阴文
							if (device_state <= 0)
							{
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;
								int output_device_id = StaticValue::GetInstance()->
									m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
							}
						}
					}
				}
				else
				{
					//	qDebug() << "i=" << i << ",iHeartTime=" << iHeartTime << ",PixelDelayInms=" << PixelDelayInms <<
					//				",CurrentPixelTime=" << CurrentPixelTime << ",show_num=" << show_num << " 0";
				}
			}
		}

	}
	//	}
#elif (INIT_IMAGE_SHOW==2  ||INIT_IMAGE_SHOW==4)//新的InitImageShow 方法
{
	int loop_times, delay_time;
	QSize img_size;					//
	//	int iHeartTime = m_HeartCount - item->begin_time;
	int iHeartTime;

	QList<QList<int>> show_teams = item->m_show_teams;	//当前表演编队
	QImage last_image;
	QSize sendimagesize;
	int sendimagelooptimes;
	int sendimage_perialtime;			//苹果图片的一次掉落时间

	if (m_CtrlModeFlag == 0)
	{
		//不是收到了ipad发送的图片，正常表演
		iHeartTime = m_PlayTimeCount - item->begin_time;	//
		loop_times = item->m_loop_time;
		delay_time = item->m_delay_time;
		img_size = item->new_image.size();
		last_image = item->new_image;
	}
	else
	{
		//收到了ipad发送的图片，ipad互动
		//		if(CurrentJobID_!=-1)
		//		{
		//			qDebug()<<"prepare to end the mission!!!!!!!!!!!!!!!!!!!!!!!!!!";
		//			m_CtrlModeFlag=0;
		//			StatusToHTTP(CurrentJobID_,3);
		//			CurrentJobID_   =   -1;
		//			//之前有正在表演的手机图片，需要先结束掉
		//		}
		iHeartTime = m_PlayTimeCount - ipadimage_currtenttime;		//ipadimage_currtenttime是获得ipad图片的时刻
		//		sendimagelooptimes = 5;
		//		delay_time = 10;
		sendimagelooptimes = StaticValue::GetInstance()->str_ipaimageTimes.toInt();		//掉落的次数
		delay_time = StaticValue::GetInstance()->str_ipaimageDelayTime.toInt();			//间隔时间
		last_image.loadFromData(last_image_array);						//获得图片数据
		last_image = last_image.scaledToWidth(show_teams.size());		//缩放图片，宽度适配编组数量

		sendimagesize = last_image.size();			//适配尺寸后图片的大小，应该是包含了宽度和高度信息？
		//	img_size = last_image.size();
		img_size = sendimagesize;					//和非ipad的普通图片一致起来，普通图片的长宽等尺寸信息也在img_size变量内
		sendimage_perialtime = delay_time * (sendimagesize.height() - 1);	//ipad图片的一次掉落的所用时间
	}

	//以下操作普通图片和ipad图片是一样的
	int perial_time = delay_time * (img_size.height() - 1);	//一个完整图片掉落的时长

	//	int row_num = (iHeartTime%perial_time) / delay_time;	//图片的当前行 
	int current_padimage_looptime = iHeartTime / (delay_time * (sendimagesize.height() - 1));		//ipad图片周期，已经掉落的次数
	int current_loop_time = iHeartTime / (delay_time * (img_size.height() - 1));					//判断走了多少个周期
	if (m_CtrlModeFlag == 0)
	{
		//普通普片的情况
		if (current_loop_time >= loop_times && loop_times != -1)
		{
			stopTimer();
			return;
		}
	}
	else
	{
		//ipad的情况
		qDebug() << "ipad图片周期" << current_padimage_looptime << "判断走了多少个周期" << current_loop_time;
		if (current_padimage_looptime >= sendimagelooptimes && sendimagelooptimes != -1)
		{
			//远程控制图片播放完毕
			m_CtrlModeFlag = 0;			//清除ipad有图片的标志
			StatusToHTTP(CurrentJobID_, 3);
			CurrentJobID_ = -1;
			//	ipadimage_currtenttime=item->begin_time;
			//	stopTimer();
			return;
		}
	}

	//	int row_num = (iHeartTime%perial_time) / delay_time;	//图片的当前行
	//	int show_num = img_size.height() - row_num - 1;		//当前输出的图片的行号

	//	if (iHeartTime % delay_time <= TIME_RES)
	//	{
			//每timeout一次就执行到这里一次，这个if没有啥用，屏蔽了没啥变化，估计对比值大于TIME_RES才有用
		//	if (m_CtrlModeFlag == 1)
		//	{
				//	qDebug()<<"outer loop";
		//		int i = 100;
		//		++i;
				//	qDebug()<<img_size;
		//	}

			//当前一行内，宽度上的点循环
	for (int i = 0; i < img_size.width(); ++i)       //第二遍循环挂掉
	{
		QList<int> device_list = show_teams.at(i);

		//根据图片的像素点，找到自己编队内的设备号
		for (int k = 0; k < device_list.size(); ++k)	//device_list.size()的值是1，只对应一个像素点，可能多米诺跑动的会大于1
		{
			//找到i像素点对应的水阀ID
			int device_id = device_list.at(k);		//设备号
			int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;	//输出值
			int PixelDelayInms = StaticValue::GetInstance()->m_device_map[device_id]->delayinms;	//延时值
			//anzs for test 为了清晰可见
			PixelDelayInms *= 10;
			int CurrentPixelTime = iHeartTime - PixelDelayInms;
			if (CurrentPixelTime >= 0)
			{
				int row_num = (CurrentPixelTime % perial_time) / delay_time;	//图片的当前行
				int show_num = img_size.height() - row_num - 1;				//当前输出的图片的行号，不是从0行输出，是从最大行开始输出，图片最底部
				if (CurrentPixelTime % delay_time <= TIME_RES)
				{
					if (QColor(last_image.pixel(k, show_num)) == Qt::black)		//感觉这里的i应该是变量k
					{
						//图形像素点为黑，图形非空白处的意思
						if (item->m_IsYangWen)
						{	//阳文
							if (device_state <= 0)   //此动作在关闭时，才将其打开，并改变设备状态，如果处于打开状态，那么不处理
							{
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;			//设置对应的设备为开
								int output_device_id = StaticValue::GetInstance()->
									m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;	//数据有变化
							}
						}
						else
						{	//阴文
							if (device_state > 0)
							{
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
								int output_device_id = StaticValue::GetInstance()->
									m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
							}
						}
					}
					//	else if(QColor(item->new_image.pixel(i,show_num)) == Qt::white)
					else if (QColor(last_image.pixel(k, show_num)) == Qt::white)
					{
						//图片像素点为白，空白处的意思
						if (item->m_IsYangWen)
						{	//阳文
							if (device_state > 0)
							{
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
								int output_device_id = StaticValue::GetInstance()->
									m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
							}
						}
						else
						{	//阴文
							if (device_state <= 0)
							{
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;
								int output_device_id = StaticValue::GetInstance()->
									m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
							}
						}
					}
				}
				else
				{
					//	qDebug() << "i=" << i << ",iHeartTime=" << iHeartTime << ",PixelDelayInms=" << PixelDelayInms <<
					//				",CurrentPixelTime=" << CurrentPixelTime << ",show_num=" << show_num << " 0";
				}
			}
		}
	}
	//	}
}
#elif INIT_IMAGE_SHOW==1 //新的InitImageShow 方法
{
	int loop_times, delay_time;
	QSize img_size;					//
	//	int iHeartTime = m_HeartCount - item->begin_time;
	int iHeartTime;

	QList<QList<int>> show_teams = item->m_show_teams;	//当前表演编队
	QImage last_image;
	QSize sendimagesize;
	int sendimagelooptimes;
	int sendimage_perialtime;			//苹果图片的一次掉落时间
	if (m_CtrlModeFlag == 0)
	{
		//不是收到了ipad发送的图片，正常表演
		iHeartTime = m_PlayTimeCount - item->begin_time;	//
		loop_times = item->m_loop_time;
		delay_time = item->m_delay_time;
		img_size = item->new_image.size();
		last_image = item->new_image;
	}
	else
	{
		//收到了ipad发送的图片，ipad互动
		//		if(CurrentJobID_!=-1)
		//		{
		//			qDebug()<<"prepare to end the mission!!!!!!!!!!!!!!!!!!!!!!!!!!";
		//			m_CtrlModeFlag=0;
		//			StatusToHTTP(CurrentJobID_,3);
		//			CurrentJobID_   =   -1;
		//			//之前有正在表演的手机图片，需要先结束掉
		//		}
		iHeartTime = m_PlayTimeCount - ipadimage_currtenttime;		//ipadimage_currtenttime是获得ipad图片的时刻
		//		sendimagelooptimes = 5;
		//		delay_time = 10;
		sendimagelooptimes = StaticValue::GetInstance()->str_ipaimageTimes.toInt();		//掉落的次数
		delay_time = StaticValue::GetInstance()->str_ipaimageDelayTime.toInt();			//间隔时间
		last_image.loadFromData(last_image_array);						//获得图片数据
		last_image = last_image.scaledToWidth(show_teams.size());		//缩放图片，宽度适配编组数量

		sendimagesize = last_image.size();			//适配尺寸后图片的大小，应该是包含了宽度和高度信息？
		//	img_size = last_image.size();
		img_size = sendimagesize;					//和非ipad的普通图片一致起来，普通图片的长宽等尺寸信息也在img_size变量内
		sendimage_perialtime = delay_time * (sendimagesize.height() - 1);	//ipad图片的一次掉落的所用时间
	}

	//以下操作普通图片和ipad图片是一样的
	int perial_time = delay_time * (img_size.height() - 1);	//一个完整图片掉落的时长

	int row_num = (iHeartTime % perial_time) / delay_time;	//图片的当前行
	int current_padimage_looptime = iHeartTime / (delay_time * (sendimagesize.height() - 1));		//ipad图片周期，已经掉落的次数
	int current_loop_time = iHeartTime / (delay_time * (img_size.height() - 1));					//判断走了多少个周期
	if (m_CtrlModeFlag == 0)
	{
		//普通普片的情况
		if (current_loop_time >= loop_times && loop_times != -1)
		{
			stopTimer();
			return;
		}
	}
	else
	{
		//ipad的情况
		qDebug() << "ipad图片周期" << current_padimage_looptime << "判断走了多少个周期" << current_loop_time;
		if (current_padimage_looptime >= sendimagelooptimes && sendimagelooptimes != -1)
		{
			//远程控制图片播放完毕
			m_CtrlModeFlag = 0;			//清除ipad有图片的标志
			StatusToHTTP(CurrentJobID_, 3);
			CurrentJobID_ = -1;
			//	ipadimage_currtenttime=item->begin_time;
			//	stopTimer();
			return;
		}
	}

	int show_num = img_size.height() - row_num - 1;		//当前输出的图片的行号
	if (iHeartTime % delay_time <= TIME_RES)
	{
		//每timeout一次就执行到这里一次，这个if没有啥用，屏蔽了没啥变化，估计对比值大于TIME_RES才有用
		if (m_CtrlModeFlag == 1)
		{
			//	qDebug()<<"outer loop";
			int i = 100;
			++i;
			//	qDebug()<<img_size;
		}

		//当前一行内，宽度上的点循环
		for (int i = 0; i < img_size.width(); ++i)       //第二遍循环挂掉
		{
			//根据图片的像素点，找到自己编队内的设备号
		//	for(int j = 0; j < show_teams.size(); j++)
		//	{
			QList<int> device_list = show_teams.at(i);
			for (int k = 0; k < device_list.size(); ++k)	//device_list.size()的值是1，只对应一个像素点，可能多米诺跑动的会大于1
			{
				//找到i像素点对应的水阀ID
				int device_id = device_list.at(k);
				int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
				//	if(QColor(item->new_image.pixel(i,show_num)) == Qt::black)
				if (QColor(last_image.pixel(i, show_num)) == Qt::black)		//感觉这里的i应该是变量k
				{
					//图形像素点为黑，图形非空白处的意思
					if (item->m_IsYangWen)
					{	//阳文
						if (device_state <= 0)   //此动作在关闭时，才将其打开，并改变设备状态，如果处于打开状态，那么不处理
						{
							//anzs deviceInfo* device_info = new DeviceInfo();
							StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;			//设置对应的设备为开
							int output_device_id = StaticValue::GetInstance()->
								m_device_map[device_id]->output_device_id;
							StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;	//数据有变化

						}
					}
					else
					{	//阴文
						if (device_state > 0)
						{
							StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
							int output_device_id = StaticValue::GetInstance()->
								m_device_map[device_id]->output_device_id;
							StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
						}
					}
				}
				//	else if(QColor(item->new_image.pixel(i,show_num)) == Qt::white)
				else if (QColor(last_image.pixel(i, show_num)) == Qt::white)
				{
					//图片像素点为白，空白处的意思
					if (item->m_IsYangWen)
					{	//阳文
						if (device_state > 0)
						{
							StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
							int output_device_id = StaticValue::GetInstance()->
								m_device_map[device_id]->output_device_id;
							StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
						}
					}
					else
					{	//阴文
						if (device_state <= 0)
						{
							StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;
							int output_device_id = StaticValue::GetInstance()->
								m_device_map[device_id]->output_device_id;
							StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
						}
					}
				}
			}
		}
	}
}



#endif

}




/*
//旧的InitImageShow函数
void MainWindow::InitImageShow(CollidingRectItem* item)
{
	int loop_times, delay_time;
	QSize img_size;					//
	//	int iHeartTime = m_HeartCount - item->begin_time;
	int iHeartTime;

	QList<QList<int>> show_teams = item->m_show_teams;	//当前表演编队
	QImage last_image;
	QSize sendimagesize;
	int sendimagelooptimes;
	int sendimage_perialtime;			//苹果图片的一次掉落时间
	if (m_CtrlModeFlag == 0)
	{	
		//不是收到了ipad发送的图片，正常表演
		iHeartTime = m_PlayTimeCount - item->begin_time;	//
		loop_times = item->m_loop_time;
		delay_time = item->m_delay_time;
		img_size = item->new_image.size();
		last_image = item->new_image;
	}
	else
	{
		//收到了ipad发送的图片，ipad互动
		//		if(CurrentJobID_!=-1)
		//		{
		//			qDebug()<<"prepare to end the mission!!!!!!!!!!!!!!!!!!!!!!!!!!";
		//			m_CtrlModeFlag=0;
		//			StatusToHTTP(CurrentJobID_,3);
		//			CurrentJobID_   =   -1;
		//			//之前有正在表演的手机图片，需要先结束掉
		//		}
		iHeartTime = m_PlayTimeCount - ipadimage_currtenttime;		//ipadimage_currtenttime是获得ipad图片的时刻
		//		sendimagelooptimes = 5;
		//		delay_time = 10;
		sendimagelooptimes = StaticValue::GetInstance()->str_ipaimageTimes.toInt();		//掉落的次数
		delay_time = StaticValue::GetInstance()->str_ipaimageDelayTime.toInt();			//间隔时间
		last_image.loadFromData(last_image_array);						//获得图片数据
		last_image = last_image.scaledToWidth(show_teams.size());		//缩放图片，宽度适配编组数量

		sendimagesize = last_image.size();			//适配尺寸后图片的大小，应该是包含了宽度和高度信息？
	//	img_size = last_image.size();
		img_size = sendimagesize;					//和非ipad的普通图片一致起来，普通图片的长宽等尺寸信息也在img_size变量内
		sendimage_perialtime = delay_time*(sendimagesize.height() - 1);	//ipad图片的一次掉落的所用时间
	}

	//以下操作普通图片和ipad图片是一样的
	int perial_time = delay_time*(img_size.height() - 1);	//一个完整图片掉落的时长

	int row_num = (iHeartTime%perial_time) / delay_time;	//图片的当前行
	int current_padimage_looptime = iHeartTime / (delay_time*(sendimagesize.height() - 1));		//ipad图片周期，已经掉落的次数
	int current_loop_time = iHeartTime / (delay_time*(img_size.height() - 1));					//判断走了多少个周期
	if (m_CtrlModeFlag == 0)
	{	
		//普通普片的情况
		if (current_loop_time >= loop_times && loop_times != -1)
		{
			stopTimer();
			return;
		}
	}
	else
	{	
		//ipad的情况
		qDebug() << "ipad图片周期" << current_padimage_looptime << "判断走了多少个周期" << current_loop_time;
		if (current_padimage_looptime >= sendimagelooptimes && sendimagelooptimes != -1)
		{
			//远程控制图片播放完毕
			m_CtrlModeFlag = 0;			//清除ipad有图片的标志
			StatusToHTTP(CurrentJobID_, 3);
			CurrentJobID_ = -1;
			//	ipadimage_currtenttime=item->begin_time;
			//	stopTimer();
			return;
		}
	}

	int show_num = img_size.height() - row_num - 1;		//当前输出的图片的行号
	if (iHeartTime % delay_time <= TIME_RES)
	{
		//每timeout一次就执行到这里一次，这个if没有啥用，屏蔽了没啥变化，估计对比值大于TIME_RES才有用
		if (m_CtrlModeFlag == 1)
		{
			//	qDebug()<<"outer loop";
			int i = 100;
			++i;
			//	qDebug()<<img_size;
		}

		//当前一行内，宽度上的点循环
		for (int i = 0; i < img_size.width(); ++i)       //第二遍循环挂掉
		{
			//根据图片的像素点，找到自己编队内的设备号
		//	for(int j = 0; j < show_teams.size(); j++)
		//	{
			QList<int> device_list = show_teams.at(i);
			for (int k = 0; k < device_list.size(); ++k)	//device_list.size()的值是1，只对应一个像素点，可能多米诺跑动的会大于1
			{
				//找到i像素点对应的水阀ID
				int device_id = device_list.at(k);
				int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
			//	if(QColor(item->new_image.pixel(i,show_num)) == Qt::black)
				if (QColor(last_image.pixel(i, show_num)) == Qt::black)		//感觉这里的i应该是变量k
				{
					//图形像素点为黑，图形非空白处的意思
					if (item->m_IsYangWen)
					{	//阳文
						if (device_state <= 0)   //此动作在关闭时，才将其打开，并改变设备状态，如果处于打开状态，那么不处理
						{
							//anzs deviceInfo* device_info = new DeviceInfo();
							StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;			//设置对应的设备为开
							int output_device_id = StaticValue::GetInstance()->
								m_device_map[device_id]->output_device_id;
							StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;	//数据有变化

						}
					}
					else
					{	//阴文
						if (device_state > 0)
						{
							StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
							int output_device_id = StaticValue::GetInstance()->
								m_device_map[device_id]->output_device_id;
							StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
						}
					}
				}
			//	else if(QColor(item->new_image.pixel(i,show_num)) == Qt::white)
				else if (QColor(last_image.pixel(i, show_num)) == Qt::white)
				{	
					//图片像素点为白，空白处的意思
					if (item->m_IsYangWen)
					{	//阳文
						if (device_state > 0)
						{
							StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
							int output_device_id = StaticValue::GetInstance()->
								m_device_map[device_id]->output_device_id;
							StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
						}
					}
					else
					{	//阴文
						if (device_state <= 0)
						{
							StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;
							int output_device_id = StaticValue::GetInstance()->
								m_device_map[device_id]->output_device_id;
							StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
						}
					}
				}
			}
		}
	}
}
*/
/*
//新的InitImageShow函数，单像素延迟的版本，2025-12-30，有BUG
void MainWindow::InitImageShow(CollidingRectItem* item)
{
	int loop_times, delay_time;
	QSize img_size;					//
	//	int iHeartTime = m_HeartCount - item->begin_time;
	int iHeartTime;

	QList<QList<int>> show_teams = item->m_show_teams;	//当前表演编队
	QImage last_image;
	QSize sendimagesize;
	int sendimagelooptimes;
	int sendimage_perialtime;			//苹果图片的一次掉落时间

	if (m_CtrlModeFlag == 0)
	{
		//不是收到了ipad发送的图片，正常表演
		iHeartTime = m_PlayTimeCount - item->begin_time;	//
		loop_times = item->m_loop_time;
		delay_time = item->m_delay_time;
		img_size = item->new_image.size();
		last_image = item->new_image;
	}
	else
	{
		//收到了ipad发送的图片，ipad互动
		//		if(CurrentJobID_!=-1)
		//		{
		//			qDebug()<<"prepare to end the mission!!!!!!!!!!!!!!!!!!!!!!!!!!";
		//			m_CtrlModeFlag=0;
		//			StatusToHTTP(CurrentJobID_,3);
		//			CurrentJobID_   =   -1;
		//			//之前有正在表演的手机图片，需要先结束掉
		//		}
		iHeartTime = m_PlayTimeCount - ipadimage_currtenttime;		//ipadimage_currtenttime是获得ipad图片的时刻
		//		sendimagelooptimes = 5;
		//		delay_time = 10;
		sendimagelooptimes = StaticValue::GetInstance()->str_ipaimageTimes.toInt();		//掉落的次数
		delay_time = StaticValue::GetInstance()->str_ipaimageDelayTime.toInt();			//间隔时间
		last_image.loadFromData(last_image_array);						//获得图片数据
		last_image = last_image.scaledToWidth(show_teams.size());		//缩放图片，宽度适配编组数量

		sendimagesize = last_image.size();			//适配尺寸后图片的大小，应该是包含了宽度和高度信息？
		//	img_size = last_image.size();
		img_size = sendimagesize;					//和非ipad的普通图片一致起来，普通图片的长宽等尺寸信息也在img_size变量内
		sendimage_perialtime = delay_time*(sendimagesize.height() - 1);	//ipad图片的一次掉落的所用时间
	}

	//以下操作普通图片和ipad图片是一样的
  	int perial_time = delay_time*(img_size.height() - 1);	//一个完整图片掉落的时长

//	int row_num = (iHeartTime%perial_time) / delay_time;	//图片的当前行 
	int current_padimage_looptime = iHeartTime / (delay_time*(sendimagesize.height() - 1));		//ipad图片周期，已经掉落的次数
	int current_loop_time = iHeartTime / (delay_time*(img_size.height() - 1));					//判断走了多少个周期
	if (m_CtrlModeFlag == 0)
	{
		//普通普片的情况
		if (current_loop_time >= loop_times && loop_times != -1)
		{
			stopTimer();
			return;
		}
	}
	else
	{
		//ipad的情况
		qDebug() << "ipad图片周期" << current_padimage_looptime << "判断走了多少个周期" << current_loop_time;
		if (current_padimage_looptime >= sendimagelooptimes && sendimagelooptimes != -1)
		{
			//远程控制图片播放完毕
			m_CtrlModeFlag = 0;			//清除ipad有图片的标志
			StatusToHTTP(CurrentJobID_, 3);
			CurrentJobID_ = -1;
			//	ipadimage_currtenttime=item->begin_time;
			//	stopTimer();
			return;
		}
	}

//	int row_num = (iHeartTime%perial_time) / delay_time;	//图片的当前行
//	int show_num = img_size.height() - row_num - 1;		//当前输出的图片的行号

//	if (iHeartTime % delay_time <= TIME_RES)
//	{
		//每timeout一次就执行到这里一次，这个if没有啥用，屏蔽了没啥变化，估计对比值大于TIME_RES才有用
	//	if (m_CtrlModeFlag == 1)
	//	{
			//	qDebug()<<"outer loop";
	//		int i = 100;
	//		++i;
			//	qDebug()<<img_size;
	//	}

		//当前一行内，宽度上的点循环
		for (int i = 0; i < img_size.width(); ++i)       //第二遍循环挂掉
		{
			QList<int> device_list = show_teams.at(i);

			//根据图片的像素点，找到自己编队内的设备号
			for (int k = 0; k < device_list.size(); ++k)	//device_list.size()的值是1，只对应一个像素点，可能多米诺跑动的会大于1
			{
				//找到i像素点对应的水阀ID
				int device_id = device_list.at(k);		//设备号
				int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;	//输出值
				int PixelDelayInms = StaticValue::GetInstance()->m_device_map[device_id]->delayinms;	//延时值
				//anzs for test 为了清晰可见
				//PixelDelayInms *= 10;
				int CurrentPixelTime = iHeartTime - PixelDelayInms;
				if (CurrentPixelTime >= 0)
				{
					int row_num = (CurrentPixelTime % perial_time) / delay_time;	//图片的当前行
					int show_num = img_size.height() - row_num - 1;				//当前输出的图片的行号，不是从0行输出，是从最大行开始输出，图片最底部
					if (CurrentPixelTime % delay_time <= TIME_RES)
					{
						if (QColor(last_image.pixel(k, show_num)) == Qt::black)		//感觉这里的i应该是变量k
						{
							//图形像素点为黑，图形非空白处的意思
							if (item->m_IsYangWen)
							{	//阳文
								if (device_state <= 0)   //此动作在关闭时，才将其打开，并改变设备状态，如果处于打开状态，那么不处理
								{
									StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;			//设置对应的设备为开
									int output_device_id = StaticValue::GetInstance()->
										m_device_map[device_id]->output_device_id;
									StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;	//数据有变化
								}
							}
							else
							{	//阴文
								if (device_state > 0)
								{
									StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
									int output_device_id = StaticValue::GetInstance()->
										m_device_map[device_id]->output_device_id;
									StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
								}
							}
						}
						//	else if(QColor(item->new_image.pixel(i,show_num)) == Qt::white)
						else if (QColor(last_image.pixel(k, show_num)) == Qt::white)
						{
							//图片像素点为白，空白处的意思
							if (item->m_IsYangWen)
							{	//阳文
								if (device_state > 0)
								{
									StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
									int output_device_id = StaticValue::GetInstance()->
										m_device_map[device_id]->output_device_id;
									StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
								}
							}
							else
							{	//阴文
								if (device_state <= 0)
								{
									StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 255;
									int output_device_id = StaticValue::GetInstance()->
										m_device_map[device_id]->output_device_id;
									StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
								}
							}
						}
					}
					else
					{
					//	qDebug() << "i=" << i << ",iHeartTime=" << iHeartTime << ",PixelDelayInms=" << PixelDelayInms <<
					//				",CurrentPixelTime=" << CurrentPixelTime << ",show_num=" << show_num << " 0";
					}
				}
			}
		}
//	}
} 
*/
void MainWindow::InitDMXShow(CollidingRectItem* item)
{
	int iHeartTime = m_PlayTimeCount - item->begin_time;
    QList<QList<int>> show_teams = item->getShowTeamsList();
    QMap<int,RouteInfo*> routes = item->getRouteSetting();
    QMap<int,RouteInfo*>::iterator iter;
    for(iter = routes.begin();iter != routes.end();++iter)
    {
        RouteInfo* route_info = iter.value();
        //此通道为常开
        if( DMX512_ALWAYSON == route_info->type )
        {
            InitDMXOn(route_info,show_teams);
        }
        else if( DMX512_DOMINO == route_info->type )
        {
            InitDMXDomino(item,route_info,show_teams);
        }
        else if( DMX512_TRACK == route_info->type )
        {
            int track_start_delay = route_info->start_delay;
            iHeartTime = iHeartTime + track_start_delay;
            Orbit* orbit = StaticValue::GetInstance()->m_show_orbit[route_info->orbit_id];
            if(orbit)
            {
                int delay_time = route_info->delay_time;
                QMap<int,int> show_orbit =  orbit->show_orbit;

                int iCurrentTime = iHeartTime %1000;

                for(int j = 0; j < show_teams.size(); ++j)
                {
                    int playtime = iCurrentTime-j*delay_time%1000;      //1000代表表演周期，playtime表示在表演周期内的相对时间

                    if(playtime<0)
                    {
                        playtime+=1000;
                    }
                    for(QMap<int,int>::iterator iter = show_orbit.begin();iter != show_orbit.end()-1; ++iter)           //遍历轨迹中的所有控制点,取得相应的控制点区间，从而计算斜率
                    {
                        QMap<int,int>::iterator itr_next    =   iter+1;

                        if((iter.key()<=playtime)&&(playtime<=itr_next.key()))      //找到对应时间段
                        {
                            //计算斜率
                            qreal a = (iter.value()-itr_next.value())*1.0/(iter.key()-itr_next.key());

                            qreal b = ((iter.value()+itr_next.value())-a*(iter.key()+itr_next.key()))/2;

                            int newvalue    =   a*playtime+b;

                            QList<int> device_list = show_teams.at(j);

                            for(int k = 0; k < device_list.size(); ++k)
                            {
                                //获取设备号
                                int device_id = device_list.at(k);
                                int out_put_device_id = StaticValue::GetInstance()->
                                        m_device_map[device_id]->output_device_id;
                                OutPutDevice* out_put_Device = StaticValue::GetInstance()->
                                        m_output_device[out_put_device_id];

                                //通过设备号获取通道号列表
                                QList<int> route_id_list = StaticValue::GetInstance()->
                                        m_route_mapping.values(device_id);
                                std::sort(route_id_list.begin(),route_id_list.end());
                                //通道ID,保证通道下标编号不超出该设备所包含的最大通道数,对于部分DMX四通道灯，最后一个通道可能不会有任何变化
                                if (route_id_list.size()>route_info->id)
                                {
                                    int route_id = route_id_list.at(route_info->id);
									int route_state = out_put_Device->m_route_state[route_id];
									if (route_state != newvalue)
									{
										out_put_Device->bHasChange = true;
										out_put_Device->m_route_state[route_id] = newvalue;
									}
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        else if (DMX512_FROMTO==route_info->type)//FROM TO
        {
            int value0 = route_info->out_value1;
            int value1 = route_info->out_value2;
            int delay_time = route_info->delay_time;
            int change_speed = route_info->change_speed;
            InitDMXFromTo(route_info->id,value0,value1,delay_time,change_speed,show_teams,iHeartTime);
        }
        else if (DMX512_RAINBOW==route_info->type)
        {
            InitDMXRainbow(item);
            //qDebug() << tr("当前相对时间") << iHeartTime;
        }
    }
}

void MainWindow::InitDMXOn(RouteInfo* route_info,QList<QList<int>> show_teams)
{
	int index = route_info->id;						//编队号码
	int out_put_value = route_info->out_value1;

    for(int j = 0; j < show_teams.size(); ++j)
    {
        //设备列表
        QList<int> device_list = show_teams.at(j);
        for(int k = 0; k < device_list.size(); ++k)
        {
            //获取设备号
            int device_id = device_list.at(k);
            int out_put_device_id = StaticValue::GetInstance()->
                    m_device_map[device_id]->output_device_id;
            OutPutDevice* out_put_Device = StaticValue::GetInstance()->
                    m_output_device[out_put_device_id];

            //通过设备号获取通道号列表
            QList<int> route_id_list = StaticValue::GetInstance()->
                    m_route_mapping.values(device_id);
			std::sort(route_id_list.begin(),route_id_list.end());		//屏蔽此句区别不大
            //通道ID
            int route_id = route_id_list.at(index);
            int route_state = out_put_Device->m_route_state[route_id];
            if(route_state != out_put_value)
            {
                out_put_Device->bHasChange = true;
                out_put_Device->m_route_state[route_id] = out_put_value;
            }
        }
    }
}

void MainWindow::InitDMXRainbow(CollidingRectItem* item)
{
	int yyy;
	int LPoint,Interval;
	int Temp;
	int RunTimeDelay;	//延迟后的时间
	int m_intRunRed;
	int m_intRunGre;
	int m_intRunBlue;
	int m_intRunWhite;
	int ch0,ch1,ch2,ch3;

    ouputViewer::GetOPViewerInstance()->loopDMXstatus();

    QList<QList<int>> show_teams = item->getShowTeamsList();    //编队信息
    QMap<int,RouteInfo*> routes = item->getRouteSetting();      //通道信息
    if (routes.size()==0)
    {
        return;
    }
    if (show_teams.size()==0)
    {
        return;
    }
    //彩虹特效只需要取第一个通道即可，其他通道可以忽略
    QMap<int, RouteInfo*>::iterator iter = routes.begin();
    RouteInfo* route_info = iter.value();
	Temp = show_teams.size();
    for (int i=0;i<show_teams.size();i++)
    {
        QList<int> devic_list = show_teams.at(i);
		Temp = devic_list.size();
        for (int j=0;j<devic_list.size();j++)
        {
            //设备号
            int device_id = devic_list.at(j);
            int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
            OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];

            //通过设备号获取通道号列表
            QList<int>  route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
            //排序，防止通道序号出现错序的情况
            std::sort(route_id_list.begin(), route_id_list.end());
            if (route_id_list.size()>=3)//DMX灯的设备一般包括三个以上的通道
            {
/*				//吕高鹏原来的程序
				//实际处理灯光数据,HSV模型
                int hid =   route_id_list.at(0);
                int sid =   route_id_list.at(1);
                int vid =   route_id_list.at(2);
                int v = 255 * (m_HeartCount % route_info->circle)/route_info->circle;
				out_put_Device->m_route_state[hid] = curcolor.red();
				out_put_Device->m_route_state[sid] = color.green();
				out_put_Device->m_route_state[vid] = curcolor.blue();
                out_put_Device->bHasChange  =   true;
				*/

/*				//修改后可以四通道的前三通道有输出的程序
				//实际处理灯光数据,HSV模型
				int hid =   route_id_list.at(0);
				int sid =   route_id_list.at(1);
				int vid =   route_id_list.at(2);
				int v = 255 * (m_HeartCount % route_info->circle)/route_info->circle;

				out_put_Device->m_route_state[hid] = v;				//curcolor.red();
				out_put_Device->m_route_state[sid] =(v+85)%255;		//color.green();
				out_put_Device->m_route_state[vid] =(v+170)%255;	//curcolor.blue();
				out_put_Device->bHasChange  =   true;
				*/

				//仿测试程序色彩渐变的程序
				switch(route_id_list.size())
				{
				case 3:		//RGB灯
					LPoint = i;	//获得灯的指针
					Interval = (m_intRunInt/m_intRunSpeed)*m_intRunStep*LPoint;

					RunTimeDelay = TimeHaveRun + m_intRunInt*i;		//延迟后的时间
					RunTimeDelay = RunTimeDelay %	TimePeriod;		//变化的周期
					RunPoint = RunTimeDelay / m_intRunSpeed;		//渐变的位置

					Temp = RunPoint*m_intRunStep;
					if(Temp <= 255)
						m_intRunRed = Temp;
					else if(Temp <= 511)
						m_intRunRed = 511-Temp;

					Temp = (RunPoint*m_intRunStep + 170)%512;
					if(Temp <= 255)
						m_intRunGre = Temp;
					else if(Temp <= 511)
						m_intRunGre = 511-Temp;

					Temp = (RunPoint*m_intRunStep + 340)%512;
					if(Temp <= 255)
						m_intRunBlue = Temp;
					else if(Temp <= 511)
						m_intRunBlue = 511-Temp;

					ch0 =   route_id_list.at(0);	//灯通道0的DMX通道号码
					ch1 =   route_id_list.at(1);	//灯通道1的DMX通道号码
					ch2 =   route_id_list.at(2);	//灯通道2的DMX通道号码
					out_put_Device->m_route_state[ch0] = m_intRunRed;
					out_put_Device->m_route_state[ch1] = m_intRunGre;
					out_put_Device->m_route_state[ch2] = m_intRunBlue;
					out_put_Device->bHasChange  =   true;
					break;

				case 4:		//一般情况下的RGBW灯
					LPoint = i;	//获得灯的指针
					Interval = (m_intRunInt/m_intRunSpeed)*m_intRunStep*LPoint;

					RunTimeDelay = TimeHaveRun + m_intRunInt*i;	//延迟后的时间
					RunTimeDelay = RunTimeDelay %	TimePeriod;		//变化的周期
					RunPoint = RunTimeDelay / m_intRunSpeed;		//渐变的位置

					Temp = RunPoint*m_intRunStep;
					if(Temp <= 255)
						m_intRunRed = Temp;
					else if(Temp <= 511)
						m_intRunRed = 511-Temp;

					Temp = (RunPoint*m_intRunStep + 128)%512;
					if(Temp <= 255)
						m_intRunGre = Temp;
					else if(Temp <= 511)
						m_intRunGre = 511-Temp;

					Temp = (RunPoint*m_intRunStep + 256)%512;
					if(Temp <= 255)
						m_intRunBlue = Temp;
					else if(Temp <= 511)
						m_intRunBlue = 511-Temp;

					Temp = (RunPoint*m_intRunStep + 384)%512;
					if(Temp <= 255)
						m_intRunWhite = Temp;
					else if(Temp <= 511)
						m_intRunWhite = 511-Temp;

					ch0 =   route_id_list.at(0);	//灯通道0的DMX通道号码
					ch1 =   route_id_list.at(1);	//灯通道1的DMX通道号码
					ch2 =   route_id_list.at(2);	//灯通道2的DMX通道号码
					ch3 =   route_id_list.at(3);	//灯通道3的DMX通道号码
					out_put_Device->m_route_state[ch0] = m_intRunRed;
					out_put_Device->m_route_state[ch1] = m_intRunGre;
					out_put_Device->m_route_state[ch2] = m_intRunBlue;
					out_put_Device->m_route_state[ch3] = m_intRunWhite;
					out_put_Device->bHasChange  =   true;
					break;

			/*	case 4:		//嘉兴特殊的灯，WRGB
					LPoint = i;	//获得灯的指针
					Interval = (m_intRunInt/m_intRunSpeed)*m_intRunStep*LPoint;

					RunTimeDelay = TimeHaveRun + m_intRunInt*i;	//延迟后的时间
					RunTimeDelay = RunTimeDelay %	TimePeriod;		//变化的周期
					RunPoint = RunTimeDelay / m_intRunSpeed;		//渐变的位置

					Temp = RunPoint*m_intRunStep;
					if(Temp <= 255)
						m_intRunRed = Temp;
					else if(Temp <= 511)
						m_intRunRed = 511-Temp;

					Temp = (RunPoint*m_intRunStep + 170)%512;
					if(Temp <= 255)
						m_intRunGre = Temp;
					else if(Temp <= 511)
						m_intRunGre = 511-Temp;

					Temp = (RunPoint*m_intRunStep + 340)%512;
					if(Temp <= 255)
						m_intRunBlue = Temp;
					else if(Temp <= 511)
						m_intRunBlue = 511-Temp;

					ch0 =   route_id_list.at(0);	//灯通道0的DMX通道号码
					ch1 =   route_id_list.at(1);	//灯通道1的DMX通道号码
					ch2 =   route_id_list.at(2);	//灯通道2的DMX通道号码
					ch3 =   route_id_list.at(3);	//灯通道3的DMX通道号码
					out_put_Device->m_route_state[ch0] = 0;
					out_put_Device->m_route_state[ch1] = m_intRunRed;
					out_put_Device->m_route_state[ch2] = m_intRunGre;
					out_put_Device->m_route_state[ch3] = m_intRunBlue;
					out_put_Device->bHasChange  =   true;
					break; */
				}
            }
            
        }
    }
    
}

void MainWindow::InitDMXDomino(CollidingRectItem* item,
                               RouteInfo* route_info,QList<QList<int>> show_teams)
{
/*    //给定默认参数
    static qint16   len1    =   3;
    static qint16   len2    =   3;
    static qint8    c1      =   255;
    static qint8    c2      =   0;
    static qint32   speed   =   400;    //每300毫秒前进一步
    QByteArray PlayGround;                                  //所有变化都在其中完成，可以称作表演场地
    QByteArray subPlay(len1+len2,0);
    for(int i=0;i<len1+len2;)
    {
        for(int j=0;j<len1;j++,i++)
        {
            subPlay[i]  =   c1;
        }
        for(int k=0;k<len2;k++,i++)
        {
            subPlay[i]  =   c2;
        }
    }
    while (PlayGround.size()<=show_teams.size())            //通过字串不断向总串中复制
    {
        PlayGround.push_back(subPlay);
    }
    PlayGround.truncate(show_teams.size());                 //对于可能超出的范围，截断处理
    int RelativeTime    =   m_HeartCount    -   item->begin_time;   //相对表演时间
    int MoveStep        =   RelativeTime/speed;                     //计算移动步数
    MoveStep    %=      PlayGround.size();                          //步数取模，就是实际要移动的数量

//    if(RelativeTime<=item->end_time-item->begin_time)               //实现渐入效果
//    {
//        for(int i=0;i<PlayGround.size()-MoveStep;i++)
//        {
//            PlayGround[i]   =   0;
//        }
//    }

    //MoveStep表示掐头去尾的个数
    QByteArray toMove   =   PlayGround.left(MoveStep);              //掐头
    PlayGround  =   PlayGround.right(PlayGround.size()-MoveStep);   //去尾
    PlayGround.push_back(toMove);                                   //尾端插入

    int Period  =   floor(RelativeTime/(speed*PlayGround.size()));

    if(Period%2==0)         //进入的周期
    {
        for(int i=0;i<PlayGround.size()-MoveStep;i++)
        {
            PlayGround[i]   =   0;
        }
    }
    else if(Period%2==1)    //出去的周期
    {
        for(int i=PlayGround.size();i>PlayGround.size()-MoveStep;i--)
        {
            PlayGround[i]   =   0;
        }
    }
    //至此，数据处理完成，接下来将数据重映射、分配、赋值

    qDebug()<<PlayGround;

    for(int i=0; i<show_teams.size();i++)
    {
        QList<int> device_list = show_teams.at(i);
        for(int j = 0; j < device_list.size(); ++j)
        {
            //获取设备号
            int device_id = device_list.at(j);
            int out_put_device_id = StaticValue::GetInstance()->
                    m_device_map[device_id]->output_device_id;
            OutPutDevice* out_put_Device = StaticValue::GetInstance()->
                    m_output_device[out_put_device_id];
            //通过设备号获取通道号列表
            QList<int> route_id_list = StaticValue::GetInstance()->
                    m_route_mapping.values(device_id);
            std::sort(route_id_list.begin(),route_id_list.end());
            //通道ID
            int route_id = route_id_list.at(route_info->id);
            int route_state = out_put_Device->m_route_state[route_id];
            if(route_state != PlayGround[i])
            {
                out_put_Device->bHasChange = true;
                out_put_Device->m_route_state[route_id] = PlayGround[i];
            }
        }
    }
	*/

	int iHeartTime = m_PlayTimeCount - item->begin_time;
	show_teams = item->m_show_teams;	//获得本段编曲的编队号码

	int id = route_info->id;					//id号，RGBW=0-3
	int loop_times = route_info->loop_time;		//循环次数
	int delay_time = route_info->delay_time;	//延迟时间
	int keep_time = route_info->keep_time;		//保持时间
	int perial_time = route_info->perial_time;	//波浪周期时间

	int out_value1 = route_info->out_value1;	//输出值1
	int out_value2 = route_info->out_value2;	//输出值2
	int start_delay = route_info->start_delay;	//启动延迟

	int reverse_flag = route_info->reverse_flag;//反向，1：反向，0：正常
	int from_left = route_info->from_left;		//左起，1：左起，0：全铺

	int total_perial_time = delay_time*show_teams.size();		//周期时间为延迟时间乘以表演编队的组的个数

	if (0 == from_left)
		iHeartTime = iHeartTime + total_perial_time;

	iHeartTime = iHeartTime - start_delay;		//加上延迟项

	iHeartTime = (iHeartTime / 5) * 5;	//归一化时间
	int iTempTime = iHeartTime;			//暂存一下本段编曲运行了的时间长度
	int curr_point = 0;

	if (delay_time < 0)		//延迟时间不能为负

		return;

	int donoff_point;

	if (delay_time == 0)
	{	//延迟时间为0，全关全开运行
		//是不是要结束
		//屏蔽的这段压根执行不到，条件永远不成立
		/*	item->m_curr_loop_time = iHeartTime / perial_time;		//当前循环了多少个周期
		if (loop_times != -1 && item->m_curr_loop_time >= loop_times)	//无限循环和超出循环次数时
		{
		item->m_curr_loop_time = 0;

		stopTimer();	//清除输出数据并发送到板卡，将来改为仅更新数据，最后统一比对发送

		for (int i = 0; i < show_teams.size(); i++)
		{
		//并且把保持的标志清除
		item->keep_time_flag[i] = 0;
		item->keep_time[i] = 0;

		//把所有内部数据都清除
		QList<int> show_device_list = show_teams.at(i);
		for (int k = 0; k < show_device_list.size(); ++k)
		{	//对编队的一个组的所有设备进行操作，都无效
		int device_id = show_device_list.at(k);
		StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
		}
		}
		return;
		} */

		//计算是否要关闭
		for (int j = 0; j < show_teams.size(); j++)
		{
			//所有有效的分组计时
			if (1 == item->dmx_keep_time_flag[id][j])
				item->dmx_keep_time[id][j] = item->dmx_keep_time[id][j] + 5;

			//如果计时超过保持时间，就关闭
			if (item->dmx_keep_time[id][j] >= keep_time)
			{
				//如果超时就关闭
				QList<int> close_device_list = show_teams.at(j);
				for (int k = 0; k < close_device_list.size(); ++k)
				{	//对编队的一个组的所有设备进行操作，都无效
					int device_id = close_device_list.at(k);
					int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
					OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
					//通过设备号获取通道号列表
					QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
					std::sort(route_id_list.begin(), route_id_list.end());
					//通道ID
					int route_id = route_id_list.at(route_info->id);
					int route_state = out_put_Device->m_route_state[route_id];
					if (route_state != out_value2)
					{
						out_put_Device->bHasChange = true;
						out_put_Device->m_route_state[route_id] = out_value2;
					}
				}

				//超时的分组不再计时
				item->dmx_keep_time_flag[id][j] = 0;
				item->dmx_keep_time[id][j] = 0;
			}
		}

		//把所有有效的分组都输出
		for (int i = 0; i < show_teams.size(); ++i)
		{
			if (1 == item->dmx_keep_time_flag[id][i])
			{
				QList<int> show_device_list = show_teams.at(i);
				for (int k = 0; k < show_device_list.size(); ++k)
				{	//对编队的一个组的所有设备进行操作，都无效
					int device_id = show_device_list.at(k);
					int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
					OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
					//通过设备号获取通道号列表
					QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
					std::sort(route_id_list.begin(), route_id_list.end());
					//通道ID
					int route_id = route_id_list.at(route_info->id);
					int route_state = out_put_Device->m_route_state[route_id];
					if (route_state != out_value1)
					{
						out_put_Device->bHasChange = true;
						out_put_Device->m_route_state[route_id] = out_value1;
					}
				}
			}
		}

		//延迟时间是打开新组的时间
		//只在等于延迟时间的时候新组才会有效
		int time_temp_set = iHeartTime % perial_time;
		if (time_temp_set == 0)
		{
			//所有的组都有效
			for (int i = 0; i < show_teams.size(); ++i)
			{
				item->dmx_keep_time[id][i] = 0;
				item->dmx_keep_time_flag[id][i] = 1;

				QList<int> show_device_list = show_teams.at(i);
				for (int k = 0; k < show_device_list.size(); ++k)
				{	//对编队的一个组的所有设备进行操作，都无效
					int device_id = show_device_list.at(k);
					int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
					OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
					//通过设备号获取通道号列表
					QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
					std::sort(route_id_list.begin(), route_id_list.end());
					//通道ID
					int route_id = route_id_list.at(route_info->id);
					int route_state = out_put_Device->m_route_state[route_id];
					if (route_state != out_value1)
					{
						out_put_Device->bHasChange = true;
						out_put_Device->m_route_state[route_id] = out_value1;
					}
				}
			}
		}
	}
	else
	{
		if (total_perial_time <= perial_time)	//如果理论周期时间等于实际设置的周期时间
		{
			//是不是要结束
			/*	item->m_curr_loop_time = iHeartTime / perial_time;		//当前循环了多少个周期
			if (loop_times != -1 && item->m_curr_loop_time >= loop_times)	//无限循环和超出循环次数时
			{
			item->m_curr_loop_time = 0;

			stopTimer();	//清除输出数据并发送到板卡，将来改为仅更新数据，最后统一比对发送

			for (int i = 0; i < show_teams.size(); i++)
			{
			//并且把保持的标志清除
			item->keep_time_flag[i] = 0;
			item->keep_time[i] = 0;

			//把所有内部数据都清除
			QList<int> show_device_list = show_teams.at(i);
			for (int k = 0; k < show_device_list.size(); ++k)
			{	//对编队的一个组的所有设备进行操作，都无效
			int device_id = show_device_list.at(k);
			StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
			}
			}
			return;
			} */

			//计算是否要关闭
			for (int j = 0; j < show_teams.size(); j++)
			{
				if (reverse_flag)
					curr_point = show_teams.size() - j -1;
				else
					curr_point = j;

				if (curr_point >= 0)
				{
					//所有有效的分组计时
					if (1 == item->dmx_keep_time_flag[id][curr_point])
						item->dmx_keep_time[id][curr_point] = item->dmx_keep_time[id][curr_point] + 5;

					//如果计时超过保持时间，就关闭
					if (item->dmx_keep_time[id][curr_point] >= keep_time)
					{
						//如果超时就关闭
						QList<int> close_device_list = show_teams.at(curr_point);
						for (int k = 0; k < close_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = close_device_list.at(k);
							int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
							OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
							//通过设备号获取通道号列表
							QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
							std::sort(route_id_list.begin(), route_id_list.end());
							//通道ID
							int route_id = route_id_list.at(route_info->id);
							int route_state = out_put_Device->m_route_state[route_id];
							if (route_state != out_value2)
							{
								out_put_Device->bHasChange = true;
								out_put_Device->m_route_state[route_id] = out_value2;
							}
						}

						//超时的分组不再计时
						item->dmx_keep_time_flag[id][curr_point] = 0;
						item->dmx_keep_time[id][curr_point] = 0;
					}
				}
			}

			//把所有有效的分组都输出
			for (int i = 0; i < show_teams.size(); ++i)
			{
				if (reverse_flag)
					curr_point = show_teams.size() - i - 1;
				else
					curr_point = i;

				if (curr_point >= 0)
				{
					if (1 == item->dmx_keep_time_flag[id][curr_point])
					{
						QList<int> show_device_list = show_teams.at(curr_point);
						for (int k = 0; k < show_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = show_device_list.at(k);
							int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
							OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
							//通过设备号获取通道号列表
							QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
							std::sort(route_id_list.begin(), route_id_list.end());
							//通道ID
							int route_id = route_id_list.at(route_info->id);
							int route_state = out_put_Device->m_route_state[route_id];
							if (route_state != out_value1)
							{
								out_put_Device->bHasChange = true;
								out_put_Device->m_route_state[route_id] = out_value1;
							}
						}
					}
				}
			}

			//延迟时间是打开新组的时间
			//只在等于延迟时间的时候新组才会有效
			int time_temp_set = iHeartTime % perial_time;
			if ((time_temp_set%delay_time) == 0)
			{
				//计算那个分组处于有效期
				int donoff_point = time_temp_set / delay_time;
				if (reverse_flag)
					curr_point = show_teams.size() - donoff_point - 1;
				else
					curr_point = donoff_point;

				if (curr_point >= 0)
				{
					if ((item->dmx_keep_time_flag[id][curr_point] == 0) && (curr_point < show_teams.size()))
					{
						item->dmx_keep_time[id][curr_point] = 0;
						item->dmx_keep_time_flag[id][curr_point] = 1;

						//让这个分组有效，并输出
						QList<int> show_device_list = show_teams.at(curr_point);
						for (int k = 0; k < show_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = show_device_list.at(k);
							int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
							OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
							//通过设备号获取通道号列表
							QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
							std::sort(route_id_list.begin(), route_id_list.end());
							//通道ID
							int route_id = route_id_list.at(route_info->id);
							int route_state = out_put_Device->m_route_state[route_id];
							if (route_state != out_value1)
							{
								out_put_Device->bHasChange = true;
								out_put_Device->m_route_state[route_id] = out_value1;
							}
						}
					}
				}
			}
		}
		else if (total_perial_time > perial_time)	//如果理论周期时间大于实际设置的周期时间（追逐）
		{
			//是不是要结束
			/*	item->m_curr_loop_time = iHeartTime / total_perial_time;		//当前循环了多少个周期
			if (loop_times != -1 && item->m_curr_loop_time >= loop_times)	//无限循环和超出循环次数时
			{
			item->m_curr_loop_time = 0;

			stopTimer();	//清除输出数据并发送到板卡，将来改为仅更新数据，最后统一比对发送

			for (int i = 0; i < show_teams.size(); i++)
			{
			//并且把保持的标志清除
			item->keep_time_flag[i] = 0;
			item->keep_time[i] = 0;

			//把所有内部数据都清除
			QList<int> show_device_list = show_teams.at(i);
			for (int k = 0; k < show_device_list.size(); ++k)
			{	//对编队的一个组的所有设备进行操作，都无效
			int device_id = show_device_list.at(k);
			StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
			}
			}
			return;
			} */

			//计算是否要关闭
			for (int j = 0; j < show_teams.size(); j++)
			{
				if (reverse_flag)
					curr_point = show_teams.size() - j - 1;
				else
					curr_point = j;

				if (curr_point >= 0)
				{
					//所有有效的分组计时
					if (1 == item->dmx_keep_time_flag[id][curr_point])
						item->dmx_keep_time[id][curr_point] = item->dmx_keep_time[id][curr_point] + 5;

					//如果计时超过保持时间，就关闭
					if (item->dmx_keep_time[id][curr_point] >= keep_time)
					{
						//如果超时就关闭
						QList<int> close_device_list = show_teams.at(curr_point);
						for (int k = 0; k < close_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = close_device_list.at(k);
							int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
							OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
							//通过设备号获取通道号列表
							QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
							std::sort(route_id_list.begin(), route_id_list.end());
							//通道ID
							int route_id = route_id_list.at(route_info->id);
							int route_state = out_put_Device->m_route_state[route_id];
							if (route_state != out_value2)
							{
								out_put_Device->bHasChange = true;
								out_put_Device->m_route_state[route_id] = out_value2;
							}
						}

						//超时的分组不再计时
						item->dmx_keep_time_flag[id][curr_point] = 0;
						item->dmx_keep_time[id][curr_point] = 0;
					}
				}
			}

			//把所有有效的分组都输出
			for (int i = 0; i < show_teams.size(); ++i)
			{
				if (reverse_flag)
					curr_point = show_teams.size() - i - 1;
				else
					curr_point = i;

				if (curr_point >= 0)
				{
					if (1 == item->dmx_keep_time_flag[id][curr_point])
					{
						QList<int> show_device_list = show_teams.at(curr_point);
						for (int k = 0; k < show_device_list.size(); ++k)
						{	//对编队的一个组的所有设备进行操作，都无效
							int device_id = show_device_list.at(k);
							/*	int device_state = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
								if (device_state != out_value1)   //正在表演时才会考虑将其关闭，如果已经被关闭，那么就不处理
								{
								int output_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
								StaticValue::GetInstance()->m_output_device[output_device_id]->bHasChange = true;
								StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = out_value1;
								} */
							int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
							OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
							//通过设备号获取通道号列表
							QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
							std::sort(route_id_list.begin(), route_id_list.end());
							//通道ID
							int route_id = route_id_list.at(route_info->id);
							int route_state = out_put_Device->m_route_state[route_id];
							if (route_state != out_value1)
							{
								out_put_Device->bHasChange = true;
								out_put_Device->m_route_state[route_id] = out_value1;
							}
						}
					}
				}
			}

			//		int time_temp_set = iHeartTime % perial_time; //是否到了设置的周期时间
			if ((iHeartTime%delay_time) == 0)
			{	//-1循环的情况
				if (loop_times == -1)
				{
					int ttt = 0;
					do{
					//	iTempTime = (iHeartTime % (perial_time*total_perial_time)) - ttt*perial_time;
					//	iTempTime = (iHeartTime % total_perial_time) - ttt*perial_time;

						iTempTime = (iHeartTime % perial_time) + ttt*perial_time;

						int donoff_point = iTempTime / delay_time;
						if (reverse_flag)
							curr_point = show_teams.size() - donoff_point - 1;
						else
							curr_point = donoff_point;

						if (curr_point >= 0)
						{
							if ((item->dmx_keep_time_flag[id][curr_point] == 0) && (curr_point < show_teams.size()))
							{
								item->dmx_keep_time[id][curr_point] = 0;
								item->dmx_keep_time_flag[id][curr_point] = 1;

								//让这个分组有效，并输出
								QList<int> show_device_list = show_teams.at(curr_point);
								for (int k = 0; k < show_device_list.size(); ++k)
								{	//对编队的一个组的所有设备进行操作，都无效
									int device_id = show_device_list.at(k);
									int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
									OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
									//通过设备号获取通道号列表
									QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
									std::sort(route_id_list.begin(), route_id_list.end());
									//通道ID
									int route_id = route_id_list.at(route_info->id);
									int route_state = out_put_Device->m_route_state[route_id];
									if (route_state != out_value1)
									{
										out_put_Device->bHasChange = true;
										out_put_Device->m_route_state[route_id] = out_value1;
									}
								}
							}
						}
						ttt++;
				//	} while (iTempTime >= 0);
					} while (iTempTime <= (total_perial_time + perial_time));
				}

				//不是无限循环
				if (loop_times != -1)
				{
					int ttt = 0;
					do{
						//	iTempTime = (iHeartTime % (perial_time*total_perial_time)) - ttt*perial_time;
						iTempTime = iHeartTime - ttt*perial_time;

						if (iTempTime >= 0)
						{
							int donoff_point = iTempTime / delay_time;
							if (reverse_flag)
								curr_point = show_teams.size() - donoff_point - 1;
							else
								curr_point = donoff_point;

							if (curr_point >= 0)
							{
								if ((item->dmx_keep_time_flag[id][curr_point] == 0) && (curr_point < show_teams.size()))
								{
									item->dmx_keep_time[id][curr_point] = 0;
									item->dmx_keep_time_flag[id][curr_point] = 1;

									//让这个分组有效，并输出
									QList<int> show_device_list = show_teams.at(curr_point);
									for (int k = 0; k < show_device_list.size(); ++k)
									{	//对编队的一个组的所有设备进行操作，都无效
										int device_id = show_device_list.at(k);
										int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
										OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
										//通过设备号获取通道号列表
										QList<int> route_id_list = StaticValue::GetInstance()->m_route_mapping.values(device_id);
										std::sort(route_id_list.begin(), route_id_list.end());
										//通道ID
										int route_id = route_id_list.at(route_info->id);
										int route_state = out_put_Device->m_route_state[route_id];
										if (route_state != out_value1)
										{
											out_put_Device->bHasChange = true;
											out_put_Device->m_route_state[route_id] = out_value1;
										}
									}
								}
							}
						}
						ttt++;
					} while ((iTempTime >= 0) && (ttt < loop_times));
				}
			}
		}
	}

//	//输出处理
////	if(1==Change)
////	{	//有变化才处理
//		//关闭设备
//		if(item->close_team_id != -1)
//		{
//            QList<int> close_device_list = show_teams.at(item->close_team_id);          //取得设备编号
//			for(int j = 0; j < close_device_list.size(); ++j)
//			{
//				//获取设备号
//                int device_id = close_device_list.at(j);                                //实际设备编号(设备编号全局唯一)
//                int out_put_device_id = StaticValue::GetInstance()->                    //控制版编号
//						m_device_map[device_id]->output_device_id;
//                QMap<int,DeviceInfo*> DevInfoCopy   =   StaticValue::GetInstance()->m_device_map;
//                OutPutDevice* out_put_Device = StaticValue::GetInstance()->             //控制板
//						m_output_device[out_put_device_id];
//				//通过设备号获取通道号列表
//				QList<int> route_id_list = StaticValue::GetInstance()->
//						m_route_mapping.values(device_id);
//				qSort(route_id_list.begin(),route_id_list.end());
//				//通道ID
//				int route_id = route_id_list.at(route_info->id);
//				int route_state = out_put_Device->m_route_state[route_id];
//				if(route_state != route_info->out_value2)
//				{
//					out_put_Device->bHasChange = true;
//					out_put_Device->m_route_state[route_id] = route_info->out_value2;
//				}
//			}
//		}

//		//全部清除
//		if(item->show_team_id == 0)
//		{
//			//新的开始，全部清除
//			for(int i=0; i<show_teams.size();i++)
//			{
//				QList<int> close_device_list = show_teams.at(i);
//				for(int j = 0; j < close_device_list.size(); ++j)
//				{
//					//获取设备号
//					int device_id = close_device_list.at(j);
//					int out_put_device_id = StaticValue::GetInstance()->
//							m_device_map[device_id]->output_device_id;
//					OutPutDevice* out_put_Device = StaticValue::GetInstance()->
//							m_output_device[out_put_device_id];
//					//通过设备号获取通道号列表
//					QList<int> route_id_list = StaticValue::GetInstance()->
//							m_route_mapping.values(device_id);
//					qSort(route_id_list.begin(),route_id_list.end());
//					//通道ID
//					int route_id = route_id_list.at(route_info->id);
//					int route_state = out_put_Device->m_route_state[route_id];
//					if(route_state != 0)
//					{
//						out_put_Device->bHasChange = true;
//						out_put_Device->m_route_state[route_id] = 0;
//					}
//				}
//			}
//		}

//		//编队起始的时候要关闭该编队的所有设备

//		//打开设备
//		if(item->show_team_id < show_teams.size() &&
//				item->show_team_id != item->close_team_id)
//		{
//			QList<int> show_device_list = show_teams.at(item->show_team_id);
//			for(int j = 0; j < show_device_list.size(); ++j)
//			{
//				//获取设备号
//				int device_id = show_device_list.at(j);
//				int out_put_device_id = StaticValue::GetInstance()->
//						m_device_map[device_id]->output_device_id;
//				OutPutDevice* out_put_Device = StaticValue::GetInstance()->
//						m_output_device[out_put_device_id];

//				//通过设备号获取通道号列表
//				QList<int> route_id_list = StaticValue::GetInstance()->
//						m_route_mapping.values(device_id);
//				qSort(route_id_list.begin(),route_id_list.end());
//				//通道ID
//				int route_id = route_id_list.at(route_info->id);
//				int route_state = out_put_Device->m_route_state[route_id];
//				if(route_state != route_info->out_value1)
//				{
//					out_put_Device->bHasChange = true;
//					out_put_Device->m_route_state[route_id] = route_info->out_value1;
//				}
//	//		}
//		}
//	}
}

void MainWindow::InitDMXFromTo(int index,int value1,int value2,int iDelayTime,
                               int change_speed,QList<QList<int>> show_teams,int iHeartTime)
{
    static int lastValue = 0;
    float blank = value1 > value2 ?(value1 - value2):(value2 - value1);
    //每个timer周期，通道应该变化的Num值
    float every_change_num = blank / change_speed;

    for(int j = 0; j < show_teams.size(); ++j)
    {

        if(j*iDelayTime < iHeartTime)
        {
            //当前通道的当前值
            int iRouteValue = 0;
            if(iHeartTime >= j*iDelayTime)
            {
                iRouteValue = ((iHeartTime - j*iDelayTime)%change_speed)*every_change_num;
            }
            else
                iRouteValue = (iHeartTime%change_speed)*every_change_num;

            if(value1 > value2)
//                iRouteValue = value1 - iRouteValue;
                iRouteValue = value1;
//            else if(value1 == value2)
//                iRouteValue = value1;
            if(j==0)
            {
//                请在此处继续调试
                QString path= "./DMXoutput.txt";
                QFile file(path);
                if(!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append)){
                        file.close();
                    } else {
                        QTextStream out(&file); out << QString("%1\n").arg(iRouteValue);
                        file.close();
                    }
                if(abs(lastValue-iRouteValue)>=20)
                {
                    int o = 100;
                    int k = 200;
                    o*=k;
                }
                lastValue   =   iRouteValue;
            }

            //表演编队内设备列表
            QList<int> device_list = show_teams.at(j);
            for(int k = 0; k < device_list.size(); ++k)
            {
                //获取设备号
                int device_id = device_list.at(k);
                int out_put_device_id = StaticValue::GetInstance()->
                        m_device_map[device_id]->output_device_id;
                OutPutDevice* out_put_Device = StaticValue::GetInstance()->
                        m_output_device[out_put_device_id];

                //通过设备号获取通道号列表
                QList<int> route_id_list = StaticValue::GetInstance()->
                        m_route_mapping.values(device_id);
                std::sort(route_id_list.begin(),route_id_list.end());
                //通道ID
                int route_id = route_id_list.at(index);
                int route_state = out_put_Device->m_route_state[route_id];
                if(route_state != iRouteValue)
                {
                    out_put_Device->bHasChange = true;
                    out_put_Device->m_route_state[route_id] = iRouteValue;
                }
            }
        }
    }
}

void MainWindow::SendHeartPackage()
{
    //向所有的灯光设备发送心跳包
 //   QUdpSocket udp_socket;
    QMap<int,OutPutDevice*> out_put_device =
            StaticValue::GetInstance()->m_output_device;
    QMap<int,OutPutDevice*>::iterator iter = out_put_device.begin();
    for( ; iter != out_put_device.end() ; ++iter)
    {
        OutPutDevice* output_device = iter.value();
		if(	(output_device->type.compare("DMX512") == 0) ||		//电气设备
			(output_device->type.compare("CanMain") == 0) )		//电气设备
        {
            QString qStrIpAddress = output_device->ipAddress;
            int port = output_device->port;
            udp_socket.writeDatagram(m_HeartBuffer,4,
                                     QHostAddress(qStrIpAddress),port);
        }
    }
}

void MainWindow::LoopDmx512Device()
{
	QMap<int,OutPutDevice*> out_put_device_list =
			StaticValue::GetInstance()->m_output_device;
	int k = out_put_device_list.size();
	QMap<int,OutPutDevice*>::iterator iter = out_put_device_list.begin();
	for(;iter != out_put_device_list.end();++iter)
	{
		OutPutDevice* out_put_device = iter.value();
		if(out_put_device->bHasChange && out_put_device->type.compare("DMX512") == 0)
		{
			QString qStrIpAddress = out_put_device->ipAddress;
			int port = out_put_device->port;
			QMap<int,int> dmx_state = out_put_device->m_route_state;
			QMap<int,int>::iterator dmx_iter = dmx_state.begin();
			int length = dmx_state.size();
			int minRouteID = dmx_iter.key();

			if (length < 960)
			{
				m_DmxBuffer[0] = 'D';
				m_DmxBuffer[1] = 'X';
				m_DmxBuffer[2] = 0x44;

				//data_ppp是有数据的起始通道
				m_DmxBuffer[3] = (minRouteID & 0x0300) >> 8;	//data_ppp MSB，0到511，最大511
				m_DmxBuffer[4] = (minRouteID & 0x00ff);		//data_ppp LSB

				//data_len 是有数据的通道长度
				//e.g.从4通道开始到11通道有输出，则起始通道是4							//长度是8
				m_DmxBuffer[5] = (length & 0x0F00) >> 8;	//data_len MSB，1到1920，最大1920
				m_DmxBuffer[6] = (length & 0x00ff);		//data_len LSB

				for (int i = 0; i < length; i++, dmx_iter++)	//数据
					m_DmxBuffer[7 + i] = dmx_iter.value();

				unsigned char t = 0;
				for (int i = 0; i < (length + 7); i++)  	//和校验
					t = t + m_DmxBuffer[i];
				m_DmxBuffer[length + 7] = t;

				udp_socket.writeDatagram(m_DmxBuffer, length + 8,
					QHostAddress(qStrIpAddress), port);
			}
			else
			{
				//发送小于960的部分
				m_DmxBuffer[0] = 'D';
				m_DmxBuffer[1] = 'X';
				m_DmxBuffer[2] = 0x44;

				//data_ppp是有数据的起始通道
				m_DmxBuffer[3] = (minRouteID & 0x0300) >> 8;	//data_ppp MSB，0到511，最大511
				m_DmxBuffer[4] = (minRouteID & 0x00ff);		//data_ppp LSB

				//data_len 是有数据的通道长度
				//e.g.从4通道开始到11通道有输出，则起始通道是4							//长度是8
				m_DmxBuffer[5] = (960 & 0x0F00) >> 8;	//data_len MSB，1到1920，最大1920
				m_DmxBuffer[6] = (960 & 0x00ff);		//data_len LSB

				for (int i = 0; i < 960; i++, dmx_iter++)	//数据
					m_DmxBuffer[7 + i] = dmx_iter.value();

				unsigned char t = 0;
				for (int i = 0; i < (960 + 7); i++)  	//和校验
					t = t + m_DmxBuffer[i];
				m_DmxBuffer[960 + 7] = t;

				udp_socket.writeDatagram(m_DmxBuffer, 960 + 8,
					QHostAddress(qStrIpAddress), port);

				//发送大于960的部分
				m_DmxBuffer[0] = 'D';
				m_DmxBuffer[1] = 'X';
				m_DmxBuffer[2] = 0x44;

				//data_ppp是有数据的起始通道
				m_DmxBuffer[3] = ((minRouteID+960) & 0x0F00) >> 8;	//data_ppp MSB，0到511，最大511
				m_DmxBuffer[4] = ((minRouteID+960) & 0x00ff);		//data_ppp LSB

				//data_len 是有数据的通道长度
				//e.g.从4通道开始到11通道有输出，则起始通道是4							//长度是8
				m_DmxBuffer[5] = ((length-960) & 0x0F00) >> 8;	//data_len MSB，1到1920，最大1920
				m_DmxBuffer[6] = ((length-960) & 0x00ff);		//data_len LSB

				for (int i = 0; i < (length - 960); i++, dmx_iter++)	//数据
					m_DmxBuffer[7 + i] = dmx_iter.value();

				t = 0;
				for (int i = 0; i < (length-960 + 7); i++)  	//和校验
					t = t + m_DmxBuffer[i];
				m_DmxBuffer[length-960 + 7] = t;

				udp_socket.writeDatagram(m_DmxBuffer, length-960 + 8,
					QHostAddress(qStrIpAddress), port);
			}
			QByteArray tmparray(m_DmxBuffer);
			qDebug() << tmparray;

			//发送完改变的数据包，即将其设置为未改变状态，防止重复发相同的数据
			out_put_device->bHasChange = false;
		}
	}
}

void MainWindow::LoopDo64Device()
{
    int i,j;
    int temp;
    int ck;
    int ct=2;

//    QUdpSocket udp_socket;

    QMap<int,OutPutDevice*> out_put_device_list =
            StaticValue::GetInstance()->m_output_device;
    QMap<int,OutPutDevice*>::iterator iter = out_put_device_list.begin();

    for(;iter != out_put_device_list.end();++iter)
    {
        OutPutDevice* out_put_device = iter.value();
        if(out_put_device->bHasChange && out_put_device->type.compare("DO64Ethenet") == 0)
        {

            QString qStrIPAddress = out_put_device->ipAddress;
            int qStrPort = out_put_device->port;
            QMap<int,int> device_list = out_put_device->m_mapping;
            QList<int> keys = device_list.keys();
            memset(m_Buffer,0,20);                  //用0置位
            m_Buffer[0] = 'D';
            m_Buffer[1] = 'O';
            ct=2;

            int require_byte    =   out_put_device->m_mapping.size()/8-1;            //计算设备的控制报文所占用的字节数
            for(j = require_byte; j >= 0;j--)
            {
                temp=0;
                for(i = 7; i >= 0; i--)
                {
                    int device_id = device_list[keys.at(j*8+i)];
                    int istate = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;

                    ck =0x01 <<i;
                    if( 1 == istate)
                    {
                        temp = temp | ck;
                    }
                    if(device_id != 0)
                        m_elec_show->setItemState(device_id,istate);
                }

                m_Buffer[ct  ] = hex_asc((temp&0xf0)>>4);
                m_Buffer[ct+1] = hex_asc((temp&0x0f));
                ct = ct+2;
            }
            int check = 0;
            for(i=0;i<18;i++)
            {
                check = check+m_Buffer[i];
            }
            if((check&0xff)!=0x93)
                temp=0;

            m_Buffer[18] = hex_asc((check&0xf0)>>4);
            m_Buffer[19] = hex_asc((check&0x0f));

            udp_socket.writeDatagram(m_Buffer,20,
                                     QHostAddress(qStrIPAddress),qStrPort);
            out_put_device->bHasChange = false;

        }

    }

//    if(totalvalve==0)       //可能定时器的频率远大于场景本身变化的频率，所以在定时器运行中haschanged的几率比较小
//    {
//        return;
//    }

}

//启动播放，菜单按下
void MainWindow::on_start_triggered()
{
    //add by eagle
    int isnewTab = m_tabWidget->count();
	if (isnewTab == 1 || m_tabWidget->currentIndex() == 0){
		//delete by eagle for crash when tab can't create QMessageBox::information(nullptr, "提示", "当前活动页不是编曲页");
		return;
    }
    //
    m_loopTimer->start(TIME_RES);
	m_loopTimer->setTimerType(Qt::PreciseTimer);
    AOset_.enable_  =   true;               //强制启动变频水泵
    MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
    //如果当前编曲ID和上次编曲ID不同时，重新更新编曲内表演动作列表；
    if(current_widget->id != m_CurrentMusicID)
    {
        m_CurrentMusicID = current_widget->id;

        m_CurrentMusicActions.clear();
        m_CurrentMusicActions = current_widget->getMusicActions();
    }
    current_widget->start();
	m_PlayTimeCount = 0;			//循环播放重新开始的时候也会运行到这里，总巡行时间清零
    m_IsOutPutTimerOn = true;

	m_IsManuPlayFlag = true;
 //   current_widget->m_timeline->stateChanged(QTimeLine::Running);//,QTimeLine::QPrivateSignal);
//NotRunning 0   Paused 1   Running 2
 //   if(m_isloopMusic==1)
 //   {

 //       while(current_widget->m_timeline->state()==(QTimeLine::NotRunning))
//        {
 //                    on_stop_triggered();
 //                   Sleep(2000);
  //                  current_widget->m_timeline->setLoopCount(0);
   //                 MainWindow::on_start_triggered();
//        }
//    }


         //       current_widget->resetStartTime(0);

          //      current_widget->start();
          //      m_HeartCount = 0;
        //        m_IsOutPutTimerOn = true;
          //          for(int i=0;i<100000;i++)
           //         {
          //      if(current_widget->m_timeline->state()==(QTimeLine::NotRunning))
       //             if(current_widget->m_timeline->currentTime()==current_widget->m_timeline)
         //          { MainWindow::on_start_triggered();}
        //            }
       //         ResetStartTimer(0);
         //        current_widget->start();

 //                m_HeartCount = 0;
   //              m_IsOutPutTimerOn = true;

    //        }
     //   }
 //       else
 //       {

  //          current_widget->start();
 //           m_HeartCount = 0;
  //          m_IsOutPutTimerOn = true;
 //       }



}

//暂停键，菜单按下
void MainWindow::on_pause_triggered()
{
	int isnewTab = m_tabWidget->count();
	if (isnewTab == 1 || m_tabWidget->currentIndex() == 0){
		//delete by eagle for crash when tab can't  QMessageBox::information(nullptr, "提示", "当前活动页不是编曲页");
		return;
	}
    
	MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
	current_widget->resume();
	m_loopTimer->setTimerType(Qt::PreciseTimer);
    m_loopTimer->start(TIME_RES);	
    m_IsOutPutTimerOn = true;
}

//停止键，菜单按下
void MainWindow::on_stop_triggered()
{
	int isnewTab = m_tabWidget->count();
	if (isnewTab == 1 || m_tabWidget->currentIndex() == 0){
		//delete by eagle for crash when tab can't  QMessageBox::information(nullptr, "提示", "当前活动页不是编曲页");
		return;
	}
	MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
	current_widget->stop();			//停止时间线、停止音乐、清除m_IsPlay标志
    stopTimer();					//清空输出数据
    setvol(0);						//水泵变频器10V数据清零
    AOset_.enable_  =   false;
    m_IsOutPutTimerOn = false;

	m_IsManuPlayFlag = false;
}

//非停止按键按下的动作响应
void MainWindow::on_stop_progrem_body()
{
	int isnewTab = m_tabWidget->count();
	if (isnewTab == 1 || m_tabWidget->currentIndex() == 0){
		//delete by eagle for crash when tab can't  QMessageBox::information(nullptr, "提示", "当前活动页不是编曲页");
		return;
	}
	MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
	current_widget->stop();			//停止时间线、停止音乐、清除m_IsPlay标志
	stopTimer();					//清空输出数据
	setvol(0);						//水泵变频器10V数据清零
	AOset_.enable_ = false;
	m_IsOutPutTimerOn = false;
}

void MainWindow::on_action4test_triggered()
{
//    QMessageBox::information(this,"提示","测试菜单触发");
    AOsetting set;

    set.lowVoltage_ =   AOset_.lowvoltage_;

    set.HighVoltage_    =   AOset_.highvoltage_;

    set.ipaddr_     =   AOset_.ipaddr_;

    set.enable_     =   AOset_.enable_;

    set.resetsetting();

    if(set.exec()==QDialog::Accepted)
    {
        AOset_.lowvoltage_  =   set.lowVoltage_;

        AOset_.highvoltage_ =   set.HighVoltage_;

        AOset_.ipaddr_      =   set.ipaddr_;

        AOset_.enable_      =   set.enable_;

//        AOset_.enable_      =   set.e
    }
}
void MainWindow::on_pointertime_triggered()
{
//    MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
//    current_widget->pointertime();
//    QMessageBox::information(this,"test","hehe");
//    RequestDMX();
     
      ouputViewer* pOutPutViewer    =    ouputViewer::GetOPViewerInstance();

      pOutPutViewer->show();
}

void MainWindow::TimelineHasFinished()
{
    //    m_Timer->stop();
}

void MainWindow::CommonRecvCommand()
{

    if(rcstatus_.WorkingMode!=remotecontrol::Mode::COMMON)      //如果不是常规模式 不做任何处理
    {
        if(CommonSocket_->pendingDatagramSize())                //清空缓冲区
        {
            QByteArray datagram;
            QHostAddress sender;
            quint16 senderPort;
            datagram.resize(CommonSocket_->pendingDatagramSize());
            CommonSocket_->readDatagram(datagram.data(), datagram.size(),
                                    &sender, &senderPort);
        }
        return;
    }
    while (CommonSocket_->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(CommonSocket_->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        CommonSocket_->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        QString str(datagram);
        QString tipStr;
        if(str.startsWith("playmode"))         //设置循环模式
        {
            QString PlayMode =   str.right(1);  //截取表示循环模式的数字
            int PlayModeNum =   PlayMode.toInt();
//            m_playlist->setPlaybackMode(Mode);
        }

        else if(str.startsWith("playq"))        //查找以play开头的指令
        {
//            m_player->setCursor(Qt::BlankCursor);
            QMap<int,MusicInfo*>    MusicMap    =   StaticValue::GetInstance()->m_music_info;

            QString PlayNum =   str.right(str.size()-5);  //截取表示曲目的数字
            int  PlayIndex  =   PlayNum.toInt();
            bool res    =   MusicMap.contains(PlayIndex);
            if(res==false)
            {
               tipStr  =    tr("播放编曲失败！编曲%1不存在！").arg(PlayIndex);
            }
            else
            {
               tipStr =    tr("播放编曲%1").arg(PlayIndex);
			   on_stop_progrem_body();
               emit tree_view->leftDoubleClicked(TreeType::COMP_DEVICE,PlayIndex);
               on_start_triggered();
            }
        }
        else if(str.startsWith("stop"))
        {
			if (m_IsManuPlayFlag)
			{
				on_stop_progrem_body();

				QString tipStr = "停止播放";
			}
        }
        else if(str.startsWith("continue"))
        {
			if (m_IsManuPlayFlag)
			{
				on_pause_triggered();              //继续按钮
				QString tipStr = "继续播放";
			}
        }

        statusLabel_->setText(tipStr);
    }
}

void MainWindow::DMXRecvCommand()
{
    if(rcstatus_.WorkingMode!=remotecontrol::Mode::DMX)      //如果不是DMX模式 不做任何处理
    {
	//	QMessageBox::information(this,"warning","常规模式，不接收非DMX指令");
		if(DMXRequest_->pendingDatagramSize())               //清空缓冲区
        {
            QHostAddress sender;
            quint16 senderPort;
            QByteArray datagram;
            datagram.resize(DMXRequest_->pendingDatagramSize());
            DMXRequest_->readDatagram(datagram.data(), datagram.size(),
                                    &sender, &senderPort);
        }
        return;

    }
    while (DMXRequest_->hasPendingDatagrams())
    {
        static QString LastDMXcommand   =   "";          //上一条DMX指令，防止重复执行相同的命令
        QString tipStr;
        QByteArray datagram;
        datagram.resize(DMXRequest_->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        DMXRequest_->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        quint16 datasize    =   datagram.size();
        Q_ASSERT(datasize==rcstatus_.DataLength+10);      //报文头长度固定为10

        unsigned char c =   0;
        for(int i = 0;i<datasize-1;i++)                  //除去最后一位的数据校验
        {
            c   += datagram.at(i);
        }
        if(c!=datagram.at(datasize-1))                   //校验和检查失败！
        {
            tipStr  =   "校验和检查失败";
            statusLabel_->setText(tipStr);
            return;
        }
        if(datagram.at(4)!=0x01)                        //进一步的标志位检查
        {
            statusLabel_->setText("标志位不正确!");
            return;
        }


        QString str(datagram.right(datasize-9));       //刨除报文头，得到数据，并转化成命令字符

        //数据校验

        if(LastDMXcommand==str)                 //本次指令和上次的指令完全相同，不做任何处理，退出函数
        {
            return;
        }

        LastDMXcommand  =   str;                //保存当前新的指令

        if(str.startsWith("playmode"))         //设置循环模式
        {
            QString PlayMode =   str.right(1);  //截取表示循环模式的数字
            int PlayModeNum =   PlayMode.toInt();
//            m_playlist->setPlaybackMode(Mode);
        }

        else if(str.startsWith("playq"))        //查找以play开头的指令
        {
//            m_player->setCursor(Qt::BlankCursor);
            QMap<int,MusicInfo*>    MusicMap    =   StaticValue::GetInstance()->m_music_info;

            QString PlayNum =   str.right(str.size()-5);  //截取表示曲目的数字
            int  PlayIndex  =   PlayNum.toInt();
            bool res    =   MusicMap.contains(PlayIndex);
            if(res==false)
            {
               tipStr  =    tr("播放编曲失败！编曲%1不存在！").arg(PlayIndex);
            }
            else
            {
               tipStr =    tr("播放编曲%1").arg(PlayIndex);
			   on_stop_progrem_body();
               emit tree_view->leftDoubleClicked(TreeType::COMP_DEVICE,PlayIndex);
               on_start_triggered();
            }
        }
        else if(str.startsWith("stop"))
        {

			on_stop_progrem_body();
            tipStr  =   "停止播放";
        }
        else if(str.startsWith("continue"))
        {
            on_pause_triggered();              //继续按钮
            tipStr  =   "继续播放";
        }
        else
        {
            tipStr  =  str + "(无效指令)";
        }

        QTime   Curtime =   QTime::currentTime();
        tipStr+=Curtime.toString("h:m:s");
        statusLabel_->setText(tipStr);
    }
}

void MainWindow::AOtimerout()
{
    QMap<int,OutPutDevice*> out_put_device_list =
            StaticValue::GetInstance()->m_output_device;
    QMap<int,OutPutDevice*>::iterator iter = out_put_device_list.begin();

    int totalvalve  =   0;
    int workingcount    =   0;
    for(;iter != out_put_device_list.end();++iter)          //遍历所有输出设备
    {
        OutPutDevice* out_put_device = iter.value();

        if(out_put_device->type.compare("DO64Ethenet") == 0||out_put_device->type.compare("SL32") == 0)    //只查找DO64设备和SL32输出设备
        {
            totalvalve+=out_put_device->m_mapping.size(); //累加总水阀数量

            QMap<int,int> device_list = out_put_device->m_mapping;
            QList<int> keys = device_list.keys();
            for(int i=0;i<keys.size();i++)
            {
                int device_id = device_list[i];
                int istate = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;        //取得工作状态
                if(istate==1)                           //工作状态中的水阀
                {
                    workingcount++;
                }
            }
        }
    }

    qreal curvoltage ;

    if(totalvalve==0)           //除数不能为0！！！
    {
        curvoltage  =   AOset_.lowvoltage_;
    }
    else
    {
        curvoltage  =   workingcount*(AOset_.highvoltage_-AOset_.lowvoltage_)/totalvalve + AOset_.lowvoltage_;
    }


    int digvol  =   curvoltage*255/AOset_.maxvoltage_;

    QString statusstr = tr("当前工作的水阀:%1 总水阀：%2").arg(workingcount).arg(totalvalve);

    QString volstr      =   tr("水阀最低电压:%1 最高电压:%2 实时电压:%3 数字电压:%4").arg(AOset_.lowvoltage_).arg(AOset_.highvoltage_).arg(curvoltage).arg(digvol);

    mybar_->clearMessage();

    mybar_->showMessage(statusstr+volstr);

    if(AOset_.enable_)
    {
        setvol(digvol);     //发送电压控制报文
    }
}
void MainWindow::stopTimer()
{
    m_loopTimer->stop();
//    m_isloopMusic=0;
//    ui->loopsingle->setChecked(0);

    //将DO64输出设备置空
    QMap<int,OutPutDevice*> out_put_device = StaticValue::GetInstance()->m_output_device;
    QList<int> keys = out_put_device.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        OutPutDevice* output_device = out_put_device[keys.at(i)];
        QString type = output_device->type;
		if (type.compare("DO64Ethenet") == 0 || type.compare("SL32") == 0 || type.compare("SL40") == 0)
        {
            QMap<int,int> device_list = output_device->m_mapping;
            QList<int> device_keys = device_list.keys();
            for(int i = 0; i < device_keys.size();++i)
            {
                output_device->bHasChange = true;
                int device_id = device_list[device_keys.at(i)];
                StaticValue::GetInstance()->m_device_map[device_id]->out_put_value = 0;
            }
        }
    }

    //将DMX512设备置空
    QMap<int,OutPutDevice*> out_put_device_list =
            StaticValue::GetInstance()->m_output_device;
    QMap<int,OutPutDevice*>::iterator output_iter = out_put_device_list.begin();
    for(;output_iter != out_put_device_list.end();++output_iter)
    {
        OutPutDevice* out_put_device = output_iter.value();
        if(	(out_put_device->type.compare("DMX512") == 0)  ||
			(out_put_device->type.compare("CanMain") == 0) )
        {
            QMap<int,int> dmx_state = out_put_device->m_route_state;
            QMap<int,int>::iterator dmx_iter = dmx_state.begin();
            for(; dmx_iter != dmx_state.end(); ++dmx_iter)
            {
                out_put_device->bHasChange = true;
                out_put_device->m_route_state[dmx_iter.key()] = 0;
            }
        }
    }

    LoopDo64Device();		//组织DO64的数据包并输出
    LoopDmx512Device();		//长短DMX都是这个输出
    LoopSL32();             //组织SL32的数据包输出
	LoopSL40();             //组织SL32的数据包输出
	LoopCanMain();

    QMap<int,OutPutDevice*>::iterator iter = out_put_device_list.begin();
    for(;iter != out_put_device_list.end();++iter)
    {
        OutPutDevice* out_put_device = iter.value();
        if(out_put_device->type.compare("DMX512") == 0)
        {
            out_put_device->m_route_state.clear();
        }
    }
}


void MainWindow::on_open_triggered()
{
}

void MainWindow::on_save_triggered()
{
    if(m_tabWidget->currentIndex() != 0)
    {
        MusicWidget* music = (MusicWidget*)m_tabWidget->currentWidget();
        music->saveMusic();
        music->savecutlines();
    }
    SavetimingMission();        //保存定时任务信息
    SaveControlStatus();        //保存远程控制信息
}

void MainWindow::on_quit_triggered()
{
	QCloseEvent *closeEvent = new QCloseEvent();// File 退出，发送一个envent时间，清理各种资源
	this->closeEvent(closeEvent);
    exit(0);
}

//新建编曲
void MainWindow::on_new_music_triggered()
{
    QString fullPath =
            QFileDialog::getOpenFileName(this,
                                         tr("打开文件"),
                                         QDir::currentPath(),
                                         tr("音乐文件(*.mp3 *.wav *.flac)"));
    MusicInfo* music_info = new MusicInfo();
    music_info->type = NEW_MUSIC;
    music_info->name = QFileInfo(fullPath).baseName();
	music_info->music_path = fullPath; //add by eagle for path requirement need
    if(!fullPath.isEmpty())
    {
        QString qStrSql = QString("insert into music(type,name,music_path)"
                                  " values(%1,'%2','%3')").
                arg(NEW_MUSIC).arg(music_info->name).arg(fullPath);
        QSqlQuery query;
        if(!query.exec(qStrSql))
        {
            qDebug() <<"insert music false!sql:"<<qStrSql<<
                       " error:"<<query.lastError().text();
            return ;
        }
        qStrSql = QString("select distinct last_insert_rowid() from music");
        if(!query.exec(qStrSql))
        {
            qDebug() <<"select last_insert_rowid from music false!sql:"
                    <<qStrSql<<" error:"<<query.lastError().text();
            return ;
        }
        if(query.next())
        {
            music_info->id = query.value(0).toInt();
			int view_height = m_tabWidget->rect().height();
            int view_width = m_tabWidget->rect().width();
            
			//add by eagle for spectrum （新建逻辑的波形）
			// 创建新的 spectrumwidget
			spectrumwidget* newWidget = new spectrumwidget();
			int newWidgetHeight = 90;
			newWidget->setFixedSize(view_width, newWidgetHeight);
			newWidget->setLabel(tr("音乐波形"));
			// newWidget->setStyleSheet("background-color: gray;");
			MusicWidget* musicTab = new MusicWidget(m_tabWidget);		
			if (music_info->music_path.isEmpty() == false){ //这里重新校验一下音乐时长，可以动态获取
				music_info->time_length = musicTab->getMusicLengthbypath(music_info->music_path)/1000;
			}

			// 将 newWidget 添加到 musicTab 下方
			musicTab->addWidgetBelow(newWidget);

			connect(musicTab,SIGNAL(controler()),this,SLOT(ControlTimer()));
            connect(musicTab,SIGNAL(resetStartTime(int)),this,SLOT(ResetStartTimer(int)));
            musicTab->resize(QSize(view_width,view_height));
            musicTab->InitData(music_info);
            int index = m_tabWidget->addTab(musicTab,music_info->name);
            m_tabWidget->setCurrentIndex(index);
            StaticValue::GetInstance()->m_music_info[music_info->id] = music_info;
            tree_view->resetTreeView();
        }
    }
}

void MainWindow::ResetStartTimer(int start_time)
{
	m_PlayTimeCount = start_time;
}

void MainWindow::ControlTimer()
{
    MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
    if(m_IsOutPutTimerOn)
    {
        current_widget->stop();
		current_widget->pausetrack(true);
        stopTimer();
        m_IsOutPutTimerOn = false;
    }
    else
    {
        current_widget->resume();
		current_widget->pausetrack(false);
        m_IsOutPutTimerOn = true;
    }


}

//新建程控
void MainWindow::on_new_program_control_triggered()
{
    ProgramControl control_dialog;
    control_dialog.setModal(true);
    if(QDialog::Accepted == control_dialog.exec())
    {
        MusicInfo* music_info = new MusicInfo();
        music_info->type = NEW_PROCONTROL;							//type，类型：程控
        music_info->name = control_dialog.m_name;					//name，编曲名称
        music_info->time_length = control_dialog.m_timeLength;		//time_length，时间长度，单位：秒

        QString qStrSql = QString("insert into music(type,name,time_length)"
                                  " values(%1,'%2',%3)").arg(NEW_PROCONTROL).
                arg(control_dialog.m_name).arg(control_dialog.m_timeLength);
        QSqlQuery query;
        if(!query.exec(qStrSql))
        {
            qDebug() <<"insert music false!sql:"<<qStrSql<<
                       " error:"<<query.lastError().text();
            return ;
        }
        qStrSql = QString("select distinct last_insert_rowid() from music");
        if(!query.exec(qStrSql))
        {
            qDebug() <<"select last_insert_rowid from music false!sql:"
                    <<qStrSql<<" error:"<<query.lastError().text();
            return ;
        }
        if(query.next())
        {
            music_info->id = query.value(0).toInt();
            int view_height = m_tabWidget->rect().height();
            int view_width = m_tabWidget->rect().width();
            MusicWidget* musicTab = new MusicWidget(m_tabWidget);
            connect(musicTab,SIGNAL(controler()),this,SLOT(ControlTimer()));
            connect(musicTab,SIGNAL(resetStartTime(int)),this,SLOT(ResetStartTimer(int)));
            musicTab->resize(QSize(view_width,view_height));
            musicTab->InitData(music_info);
            int index = m_tabWidget->addTab(musicTab,music_info->name);
            m_tabWidget->setCurrentIndex(index);
            StaticValue::GetInstance()->m_music_info[music_info->id] = music_info;
            tree_view->resetTreeView();
        }
    }
}

void MainWindow::on_delete_comp_file(QString name)
{
    int iCount = m_tabWidget->count();
    for(int i = 0; i < iCount; ++i)
    {
        QString qStrText = m_tabWidget->tabText(i);
        if(qStrText.compare(name) == 0)
            m_tabWidget->removeTab(i);
    }
}

int MainWindow::getDeviceNumber(QString qStrType)
{
    int iCount = 0;
    QMap<int,OutPutDevice*> out_put_device_list =
            StaticValue::GetInstance()->m_output_device;
    QMap<int,OutPutDevice*>::iterator iter = out_put_device_list.begin();
    for(;iter != out_put_device_list.end();++iter)
    {
        OutPutDevice* out_put_device = iter.value();
        if(out_put_device->type.compare(qStrType) == 0)
        {
            iCount++;
        }
    }
    return iCount;
}

//生成DMX文件
void MainWindow::on_action_DMX_triggered()
{
    QString qStrFileName = QFileDialog::getSaveFileName(this, tr("保存文件"),
														"/home/user/sovotek", tr("DMX文件(*.dmx)"));
    QString qStrOldFileName = qStrFileName + "O";
    m_original_file.setFileName(qStrOldFileName);
    if(!m_original_file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("提示"),
                                 tr("创建并打开DMX原始文件失败！"), QMessageBox::Ok);
        return ;
    }
    m_decrypt_file.setFileName(qStrFileName);
    if(!m_decrypt_file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("提示"),
                                 tr("创建并打开加密的DMX文件失败！"), QMessageBox::Ok);
        return ;
    }

    m_DO64Num = getDeviceNumber("DO64Ethenet");
    m_SL32Num = getDeviceNumber("SL32");
	m_SL40Num = getDeviceNumber("SL40");
    m_DMX512Num = getDeviceNumber("DMX512");

//	int iSize = m_DO64Num*64 + m_SL32Num*32 + m_DMX512Num*512 + 1;
	int iSize = m_DO64Num * 64 + m_SL32Num * 32 + m_SL40Num * 40 + m_DMX512Num * 512 + 1;
    m_pDMX = new char[iSize];
    StaticValue::GetInstance()->ZeroMemoryEx(m_pDMX,iSize);

    m_dmx_state.clear();
    DmxHeartCount = 0;	//以10ms为单位
	m_PlayTimeCount = 0;
    m_intRecordSnNumAll = 0;		//总记录数(本次有变化的板子的数量)
    lOffset = 0;
    lActual = 0;		//这三个变量可能全工程都可以公用,因为是顺序执行的

    lOffset = DMX_INF_END;		//256
    m_original_file.seek(lOffset);
    m_decrypt_file.seek(lOffset);
    MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
    //如果当前编曲ID和上次编曲ID不同时，重新更新编曲内表演动作列表；
    if(current_widget->id != m_CurrentMusicID)
    {
        m_CurrentMusicID = current_widget->id;
        m_CurrentMusicActions.clear();
        m_CurrentMusicActions = current_widget->getMusicActions();
    }
    int music_length = current_widget->getTimeLength()*1000;
    m_MusicType = current_widget->getType();
    m_MusicName = current_widget->getMusicName();

    do{
        InitOutPutData(DMX_TYPE);     //初始化DMX的值

        GetDmxData();    //获取DMX的数据

        SaveDmxDataToFile();  //保存DMX的数据到文件中

        //每过一段时间发送一次打开运行的数据包，这一部分屏蔽了
        //每运行一秒
		if ((m_PlayTimeCount > 0) && (((int)m_PlayTimeCount % 1000) == 0))		//1秒1次心跳信号
            SaveDmxHeartData();

		m_PlayTimeCount += TIME_RES;

	} while (m_PlayTimeCount <= music_length);

    SaveDMXHeaderData();
	m_PlayTimeCount = 0;
    if(m_pDMX)
    {
        delete []m_pDMX;
        m_pDMX = NULL;
    }
    QMessageBox::information(this, tr("提示"),
                             tr("DMX文件生成成功!"), QMessageBox::Ok);
}

//计算MCON数据，地址按照顺序来，以后再加上首地址选项
void MainWindow::GetDmxData()
{
    unsigned char temp=0;
    int CardIndex = 0;
    QMap<int,OutPutDevice*> out_put_device_list =
            StaticValue::GetInstance()->m_output_device;
    QList<int> keys = out_put_device_list.keys();
    for(; CardIndex < keys.size(); ++CardIndex)
	{	//遍历所有的卡
        OutPutDevice* out_put_device = out_put_device_list.value(keys.at(CardIndex));
        //当前输出设备没有改变就遍历下一个设备
        if(!out_put_device->bHasChange)
            continue;

        //判断设备类型是DO64还是DMX512
		QString device_type = out_put_device->type;
/*        if(device_type.compare("DO64Ethenet") == 0)
        {
            QMap<int,int> device_list = out_put_device->m_mapping;
            QList<int> keys = device_list.keys();
            for(int j = 0; j < 8;j++)
            {
                temp=0;
                for(int i = 0; i < 8; i++)
                {
                    int device_id = device_list[keys.at(j*8+i)];
                    int istate = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
                    if( 1 == istate)
                    {
                        temp= temp | (0x01<<i);
                    }
                }
                m_pDMX[CardIndex*8+j]=temp;		//m从0开始???，这里可能有问题
            }
		} */
		if(device_type.compare("SL32") == 0)
        {
            QMap<int,int> device_list = out_put_device->m_mapping;
            QList<int> keys = device_list.keys();
			for(int j = 0; j < 4;j++)
            {
                temp=0;
				for(int i = 0; i < 8; i++)  //4个字节
                {
                    int device_id = device_list[keys.at(j*8+i)];
                    int istate = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
                    if( 1 == istate)
                    {
                        temp= temp | (0x01<<i);
                    }
                }
				m_pDMX[CardIndex*4+j]=temp;		//m从0开始
			}
        }
		else if (device_type.compare("SL40") == 0)
		{
			QMap<int, int> device_list = out_put_device->m_mapping;
			QList<int> keys = device_list.keys();
			for (int j = 0; j < 5; j++)
			{
				temp = 0;
				for (int i = 0; i < 8; i++)  //4个字节
				{
					int device_id = device_list[keys.at(j * 8 + i)];
					int istate = StaticValue::GetInstance()->m_device_map[device_id]->out_put_value;
					if (1 == istate)
					{
						temp = temp | (0x01 << i);
					}
				}
				m_pDMX[CardIndex * 5 + j] = temp;		//m从0开始
			}
		}
        else if(device_type.compare("DMX512") == 0)
        {
               //本来大括号里面都是屏蔽了的
             QMap<int,int> dmx_state = out_put_device->m_route_state;
             QMap<int,int>::iterator dmx_iter = dmx_state.begin();
             int length = dmx_state.size();
             for(int i = 0; i < length; i++ , dmx_iter++)	//数据
             {
				int route_id = dmx_iter.key();
			//	m_pDMX[m_DO64Num*8 + iDMX512Index*512 + route_id] = dmx_iter.value();
			//	m_pDMX[m_DO64Num*8 + m_DMX512Num*512 + route_id] = dmx_iter.value();
			//	m_pDMX[m_SL32Num*4 + route_id] = dmx_iter.value();	//实际只有一个MCON
				m_pDMX[m_SL32Num * 4 + m_SL40Num * 5 + route_id] = dmx_iter.value();	//实际只有一个MCON
             }

		//	m_DMX512Num++;	//这行错误
        }
    }
}

int MainWindow::getChangeStateNum()
{
    int change_number = 0;
    QMap<int,OutPutDevice*> out_put_device_list =
            StaticValue::GetInstance()->m_output_device;
    QMap<int,OutPutDevice*>::iterator iter = out_put_device_list.begin();
    for(;iter != out_put_device_list.end();++iter)
    {
        OutPutDevice* out_put_device = iter.value();
        if(out_put_device->bHasChange)
        {
            change_number++;
        }
    }
    return change_number;
}

void MainWindow::SaveDmxDataToFile()
{
    //数据顺序存放
    unsigned int j;
    unsigned char t=0;

    int change_num = getChangeStateNum();
    if(change_num != 0)
    {
        //写入时间
		t_buf[0] = ((int)m_PlayTimeCount & 0xff000000) >> 24;	//m_intDoTimeFromStart MSB
		t_buf[1] = ((int)m_PlayTimeCount & 0x00ff0000) >> 16;	//
		t_buf[2] = ((int)m_PlayTimeCount & 0x0000ff00) >> 8;	//
		t_buf[3] = ((int)m_PlayTimeCount & 0x000000ff) >> 0;	//m_intDoTimeFromStart LSB

        //同时发送的数量
        t_buf[4] = change_num;

        m_original_file.write(t_buf,5);		//写入数据头

        for(j=0;j<5;j++)
            t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码

        m_decrypt_file.write(t_buf,5);	//写入数据头

        change_num = 0;

        m_intRecordSnNumAll++;	//增加总记录数，同时发送的算一条
    }

    QMap<int,OutPutDevice*> out_put_device_list =
            StaticValue::GetInstance()->m_output_device;
    QList<int> keys = out_put_device_list.keys();
    for(int i = 0; i < keys.size();++i)
    {
        int index = keys.at(i);
        OutPutDevice* out_put_device = out_put_device_list[index];

        if(	(!out_put_device->bHasChange) ||		//判断有没有变化，没有变化就判断下个设备
			(!out_put_device->bHasChange) )
            continue;

        QString qStrIPAddress = out_put_device->ipAddress;
        QStringList ip_list = qStrIPAddress.split(".");
        if(ip_list.size() != 4)
        {
            qWarning() <<"IP地址有误："<<qStrIPAddress;
            continue;
        }

/*		if(out_put_device->type.compare("DO64Ethenet") == 0)
        {
            t_buf[0] = 0;	//写入类型,0:外部IP，1：自带
            t_buf[1] = ip_list.at(3).toInt();	//写入IP地址
            t_buf[2] = 0;	//写入数据长度高字节
            t_buf[3] = 20;	//写入数据长度低字节，实际的数据长度，不包含这8个字节
            m_original_file.write(t_buf,4);		//写入数据头

            for(j=0;j<4;j++)
                t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码
            //写入
            m_decrypt_file.write(t_buf,4);	//写入加密的数据头

            //	给第一个DO64发，IP是3
            t_buf[0]='D';	//标志头1
            t_buf[1]='O';	//标志头2

            t_buf[2]=hex_asc((m_pDMX[i*8+7] & 0xf0)>>4);
            t_buf[3]=hex_asc( m_pDMX[i*8+7] & 0x0f);

            t_buf[4]=hex_asc((m_pDMX[i*8+6] & 0xf0)>>4);
            t_buf[5]=hex_asc( m_pDMX[i*8+6] & 0x0f);

            t_buf[6]=hex_asc((m_pDMX[i*8+5] & 0xf0)>>4);
            t_buf[7]=hex_asc( m_pDMX[i*8+5] & 0x0f);

            t_buf[8]=hex_asc((m_pDMX[i*8+4] & 0xf0)>>4);
            t_buf[9]=hex_asc( m_pDMX[i*8+4] & 0x0f);

            t_buf[10]=hex_asc((m_pDMX[i*8+3] & 0xf0)>>4);
            t_buf[11]=hex_asc( m_pDMX[i*8+3] & 0x0f);

            t_buf[12]=hex_asc((m_pDMX[i*8+2] & 0xf0)>>4);
            t_buf[13]=hex_asc( m_pDMX[i*8+2] & 0x0f);

            t_buf[14]=hex_asc((m_pDMX[i*8+1] & 0xf0)>>4);
            t_buf[15]=hex_asc( m_pDMX[i*8+1] & 0x0f);

            t_buf[16]=hex_asc((m_pDMX[i*8] & 0xf0)>>4);
            t_buf[17]=hex_asc( m_pDMX[i*8] & 0x0f);

            t=0;
            for(j=0;j<18;j++)
                t = t + t_buf[j];

            t_buf[18]=hex_asc((t&0xf0)>>4);
            t_buf[19]=hex_asc(t&0x0f);

            m_original_file.write(t_buf,20);		//写入20字节数据

            for(j=0;j<20;j++)
                t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码
            //写入
            m_decrypt_file.write(t_buf,20);		//写入加密的数据

			out_put_device->bHasChange = false;
		}  */

		if(out_put_device->type.compare("SL32") == 0)
		{
			t_buf[0] = 0;	//写入类型,0:外部IP，1：自带
			t_buf[1] = ip_list.at(3).toInt();	//写入IP地址
			t_buf[2] = 0;	//写入数据长度高字节
			t_buf[3] = 8;	//DL3 + DATA（4）+CHECK(1) 写入数据长度低字节，实际的数据长度，不包含这4个字节
			m_original_file.write(t_buf,4);		//写入数据头

			for(j=0;j<4;j++)
				t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码
			//写入
			m_decrypt_file.write(t_buf,4);	//写入加密的数据头

			//	给第一个DO64发，IP是3
			t_buf[0]='D';	//标志头1
			t_buf[1]='L';	//标志头2
			t_buf[2]='3';	//标志头3

			t_buf[3]= m_pDMX[i*4+3];
			t_buf[4]= m_pDMX[i*4+2];
			t_buf[5]= m_pDMX[i*4+1];
			t_buf[6]= m_pDMX[i*4+0];

			t=0;
			for(j=0;j<7;j++)
				t = t + t_buf[j];

			t_buf[7]=t&0xff;

			m_original_file.write(t_buf,8);		//写入9字节数据

			for(j=0;j<8;j++)
				t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码
			//写入
			m_decrypt_file.write(t_buf,8);		//写入加密的数据

			out_put_device->bHasChange = false;
		}
		else if (out_put_device->type.compare("SL40") == 0)
		{
			t_buf[0] = 0;	//写入类型,0:外部IP，1：自带
			t_buf[1] = ip_list.at(3).toInt();	//写入IP地址
			t_buf[2] = 0;	//写入数据长度高字节
			t_buf[3] = 9;	//DL3 + DATA（4）+CHECK(1) 写入数据长度低字节，实际的数据长度，不包含这4个字节
			m_original_file.write(t_buf, 4);		//写入数据头

			for (j = 0; j<5; j++)
				t_buf[j] = t_buf[j] ^ DmxDataLockCode[j];	//改为乱码
			//写入
			m_decrypt_file.write(t_buf, 5);	//写入加密的数据头

			//	给第一个SL40发，IP是3
			t_buf[0] = 'D';	//标志头1
			t_buf[1] = 'L';	//标志头2
			t_buf[2] = '4';	//标志头3

			t_buf[3] = m_pDMX[i * 5 + 4];
			t_buf[4] = m_pDMX[i * 5 + 3];
			t_buf[5] = m_pDMX[i * 5 + 2];
			t_buf[6] = m_pDMX[i * 5 + 1];
			t_buf[7] = m_pDMX[i * 5 + 0];

			t = 0;
			for (j = 0; j<8; j++)
				t = t + t_buf[j];

			t_buf[8] = t & 0xff;

			m_original_file.write(t_buf, 9);		//写入8字节数据

			for (j = 0; j<9; j++)
				t_buf[j] = t_buf[j] ^ DmxDataLockCode[j];	//改为乱码
			//写入
			m_decrypt_file.write(t_buf, 9);		//写入加密的数据

			out_put_device->bHasChange = false;
		}
        else if(out_put_device->type.compare("DMX512") == 0)
        {
            QMap<int,int> dmx_state = out_put_device->m_route_state;
            QMap<int,int>::iterator dmx_iter = dmx_state.begin();
            int LengthTemp = dmx_state.size();

            //心跳信号另发
            t_buf[0] = 0;	//写入类型,0:转发，1:自用数据
            t_buf[1] = ip_list.at(3).toInt();	//写入IP地址，固定为15
            t_buf[2] = ((LengthTemp +8)&0xFF00)>>8;	//写入数据长度高字节
            t_buf[3] = ((LengthTemp +8)&0x00FF);		//写入数据长度低字节，实际的数据包的长度
            m_original_file.write(t_buf,4);		//写入数据头

            for(j=0;j<4;j++)
                t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码
            //写入
            m_decrypt_file.write(t_buf,4);		//写入加密的数据

            //控制E-MCON
            t_buf[0]='D';		//D
            t_buf[1]='X';		//X

            t_buf[2]=0x44;		//CMD=0X44

            t_buf[3] = (0 & 0x0300)>>8;		//data_ppp MSB，0到511，最大511
            t_buf[4] = (0 & 0x00ff);		//data_ppp LSB

            t_buf[5] = (LengthTemp & 0x0300)>>8;	//data_len MSB，1到512，最大512
            t_buf[6] = (LengthTemp & 0x00ff);		//data_len LSB

            int ct = 7;
            for(i = 0 ; i < LengthTemp ; i++,dmx_iter++)
                t_buf[ct+i]= dmx_iter.value();


            ct = ct + LengthTemp;

            t=0;
            for(i=0;i<ct; i++)
                t=t+t_buf[i];
            t_buf[ct]=t;

            m_original_file.write(t_buf,ct+1);

            for(j=0;j<(ct+1);j++)
                t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码
            //写入
            m_decrypt_file.write(t_buf,ct+1);		//写入加密的数据

			out_put_device->bHasChange = false;
        }
    }
}

//MCON的心跳数据包
void MainWindow::SaveDmxHeartData()
{
    QMap<int,OutPutDevice*> out_put_device_list =
            StaticValue::GetInstance()->m_output_device;
    QMap<int,OutPutDevice*>::iterator iter = out_put_device_list.begin();
    for(;iter != out_put_device_list.end();++iter)
    {
        OutPutDevice* out_put_device = iter.value();
        if(out_put_device->type.compare("DMX512") == 0)
        {
            QString qStrIPAddress = out_put_device->ipAddress;
            QStringList ip_list = qStrIPAddress.split(".");
            if(ip_list.size() != 4)
            {
                qWarning() <<"IP地址有误："<<qStrIPAddress;
                continue;
            }

            unsigned char i,j;
            unsigned char t=0;

			t_buf[0] = ((int)m_PlayTimeCount & 0xff000000) >> 24;	//m_intDoTimeFromStart MSB
			t_buf[1] = ((int)m_PlayTimeCount & 0x00ff0000) >> 16;	//
			t_buf[2] = ((int)m_PlayTimeCount & 0x0000ff00) >> 8;	//
			t_buf[3] = ((int)m_PlayTimeCount & 0x000000ff) >> 0;	//m_intDoTimeFromStart LSB
            t_buf[4] = 1;			//

            m_original_file.write(t_buf,5);		//写入数据头

            for(j=0;j<5;j++)
                t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码
            //写入
            m_decrypt_file.write(t_buf,5);		//写入加密的数据

            t_buf[0] = 0;	//写入类型,0:转发，1:自用数据
            t_buf[1] = ip_list.at(3).toInt();;	//写入IP地址，固定为15
            t_buf[2] = 0;	//写入数据长度高字节
            t_buf[3] = 4;	//写入数据长度低字节，实际的数据包的长度
            m_original_file.write(t_buf,4);		//写入数据头

            for(j=0;j<4;j++)
                t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码
            //写入
            m_decrypt_file.write(t_buf,4);		//写入加密的数据

            //控制E-MCON
            t_buf[0]='D';		//D
            t_buf[1]='X';		//X

            t_buf[2]=0x60;		//CMD=0X60

            t=0;
            for(i=0;i<3; i++)
                t=t+t_buf[i];
            t_buf[3]=t;

            m_original_file.write(t_buf,4);

            for(j=0;j<4;j++)
                t_buf[j] = t_buf[j]^DmxDataLockCode[j];	//改为乱码
            //写入
            m_decrypt_file.write(t_buf,4);		//写入加密的数据

            m_intRecordSnNumAll++;	//增加总记录数
        }
    }
}

void MainWindow::SaveDMXHeaderData()
{
	char fdata[524];	//至少要承下一条记录
	StaticValue::GetInstance()->ZeroMemoryEx(fdata,256);
	unsigned char cT0,cT1,cT2,cT3;

	//写入DMX数据文件标志头(16个字节)，256字节
	//位置0-15
	//DMX数据文件标志头
//    string str = "类别:DMX数据文件";		//16字节
	string str = "Kind:DMXDataFile";	//16字节
	for(int i=0;i<str.length();i++)
		fdata[i] = str.at(i);

	//每条记录的长度，时间(4)+包头命令起始长度校验(8)+数据(22),数据是14个FA8，一个AO8
	//位置16-19，4个字节
	int iRecordLen = 4+8+(FA8_N + AO8_N*8);	//2字节的长度
	cT0=(iRecordLen & 0xff00)>>8;
	fdata[16] = cT0;
	cT1=(iRecordLen & 0x00ff);
	fdata[17] = cT1;

	//反相部分
	cT0=cT0^0xff;
	fdata[18] = cT0;
	cT1=cT1^0xff;
	fdata[19] = cT1;

	//写入带不带音乐的属性，
	//位置28
	lOffset = DMX_INF_HEAD+28;		//0

	cT0='0';		//程控
	if(NEW_MUSIC == m_MusicType)
	{
		cT0 = '1' ;	//音乐
	}
	fdata[28] = cT0;

	//写入音乐文件名称
	//位置29起，最长12字节
//	str = "DMX测试";
	str = "DMXTest";
	if(str.length()>12)
		QMessageBox::information(this, tr("提示"),
								 tr("音乐文件名称过长，数据文件存储错误!"), QMessageBox::Ok);
	else
	{
		for(int i=0;i<str.length();i++)
			fdata[i+29] = str.at(i);
	}

	lOffset = DMX_INF_END - DMX_INF_CR;	//单回车即可
	fdata[lOffset  ] = 0x0D;
	fdata[lOffset+1] = 0x0A;

	//最后写入总记录数，正反相合计8字节
	//位置20-27
	lOffset = DMX_INF_HEAD+20;		//0
	cT0=(m_intRecordSnNumAll & 0xff000000)>>24;	//4字节，MSB，高位在前
	fdata[lOffset  ] = cT0;
	cT1=(m_intRecordSnNumAll & 0x00ff0000)>>16;
	fdata[lOffset+1] = cT1;
	cT2=(m_intRecordSnNumAll & 0x0000ff00)>>8;
	fdata[lOffset+2] = cT2;
	cT3=(m_intRecordSnNumAll & 0x000000ff);		//LSB，低位在后
	fdata[lOffset+3] = cT3;

	cT0=cT0^0xff;		//4字节，MSB，反相部分高位在前
	fdata[lOffset+4] = cT0;
	cT1=cT1^0xff;
	fdata[lOffset+5] = cT1;
	cT2=cT2^0xff;
	fdata[lOffset+6] = cT2;
	cT3=cT3^0xff;		//4字节，LSB，反相部分低位在后
	fdata[lOffset+7] = cT3;

	lOffset = DMX_INF_HEAD;		//0
	lActual = m_original_file.seek(lOffset);
	m_original_file.write(fdata,256);	//写入数据

	m_original_file.flush();	//写全数据
	m_original_file.close();	//关闭文件

	//写加密文件
	lOffset = DMX_INF_HEAD;		//0
	lActual = m_decrypt_file.seek(lOffset);
	for(int i=0;i<256;i++)
		fdata[i] = fdata[i]^DmxHeadLockCode[i];	//加密
	m_decrypt_file.write(fdata,256);	//写入数据

	m_decrypt_file.flush();	//写全数据
	m_decrypt_file.close();	//关闭文件

	//清除所有表演的数据，
	m_intDoPoint=0;
	m_PlayTimeCount = 0;		//曲子运行了的时间长度，以ms为单位
}

void MainWindow::CreateListenSocket()
{
    CommonSocket_   =   new QUdpSocket(this);
    CommonSocket_->bind(rcstatus_.CommonPort);

    DMXSocket_      =   new QUdpSocket(this);
    DMXSocket_->bind(rcstatus_.DMXPort);


    connect(CommonSocket_, SIGNAL(readyRead()),
            this, SLOT(CommonRecvCommand()));

    connect(DMXSocket_, SIGNAL(readyRead()),
            this, SLOT(DMXRecvCommand()));
}

void MainWindow::InitStatusBarWidget()
{
    mybar_   =   new QStatusBar;
    mybar_->showMessage("Ready!");
    statusLabel_   =    new QLabel;
    workingstatus_  =   new QLabel;
	pingstatus_ = new QLabel;
    mybar_->insertPermanentWidget(0,statusLabel_);
    mybar_->insertPermanentWidget(0,workingstatus_);
	mybar_->insertPermanentWidget(1, pingstatus_);
    setStatusBar(mybar_);

    UpdateStatusBar();
}

void MainWindow::UpdateStatusBar()
{
    QString workingmode;
    QString ListenPort;
    if(rcstatus_.WorkingMode==remotecontrol::Mode::COMMON)
    {
        workingmode =   tr("常规模式");
        ListenPort  =   tr("监听端口:%1|").arg(rcstatus_.CommonPort);
    }
    else if(rcstatus_.WorkingMode==remotecontrol::Mode::DMX)
    {
        workingmode =   "DMX";
        ListenPort  =   tr("监听端口:%1|").arg(rcstatus_.DMXPort);

    }
    else if(rcstatus_.WorkingMode==remotecontrol::Mode::NONE)
    {
        workingmode =  tr("不接受控制");
        ListenPort  =   "";
    }

    workingstatus_->setText(workingmode+ListenPort+rcstatus_.Source);
//    CommonSocket_->disconnected();
//    DMXSocket_->disconnected();

    if(rcstatus_.WorkingMode==remotecontrol::Mode::COMMON )
    {
        if(DMXTimer_->isActive())          //如果DMX定时器是工作状态，就停掉，因为已经切换到常规模式了
        {
            DMXTimer_->stop();
        }

        if(CommonSocket_->isValid())        //重新绑定新的端口
        {
            CommonSocket_->disconnectFromHost();
            CommonSocket_->bind(rcstatus_.CommonPort);
        }
    }
    else if(rcstatus_.WorkingMode==remotecontrol::Mode::DMX)
    {
        if(!DMXTimer_->isActive())          //如果DMX定时器没有工作，就启动，因为切换到DMX模式了
        {
            DMXTimer_->start(1000);
        }
        if(DMXSocket_->isValid())           //重新绑定新的端口
        {
            DMXSocket_->disconnectFromHost();
            DMXSocket_->bind(rcstatus_.DMXPort);
        }
    }
    else if(rcstatus_.WorkingMode==remotecontrol::Mode::NONE)
    {
        if(DMXTimer_->isActive())          //如果DMX定时器没有工作，就启动，因为切换到DMX模式了
        {
            DMXTimer_->stop();
        }
        CommonSocket_->disconnectFromHost();
        CommonSocket_->bind(rcstatus_.CommonPort);
        DMXSocket_->disconnectFromHost();
        DMXSocket_->bind(rcstatus_.DMXPort);
    }
}

void MainWindow::RequestDMX()
{
    char sendbuf[10];

    sendbuf[0]='F';		//包头，三个字节
    sendbuf[1]='5';
    sendbuf[2]='1';
    sendbuf[3]='2';

    sendbuf[4]=0x01;		//CMD，读取一次DMX512数据


    sendbuf[5] = (rcstatus_.DataIndex & 0x0300)>>8;	//Start MSB，0到511，最大511
    sendbuf[6] = (rcstatus_.DataIndex & 0x00ff);		//Start LSB

    sendbuf[7] = (rcstatus_.DataLength & 0x0300)>>8;		//Len MSB，1到512，最大512
    sendbuf[8] = (rcstatus_.DataLength & 0x00ff);			//Len LSB

    unsigned char t=0;
    for(int i=0;i<9;i++)
        t=t+sendbuf[i];
    sendbuf[9]=t&0xff;

    QHostAddress destaddr(rcstatus_.DMXAddr);
    DMXRequest_->writeDatagram(sendbuf,10,destaddr,rcstatus_.DMXPort);
    qDebug()<<"dmx request"<<rcstatus_.DMXAddr;
}

void MainWindow::PrepareHeartBeat()
{
    HeartBeat_  =   new QTimer;
    HeartBeatSocket_    =   new QUdpSocket;
	HeartBeat_->setTimerType(Qt::PreciseTimer);
    connect(HeartBeat_,&QTimer::timeout,this,&MainWindow::HeartBeatTimeOut);
}

void MainWindow::initPingThreadData()
{
	// 初始化 out_put_device_list
	QMap<int, QString> out_put_device_list;
	QMap<int, OutPutDevice*> out_ping_device = StaticValue::GetInstance()->m_output_device;

	for (auto it = out_ping_device.constBegin(); it != out_ping_device.constEnd(); ++it) {
		out_put_device_list.insert(it.key(), it.value()->ipAddress);
	}
	// ... 填充 out_put_device_list, 这里可以添加测试IP手动
	//out_put_device_list.insert(11, QString("192.168.3.109"));
	//out_put_device_list.insert(12, QString("192.168.3.109"));
	
	m_pingThread = new PingThread(out_put_device_list);
	m_thread = new QThread(this);
	m_pingThread->moveToThread(m_thread);

	connect(m_thread, SIGNAL(started()), m_pingThread, SLOT(start()));
	connect(m_pingThread, SIGNAL(pingResult(double)), this, SLOT(handlePingResult(double)));
	connect(m_thread, SIGNAL(finished()), m_pingThread, SLOT(deleteLater()));
	connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));

	m_thread->start();

}

void MainWindow::HTTPtimerup()
{
    GetTaskFromHTTP();

//    if(CurrentJobID_!=-1)           //忙碌状态
//    {
//        return;
//    }
//    else
//    {
//        GetTaskFromHTTP();
//    }
}

//存储QUE文件
void MainWindow::on_action_Query_triggered()
{
	QString qStrFileName = QFileDialog::getSaveFileName(this, tr("新建表演序列文件"),
														"/home/user/sovotek", tr("Que文件(*.Que)"));
	QString qStrOldFileName = qStrFileName + "O";
	m_original_file.setFileName(qStrOldFileName);
	if(!m_original_file.open(QIODevice::WriteOnly))
	{
		QMessageBox::information(this, tr("提示"),
								 tr("创建并打开DMX原始文件失败！"), QMessageBox::Ok);
		return ;
	}
	m_decrypt_file.setFileName(qStrFileName);
	if(!m_decrypt_file.open(QIODevice::WriteOnly))
	{
		QMessageBox::information(this, tr("提示"),
								 tr("创建并打开加密的DMX文件失败！"), QMessageBox::Ok);
		return ;
	}

	int	sp;					//存储指针
	int	i,j,nLength;		//文件名长度

	char cT0,cT1,cT2,cT3;

	int iRdBytes;	//实际读出来的字节数量
	char fdata[128];
	MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
	//如果当前编曲ID和上次编曲ID不同时，重新更新编曲内表演动作列表；
	if(current_widget->id != m_CurrentMusicID)
	{
		m_CurrentMusicID = current_widget->id;
		m_CurrentMusicActions.clear();
		m_CurrentMusicActions = current_widget->getMusicActions();
	}
	int music_length = current_widget->getTimeLength()*1000;
	m_MusicType = current_widget->getType();
	m_MusicName = current_widget->getMusicName();

	// 写入播放队列数据文件标志头(18个字节)
	//位置0-17
	lOffset = QUEUE_INF_HEAD;		//0
	lActual = m_original_file.seek( lOffset );

	//播放队列数据文件标志头，该段18字节
//	string str = "类别:Queue数据文件";		//18字节
	string str = "Kind:QueueDataFile";		//18字节
	m_original_file.write(str.c_str(),str.length());
	str="\r\n";
	m_original_file.write(str.c_str(),str.length());	//写入回车，2字节

	//写入编曲的数量(2字节)，HEX格式，bottom.m_fileOpened是unsigned int，该段13字节
//	str = "队列总数:";				//9字节
	str = "CompNums:";				//9字节，位置18-26
	m_original_file.write(str.c_str(),str.length());
	cT0 = (m_query.m_fileOpened&0xff00)>>16;	//2字节
	cT1 = (m_query.m_fileOpened&0x00ff);
	m_original_file.write(&cT0,1);			//先写入高8位，位置27
	m_original_file.write(&cT1,1);			//再写入低8位，位置28
	str="\r\n";
	m_original_file.write(str.c_str(),str.length());	//写入回车，2字节，位置29-30

	//写入运行方式(1)字节，0：顺序，1：循环，2：随机循环，该段12字节
//	str = "播放方式:";		//9字节
	str = "PlayMode:";		//9字节，位置31-39
	m_original_file.write(str.c_str(),str.length());

	char szMode = (char)m_query.intDisMode;
	m_original_file.write(&szMode,1);	//1字节，位置40
	str="\r\n";
	m_original_file.write(str.c_str(),str.length());	//写入回车，2字节，位置41-42

	//写入循环次数(4字节)，HEX格式，-1是无限次循环，0-32767为有限次循环，该段15字节
//	str = "循环次数:";		//9字节
	str = "LoopNums:";		//9字节，位置43-51
	m_original_file.write(str.c_str(),str.length());	//4字节
	cT0 = (m_query.DisQueueLoopALL & 0xff000000)>>24;
	cT1 = (m_query.DisQueueLoopALL & 0x00ff0000)>>16;
	cT2 = (m_query.DisQueueLoopALL & 0x0000ff00)>>8;
	cT3 = (m_query.DisQueueLoopALL & 0x000000ff);
	m_original_file.write(&cT0,1);			//先写入高8位 MSB，位置52
	m_original_file.write(&cT1,1);			//
	m_original_file.write(&cT2,1);			//
	m_original_file.write(&cT3,1);			//再写入低8位 LSB，位置55
	str="\r\n";
	m_original_file.write(str.c_str(),str.length());	//写入回车，2字节

	//预留部分写乱码，
	char ran;
	for(i=0;i<(QUEUE_INF_LEN - QUEUE_INF_CR-18-12-15);i++)
	{
		ran=' ';
		m_original_file.write(&ran,1);	//写入1字节随机数
	}

	//QUEUE_INF_HEAD是INF存储的头位置，
	lOffset = QUEUE_INF_HEAD + QUEUE_INF_LEN - QUEUE_INF_CR;	//单回车即可
	lActual = m_original_file.seek( lOffset);
	str="";
	for(i=0;i<(QUEUE_INF_CR/2);i++)
		str=str+"\r\n";
	m_original_file.write(str.c_str(),str.length());	//写入回车

	//写入播放队列数据，文件名称，QUEUE文件
	for(sp=1;sp<=m_query.m_fileOpened;sp++)	//如果打开三个编曲，bottom.m_fileOpened的值为3
	{
		//获得编曲文件的名字

		//写入音乐文件名称
		//位置29起，最长96字节
		lOffset =	QUEUE_INF_HEAD + QUEUE_INF_LEN +		//INF部分
				QUEUE_DATA_HEAD + QUEUE_DATA_LEN*(sp-1);	//DMX文件名称部分，sp从1开始
		lActual = m_original_file.seek( lOffset);

		//获得歌曲名(不包含路径的部分，包括后缀)
		nLength = qStrFileName.length();
		for( int nCount = (nLength-1); nCount >0; nCount-- )
		{
			if( qStrFileName.at( nCount ) == '/' )	//GetAt是在Cstring里面获得指定位置的一个字符
			{
				qStrFileName = qStrFileName.right( nLength-nCount-1);	//Right获得右边的剩余字符，获得文件名称，包含后缀
				break;
			}
		}

		//去掉后缀
		nLength = qStrFileName.length();        //去掉后缀
		for( int nCount = (nLength-1); nCount >0; nCount-- )
		{
			if( qStrFileName.at( nCount ) == '.' )	//GetAt是在Cstring里面获得指定位置的一个字符
			{
			//    qStrFileName = qStrFileName.left(nLength-nCount-1);	//Right获得右边的剩余字符，获得文件名称，包含后缀
				qStrFileName = qStrFileName.left(nCount);	//Right获得右边的剩余字符，获得文件名称，包含后缀

				qStrFileName += ".dmx";
				break;
			}
		}

//		str=str.Left(end);
		//加上dmx的后缀
//		str = str + ".dmx";
		//加上dmx的后缀
//        str = "new_dmx.dmx";

		if(qStrFileName.length()>12)	//DOS8.3格式
		{
			QMessageBox::information(this, tr("提示"),
									 tr("音乐名称文件过长"), QMessageBox::Ok);
		}
		else
		{
			str=qStrFileName.toStdString();
			m_original_file.write(str.c_str(),str.length()+1);
		}

		//预留部分写乱码，
		lOffset =	QUEUE_DATA_LEN - QUEUE_DATA_CR -12;		//DOS8.3格式最多12字节
		for(i=0;i<lOffset;i++)
		{
			ran=' ';
			m_original_file.write(&ran,1);	//写入1字节随机数
		}

		lOffset =	QUEUE_INF_HEAD + QUEUE_INF_LEN +            //INF部分
					QUEUE_DATA_HEAD + QUEUE_DATA_LEN*(sp-1) -	//DMX文件名称部分
					QUEUE_DATA_CR;								//回车
		lActual = m_original_file.seek( lOffset );
		str="";
		for(i=0;i<(QUEUE_DATA_CR/2);i++)
			str=str+"\r\n";
		m_original_file.write(str.c_str(),str.length());	//写入回车

	}

	m_original_file.flush();	//写全数据
	m_original_file.close();	//关闭文件

	//加密部分
	//以读方式打开文件
	m_original_file.setFileName(qStrOldFileName);
	if(!m_original_file.open(QFile::ReadOnly))
	{
		QMessageBox::information(this, tr("提示"),
								 tr("打开播放队列数据文件失败"), QMessageBox::Ok);
		return ;
	}

	//获得文件的长度
	nLength = m_original_file.size();

	sp=0;	//文件指针
	for(i=0;i<nLength;i+=128)
	{
		//读定位
		lActual = m_original_file.seek(sp);

		//每次读出来128字节
		iRdBytes = m_original_file.read(fdata,128);

		for(j=0;j<iRdBytes;j++)
			fdata[j] = fdata[j]^QueueLockCode[j];	//改为乱码

		//写定位
		lActual = m_decrypt_file.seek(sp);

		//写入
		m_decrypt_file.write(fdata,iRdBytes);	//写回去数据

		sp = sp + iRdBytes;
	}
	m_original_file.close();	//关闭文件

	m_decrypt_file.flush();	//写全数据
	m_decrypt_file.close();	//关闭文件

	QMessageBox::information(this, tr("提示"),
							 tr("播放队列数据文件存储完毕!"), QMessageBox::Ok);
}


void MainWindow::removeSubTab(int index)	//926删除标签
{
	if(index!=0)
	{
		//m_tabWidget->currentIndex(index)  
		MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
		current_widget->stop();
		m_tabWidget->removeTab(index);
	}

}

void MainWindow::on_loopsingle_triggered()
{
    if(ui->loopsingle->isChecked())
    {
         m_isloopMusic = 1;
    }
    else
    {
         m_isloopMusic = 0;
    }

    ui->loopsingle->setChecked(ui->loopsingle->isChecked());
}

void MainWindow::looptimeOut()
{
	//一旦开始运行，这个函数会不断地调用
	MusicWidget* current_widget = (MusicWidget*)m_tabWidget->currentWidget();
//	int music_length = current_widget->getTimeLength() * 1000;				//获得编曲的总运行时间
	if(m_isloopMusic==1)
	{
		//判断是不是运行结束
		if(current_widget->m_timeline->state()==(QTimeLine::NotRunning))	//重新开始后，时间线存在，输出正常
	//	if(m_PlayTimeCount >= music_length)									//使用这一句的重新开始后没有了时间线，但是输出正常
		{
		//	qDebug()<<"looptime is out!!!!!!!!!!!!!!!";
		//	current_widget->m_timeline->setLoopCount(0);
			m_CtrlModeFlag=0;			//同时清除掉ipad有数据的标志
			on_start_triggered();		//重新开始
		}
	}
}

void MainWindow::HeartBeatTimeOut()
{
    QByteArray  arr;
    arr.resize(6);
    arr.fill(0);
    static  quint8  d   =   255;
    arr[0]  =   d;
    HeartBeatSocket_->writeDatagram(arr,QHostAddress("192.168.1.1"),3333);
    d  =     ~d;
}



void MainWindow::GetImgFinished()
{

    //本地保存web图片
    QByteArray imgarray =   Imgreply_->readAll();
    QFile imgfile("hua");
    imgfile.open(QIODevice::WriteOnly);
    imgfile.write(imgarray);
    imgfile.close();

    //播放数据、状态设置
	ipadimage_currtenttime = m_PlayTimeCount;
    m_CtrlModeFlag  =   1;
    last_image_array    =   imgarray;
//    QFileInfo fi;
//    if (file)
//        file->write(reply->readAll());
//    if (file) {
//        fi.setFile(file->fileName());
//        file->close();
//        delete file;
//        file = nullptr;
//    }

//    if (httpRequestAborted) {
//        Imgreply_->deleteLater();
//        Imgreply_ = nullptr;
//        return;
//    }

//    if (Imgreply_->error()) {
//        QFile::remove(fi.absoluteFilePath());
//        statusLabel->setText(tr("Download failed:\n%1.").arg(reply->errorString()));
//        downloadButton->setEnabled(true);
//        reply->deleteLater();
//        reply = nullptr;
//        return;
//    }
}

void MainWindow::send_status_finished()
{
    qDebug()<<"send status finished!";
    qDebug()<<reply_->url();
    qDebug()<<reply_->attribute(QNetworkRequest::HttpReasonPhraseAttribute);
    qDebug()<<reply_->rawHeaderList();
    qDebug()<<reply_->rawHeaderPairs();
}

void MainWindow::Get_Task_finished()
{

//    qDebug()<<Taskreply_->attribute(QNetworkRequest::HttpReasonPhraseAttribute);

//    qDebug()<<Taskreply_->rawHeaderList();
//    qDebug()<<Taskreply_->rawHeaderPairs();
//    QString DataAsString = QTextCodec::codecForMib(106)->toUnicode(Taskreply_->readAll());
//    QRegExp rx("[,\n]");// match a comma or a space
//    QStringList list = DataAsString.split(rx, QString::SkipEmptyParts);
//    qDebug() << list.at(3);
//    qDebug() << list.at(4);
//    qDebug() << list.at(5);
//    qDebug() << list.at(6);
    QJsonDocument TaskJson  =  QJsonDocument::fromJson(Taskreply_->readAll());
//    qDebug()<<TaskJson;
//    qDebug()<<TaskJson["Code"];
//    QJsonValue DataJson =   TaskJson["Data"];
//    qDebug()<<DataJson.type();
//    qDebug()<<TaskJson["Data"];
//    qDebug()<<TaskJson["Data"]["image"];
 QJsonObject taskJsonObject = TaskJson.object();
 if (taskJsonObject["Data"].toObject()["has_job"].toBool() == true)
   //modify by eagle if(TaskJson["Data"]["has_job"]==true)
    {
        //1 try to get image
//        qDebug()<<"you got new mission";
//        qDebug()<<TaskJson["Data"]["has_job"];
//        qDebug()<<TaskJson["Data"]["mode"];
//        qDebug()<<TaskJson["Data"]["job_id"];

        //QJsonValue  id  =   TaskJson["Data"]["job_id"];
        //modify by eagle
        QJsonValue id = taskJsonObject["Data"].toObject()["job_id"];
        if(id.toInt()==CurrentJobID_)
        {
//            QMessageBox::information(this,"warning","repeat mission!");
            qDebug()<<"repeat mission!  repeat mission!  repeat mission!  repeat mission!  repeat mission!";
//            return;
        }
        CurrentJobID_   =   id.toInt();
        //QJsonValue  imgvalue    =   TaskJson["Data"]["image"];
 		//modify by eagle
         QJsonValue imgvalue = taskJsonObject["Data"].toObject()["image"];
        QString imgstr  =   imgvalue.toString();
        if(imgstr.isEmpty())                //图片为空的情况，不做任何操作
        {
            Q_ASSERT(FALSE);
            StatusToHTTP(CurrentJobID_,2);
            StatusToHTTP(CurrentJobID_,3);
            return;
        }
        QStringList list1 = imgstr.split(',', Qt::SkipEmptyParts);
        //本地保存web图片
        QString imgstrpng   =   list1.at(1);
        QByteArray imgarray =   imgstrpng.toUtf8();
        imgarray    =   QByteArray::fromBase64(imgarray);

        int timestamp   =   QDateTime::currentMSecsSinceEpoch();
        QString strstamp    =   QString("%1.jpg").arg(timestamp);
        QImage image;
        image.loadFromData(imgarray);
        bool res    =   image.save("./webimg/"+strstamp); // writes image into ba in PNG format
//        bool res    =   image.save(strstamp); // writes image into ba in PNG format

        //回报运行状态状态
        StatusToHTTP(CurrentJobID_,2);
        //准备表演数据
		ipadimage_currtenttime = m_PlayTimeCount;
        m_CtrlModeFlag  =   1;
        last_image_array    =   imgarray;

        qDebug()<<"new mission"<<"_____"<<CurrentJobID_;
    }
    else
    {
        qDebug()<<"no mission yet"<<"_____"<<CurrentJobID_<<"_____"<<m_CtrlModeFlag;
    }
//    QJsonValue DataValue   =   TaskJson["Data"];
//    qDebug()<<DataValue["has_job"];
//    qDebug()<<DataValue["image"];
//    qDebug()<<DataValue["job_id"];
//    qDebug()<<DataValue["mode"];

}

//从这里弹出一个对话框，全屏，绘图
//void MainWindow::on_action_DRAW_triggered()
//{
////以下代码为原来打算用PC绘图弹出的绘图对话框的代码，在苹果发送模式下屏蔽
//    /*
//         DrawTableDialog draw_dialog;
//         draw_dialog.setModal(true);
//         draw_dialog.showFullScreen();
//         draw_dialog.exec();
//    */

//    //ipad发送模式下修改为：点击该按钮，则绘图由苹果IPAD完成，IPAD  send SOCKET
//    //同时禁用编曲开始跑动的按钮（该功能是否需要？）
//    //再次点击，恢复为播放表演模式，不再接收ipad发送的消息
//    //设立一个flag，用以判断是什么模式工作：m_CtrlModeFlag,0是自主播放表演，1是由ipad发送图片，缺省初始化为0

//        if(m_CtrlModeFlag)
//        {
//            m_CtrlModeFlag=0;
//            ui->action_DRAW->setText(tr("绘图互动"));
////            ui->start->setEnabled(1);
//            if(server->isListening())
//             {
//                server->close();
//              //  m_pEdt_Info->append(QObject::tr("停止监听！"));
//                QMessageBox::information(this, tr("提示"),
//                                         tr("停止监听！"), QMessageBox::Ok);
//             }

//        }
//        else
//        {
// //           m_CtrlModeFlag=1;
//            ui->action_DRAW->setText(tr("已选择绘图模式"));
//  //        ui->start->setEnabled(0);
//            if (!server->isListening())
//            {
//                if (server->listen(QHostAddress::Any, ipad_port))
//                {
//              //      m_pEdt_Info->append(QObject::tr("打开监听端口成功！"));
//                    QMessageBox::information(this, tr("提示"),
//                                             tr("打开监听端口成功！"), QMessageBox::Ok);
//                }
//                else
//                {
//              //      m_pEdt_Info->append(QObject::tr("打开监听端口失败！"));
//                    QMessageBox::information(this, tr("提示"),
//                                                 tr("打开监听端口失败！"), QMessageBox::Ok);

//                }
//            }
//            else
//            {
//             // m_pEdt_Info->append(QObject::tr("正在监听中...！"));
//                QMessageBox::information(this, tr("提示"),
//                                                 tr("正在监听中...！"), QMessageBox::Ok);
//            }
//        }

//}




//void MainWindow::receiveImgTimeOut()
//{

//    QByteArray datagram;
//    // 让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
//    datagram.resize(udp_socket.pendingDatagramSize());
//           // 接收数据报，将其存放到datagram中
//    udp_socket.readDatagram(datagram.data(), datagram.size());

//    save_receive_img(datagram);
//}



void MainWindow::newConnect()
{
/*
    while(1){
        tcpSocket = server->nextPendingConnection();
        if(tcpSocket)
          processPendingDatagram();
    }
    //if(!tcpSocket)
    //    return;
//    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagram()));
*/
//    m_pEdt_Info->append(QObject::tr("有新客户端连接到服务器"));
//    QMessageBox::information(this, tr("提示"),
//                                     tr("有新客户端连接到服务器"), QMessageBox::Ok);
    tcpSocket = server->nextPendingConnection();
    qint64 bytes_a = tcpSocket->bytesAvailable();
//    int rightlen=img_heigth*img_weigth*4+54+7;
    int rightlen = StaticValue::GetInstance()->str_ipaimageW.toInt() *
                   StaticValue::GetInstance()->str_ipaimageH.toInt() *
                   4+54+7;
    tcpSocket->setReadBufferSize(rightlen);
//    connect(tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()));
    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(processPendingDatagram()));
//    int back_length = 0;
//    QString vMsgStr = QObject::tr("Welcome");
//    if((length=m_pSocket->write(vMsgStr.toLatin1(),vMsgStr.length()))!=vMsgStr.length())
//    {
//    }

}

//ipad数据包的处理，包括手机数据包吗？
void MainWindow::processPendingDatagram() //处理等待的数据报
{
/*
    tcpSocket->waitForReadyRead(1000);
    QByteArray datagram = tcpSocket->readAll(); //读取; //拥于存放接收的数据包

    QString strMesg="OK";
    tcpSocket->write(strMesg.toStdString().c_str(),strlen(strMesg.toStdString().c_str())); //发送
    tcpSocket->waitForBytesWritten(1000);

//    qDebug()<<datagram;

    do{

        datagram.resize(receiver->pendingDatagramSize());
        //让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
        receiver->readDatagram(datagram.data(),datagram.size());
        //接收数据报，将其存放到datagram中

        //保存为bmp
      }while(receiver->hasPendingDatagrams());  //拥有等待的数据报
 */

//	int rightlen=img_heigth*img_weigth*4+54+7;	//图片数据的长度
	int rightlen =	StaticValue::GetInstance()->str_ipaimageW.toInt() *
					StaticValue::GetInstance()->str_ipaimageH.toInt() *	4+54+7;
	int padW,padH;
	padW    =   StaticValue::GetInstance()->str_ipaimageW.toInt();	//ipad图片的宽度
	padH    =   StaticValue::GetInstance()->str_ipaimageH.toInt();	//ipad图片的高度
	QByteArray tempdatagram;
	tempdatagram = tcpSocket->readAll();	//获取所有的数据
	ba+=tempdatagram;						//ba是图片数据的二维阵列，
	if(JudgeArraySize(ba))
	{
		//QMessageBox::information(NULL,"info","bingo! 51261");
		ipadimage_currtenttime = m_PlayTimeCount;	//把当前已运行时长的值赋给ipadimage_currtenttime
		m_CtrlModeFlag=1;				//有ipad图片的标志
	//	save_receive_img(ba);
		save_receive_img2021(ba);		//存储图片，数据在变量ba内，并在存储完之后清除ba内的数据
	}
}
bool MainWindow::JudgeArraySize(QByteArray datagram)
{
	datagram    =   datagram.right(datagram.size()-3);//刨除三个字节头
    QDataStream ds(datagram);
    int picsize; // Since the size you're trying to read appears to be 2 bytes
    ds >> picsize;
    datagram    =   datagram.right(datagram.size()-4);//减去一个保存图片文件大小的int，4字节
    int datasize    =   datagram.size();
    if(picsize==datasize)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void MainWindow::save_receive_img2021(QByteArray datagram)
{
	qDebug()<<"bingo!";
	datagram = datagram.right(datagram.size()-7);			//刨除三个字节的报文头，和四个字节的保存图片大小的int
	QImage netImage;
	netImage.loadFromData(datagram);
	QDateTime time_curr = QDateTime::currentDateTime();						//获取系统现在的时间
	QString str_curr = time_curr.toString("yyyy年M月d日hh时mm分ss秒");		//设置显示格式
	bool res    =   netImage.save(QString("%1.bmp").arg(str_curr),"bmp");	//保存图片？
	qDebug()<<"保存图片是否成功"<<res;
	last_image_array    =   datagram;			//将datagram存储起来，等到播放的时候的用
	ba.clear();									//清空保存网络图片缓存的中间变量
}

void MainWindow::processPendingDatagram2021()
{
//    QByteArray tempdatagram;
//    tempdatagram = tcpSocket->readAll();
//    ba+=tempdatagram;
//    int basize = ba.size();
//    if(basize==rightlen)
//    {
//        ipadimage_currtenttime=m_HeartCount;
//        m_CtrlModeFlag=1;
//        save_receive_img(ba);
//    }
}


void MainWindow::save_receive_img(QByteArray datagram)
{
//    QString filename = "D:\绍兴水帘\pictures\20160723111.bmp";
//    QImage temp_img = QImage(datagram,img_weigth,img_heigth,QImage::Format_Indexed8,0,0);
//    bool status = temp_img.save(filename,"bmp");

//加密部分
/*
    int JMbuf_len;      //加密的数据的长度
    JMbuf_len=datagram.length();
    for(int j=0;j<JMbuf_len;j++)
        datagram[j] = datagram[j]^DmxDataLockCode[j];	//获得正确参数
*/

//分析数据部分
    union Char2Int
    {
        int i;
        char cha[4];
    };
    Char2Int Union4Con; //定义一个用于类型转换的联合类型
    Union4Con.cha[0]   =   datagram[6];     //给联合赋值，顺便做字节序转换
    Union4Con.cha[1]   =   datagram[5];
    Union4Con.cha[2]   =   datagram[4];
    Union4Con.cha[3]   =   datagram[3];

    char lenbuffer[4];
    char cmdbuffer[2];

    lenbuffer[0]=datagram[3];
    lenbuffer[1]=datagram[4];
    lenbuffer[2]=datagram[5];
    lenbuffer[3]=datagram[6];

    int datalen = lenbuffer[3]          //直接对char类型进行位移操作会导致目的内存都被1填充，导致datalen不正确
                  + (lenbuffer[2] << 8)
                  + (lenbuffer[1] << 16)
                  + (lenbuffer[0] << 24);
    datalen =   Union4Con.i;    //可以直接当int型读取
    int recbufflen  =   datagram.length();
    cmdbuffer[0]=datagram[2];


//    char databuffer;
//    databuffer.

//      int lendata = datagram.length()-7;
//    int len;

//    if((datagram.at(0)=='D')&&(datagram.at(1)=='X'))
    if((datagram[0]=='D')&&(datagram[1]=='X'))
    {

        if(cmdbuffer[0]==0x00)
        {
            if(datagram.length()-7==datalen)
            {
                datagram.remove(0,7);
            }
            else
                return;
        }
        else
            return;

    }
    else
        return;

 //   QString strMesg="OK";
 //   tcpSocket->write(strMesg.toStdString().c_str(),strlen(strMesg.toStdString().c_str())); //发送
 //   tcpSocket->waitForBytesWritten(1000);
//    tcpSocket->close();

    QBuffer buffer(&datagram);
    buffer.open(QIODevice::WriteOnly);
//    QImage save_img(img_weigth,img_heigth,QImage::Format_RGB32);
    QImage save_img(StaticValue::GetInstance()->str_ipaimageW.toInt(),
                    StaticValue::GetInstance()->str_ipaimageH.toInt(),
                    QImage::Format_RGB32);
    temp_img=save_img;
    last_image_array=datagram;

    bool res = temp_img.loadFromData(datagram,"bmp");
    receive_image=temp_img;

//    QString str_name = "D:/pictures/";
    QString str_name = "";

    QDateTime time_curr = QDateTime::currentDateTime();//获取系统现在的时间
    QString str_curr = time_curr.toString("yyMMddhhmmss"); //设置显示格式
//    str_curr+=".bmp";

    str_name = str_name + str_curr + ".bmp";
    QFile filename(str_name);
 //   QFile filename("D:\pictures\20160723111.bmp");
    if (!filename.open(QIODevice::ReadWrite))
        return;

//    QImage转为QPixmap，再用QPixmap的save函数就可以了
//    QPixmap receive_pixmap;
    receive_pixmap.save(&buffer,"bmp");
//    receive_pixmap.save(datagram,"bmp");
//    receive_image.save(&buffer, "bmp");
//    receive_image.save(&filename,"bmp");
    filename.write(datagram);
    filename.flush();
    filename.close();
    last_image_filename=str_name;
    ba.clear(); //清空接收图片缓存的byteArray，等待下次接收
}


/*
void MainWindow::displayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpSocket->errorString(); //输出错误信息
}

void MainWindow::desConect()
{

}
*/
void MainWindow::on_setpicture_triggered()
{
//    #define  img_heigth    192
//    #define  img_weigth    192

    IpadImageSetDialog imageset_dialog;
   imageset_dialog.setModal(true);
 //   draw_dialog.showFullScreen();
   imageset_dialog.exec();

}

//void MainWindow::on_ontimesetclick()
//{
//   OnTimeSetDialog ontimeset_dialog;
//   ontimeset_dialog.setModal(true);
// //   draw_dialog.showFullScreen();
//   ontimeset_dialog.exec();
//}
void MainWindow::on_Query_triggered()
{
    AddqueDialog addque_dialog;
    addque_dialog.setModal(true);
    addque_dialog.exec();
}




void MainWindow::on_actionRemotecontrol_triggered()
{
    RemoteDlg dlg(&rcstatus_);
    if(dlg.exec()==QDialog::Accepted)
    {
        UpdateStatusBar();
    }
}

void MainWindow::ElecDevItemClicked(int j)
{
    //QMessageBox::information(this,"友情提示",QString("%1").arg(i));  // 薛定谔的代码？解除注释有惊喜
    LoopDo64Device();		//组织DO64的数据包并输出
    LoopDmx512Device();		//组织DMX512的数据包并输出
}

void MainWindow::on_action11_18_triggered()
{
//    playtiming pt;
//    pt.exec();
//    GetTaskFromHTTP();
//    QMessageBox::information(this,"info","you just click about button!");
}

void MainWindow::on_actionjust4mobile_triggered()
{
    QMessageBox::information(this,"info","click mobile!");
}

void MainWindow::on_GetMission_triggered(bool checked)
{
    if(checked)
    {
        HTTPtimer_->start(1000);
    }
    else
    {
        HTTPtimer_->stop();
    }
}


void MainWindow::on_ontimeset_triggered(bool checked)
{
    if(timingtimer_==nullptr)
    {
        timingtimer_    =   new QTimer;
        connect(timingtimer_,&QTimer::timeout,this,&MainWindow::timingtimer);
    }
    if(checked)
    {
        timingtimer_->start(1000);
    }
    else
    {
        timingtimer_->stop();
    }
}

//定时播控制核心代码，包括基本定时任务起始、中止时间判断，以及定时播放任务的播放模式功能
void MainWindow::timingtimer()
{
    QDateTime curDateTime   =    QDateTime::currentDateTime();
    if(timingplay_)                     //正在播放定时任务
    {
        if(curDateTime>timingend_)
        {
			on_stop_progrem_body();
            ui->GetMission->setChecked(false);
            on_GetMission_triggered(false);

            ui->action_DRAW->setChecked(false);
            on_action_DRAW_triggered(false);

            timingplay_ =   false;
        }
    }
    else                                //闲置状态
    {
        if(playtiming::timinglist_.contains(curDateTime.date()))        //当天是否有播放任务
        {
            QList<timinginfo> timingofday   =   playtiming::timinglist_.values(curDateTime.date());
            foreach (timinginfo tinfo, timingofday)
            {
                if(curDateTime>=tinfo.start_&&curDateTime<=tinfo.end_)
                {
                    //准备播放定时任务
                    timingplay_ =   true;
                    timingend_  =   tinfo.end_;
					on_stop_progrem_body();        //先停止一波再说

                    QMapIterator<int,MusicInfo*> itr(StaticValue::GetInstance()->m_music_info);
                    while(itr.hasNext())
                    {
                        itr.next();
                        if(itr.value()->name==tinfo.musicname_)
                        {
                            emit tree_view->leftDoubleClicked(TreeType::COMP_DEVICE,itr.key());
                            on_start_triggered();
                            if(tinfo.ControlMode_=="ipad控制")
                            {
                                on_action_DRAW_triggered(true);
                            }
                            else if(tinfo.ControlMode_=="手机控制")
                            {
                                ui->GetMission->setChecked(true);
                                on_GetMission_triggered(true);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}

void MainWindow::on_timinglist_triggered()
{
    playtiming pt;
    pt.exec();
}

void MainWindow::on_action_DRAW_triggered(bool checked)
{
    int i =ipad_port;

//	QTextCodec *codec=QTextCodec::codecForName("UTF-8");
//	QTextCodec::setCodecForLocale(codec);

    //以下代码为原来打算用PC绘图弹出的绘图对话框的代码，在苹果发送模式下屏蔽
        /*
             DrawTableDialog draw_dialog;
             draw_dialog.setModal(true);
             draw_dialog.showFullScreen();
             draw_dialog.exec();
        */

        //ipad发送模式下修改为：点击该按钮，则绘图由苹果IPAD完成，IPAD  send SOCKET
        //同时禁用编曲开始跑动的按钮（该功能是否需要？）
        //再次点击，恢复为播放表演模式，不再接收ipad发送的消息
        //设立一个flag，用以判断是什么模式工作：m_CtrlModeFlag,0是自主播放表演，1是由ipad发送图片，缺省初始化为0

            if(!checked)
            {
                m_CtrlModeFlag=0;
				ui->action_DRAW->setText(tr("绘图互动"));
    //            ui->start->setEnabled(1);
                if(server->isListening())
                 {
                    server->close();
                  //  m_pEdt_Info->append(QObject::tr("停止监听！"));

                 }

            }
            else
            {
     //           m_CtrlModeFlag=1;
				ui->action_DRAW->setText(tr("已选择绘图模式"));
      //        ui->start->setEnabled(0);
                if (!server->isListening())
                {
                    bool listen_res =   server->listen(QHostAddress::Any, ipad_port);
                    if (listen_res)
                    {
                  //      m_pEdt_Info->append(QObject::tr("打开监听端口成功！"));
//                        QMessageBox::information(this, tr("提示"),
//                                                 tr("打开监听端口成功！"), QMessageBox::Ok);
                    }
                    else
                    {
                  //      m_pEdt_Info->append(QObject::tr("打开监听端口失败！"));
                        QMessageBox::information(this, tr("提示"),
                                                     tr("打开监听端口失败！"), QMessageBox::Ok);

                    }
                }
                else
                {
                 // m_pEdt_Info->append(QObject::tr("正在监听中...！"));
                }
            }
}


void MainWindow::on_heartbeatsetting_triggered(bool checked)
{
    if(checked)
    {
        HeartBeat_->start(1000);
    }
    else
    {
        HeartBeat_->stop();
    }
}

void MainWindow::handlePingResult(double percentage)
{
    qDebug() << "Ping 通的百分比:" << percentage;
	QString percentageText = QString("设备ping通率: %1%").arg(percentage, 0, 'f', 2);
	pingstatus_->setText(percentageText);
}

