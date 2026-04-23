#include "MusicWidget.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QTimeLine>
#include <QEasingCurve>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QScrollArea>


//STL
#include <string>
#include <thread>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>

#include <QDebug>
#include <QThread>
#include <QMetaType>

#include "./Model/Track/track.h"

MusicWidget::MusicWidget(QWidget *parent) :
QWidget(parent)
{
	m_PartNum = 20;
	m_lineHeight = 30;
	m_NextLinePosY = 0;
	m_HeaderWidth = 150;
	m_musicpos = 0;
	m_duration = 0;
	pausedPosition = 0;
	pSystem = nullptr;
	m_spWidget = nullptr;
	m_audioPlayer = new AudioPlayer(this);
	this->time_length = 1;
	if (music_path.isEmpty() == false){
		time_length = getMusicLengthbypath(music_path);
	}

	m_MusicView = new MusicGraphicsView(this);
	sceneBase = new QGraphicsScene(m_MusicView);
	sceneBase->setBackgroundBrush(QColor(238, 242, 247));
	m_MusicView->setScene(sceneBase);
	m_MusicView->setAlignment(Qt::AlignLeft | Qt::AlignTop);//对齐坐标系 add by eagle 
	connect(m_MusicView, SIGNAL(deleteAction()), this, SIGNAL(deleteAction()));
	qRegisterMetaType<float*>("float*");
	connect(this, SIGNAL(signalMaxspec(unsigned int)), this, SLOT(slotMaxspec(unsigned int)), Qt::QueuedConnection); //add by eagle update music data,更新最大振幅
	connect(this, SIGNAL(signalAddDataToGraph(float*, unsigned int, unsigned int)),
		this, SLOT(slotAddDataToGraph(float*, unsigned int, unsigned int)), Qt::QueuedConnection);
	connect(this, &MusicWidget::signaldrawLineAtPosition, m_MusicView, &MusicGraphicsView::drawVerticalLine);
	connect(this, &MusicWidget::signalsetmovelinefocus, m_MusicView, &MusicGraphicsView::slotsetmovelinefocus);
	connect(this, &MusicWidget::signalsinitlinedata, m_MusicView, &MusicGraphicsView::slotinitlinedata); //初始化虚线 -- 参考线
	connect(m_MusicView, &MusicGraphicsView::signalsetmediaposition, this, &MusicWidget::slotsetmediaposition);
	connect(m_MusicView, &MusicGraphicsView::signalmousesetmediapos, this, &MusicWidget::slotmousesetmediaposition);
	//m_MusicView->slotinitlinedata(); 这里以后会优化，就是默认参考线是第一个画出来的
	//emit signalsinitlinedata();
	//QScrollArea *scrollArea = new QScrollArea(this);
	m_MusicView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	//设置垂直滚动条
	m_MusicView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	//设定水平滚动条
	//mediatimer = new QTimer(this);
	//mediatimer->setTimerType(Qt::PreciseTimer);
	//connect(mediatimer, &QTimer::timeout, this, &MusicWidget::updatePositionplay);
	//mediatimer->start(5); // Update every 100 milliseconds
	//add by eagle for spectrum

	bFMODStarted = false; //判定音乐是不是开始，实际上这个变量最后没有用到

	// FX
	pPitch = nullptr;
	pPitchForTime = nullptr;
	pReverb = nullptr;
	pEcho = nullptr;

	fCurrentVolume = DEFAULT_VOLUME;
	iCurrentlyPlayingTrackIndex = 0;
	fCurrentSpeedByPitch = 1.0f;
	fCurrentSpeedByTime = 1.0f;

	FMODinit();
	m_audioPlayer->setSystem(pSystem);
	QObject::connect(m_audioPlayer, &AudioPlayer::durationChanged, this, &MusicWidget::onAudioDurationChanged);
	QObject::connect(m_audioPlayer, &AudioPlayer::positionChanged, this, &MusicWidget::onPositionChanged);
	QObject::connect(m_audioPlayer, &AudioPlayer::playbackStateChanged, this, &MusicWidget::onMediaStatusChanged);
	QObject::connect(m_audioPlayer, &AudioPlayer::mediaStatusChanged, this, &MusicWidget::onMediaEndStatusChanged);
	// Draw new oscillogram.
	iCurrentlyDrawingTrackIndex = new size_t;
	(*iCurrentlyDrawingTrackIndex) = 0;
	iCurrentXPosOnGraph = 0;
	bDrawing = false;

//	MusicWidget* music_info = (MusicWidget*)this->parent();
//	InitParamer(music_info);

	QTimer::singleShot(1000, [&]() {
		qDebug() << "Function executed after 5 seconds";
	//	if(music_info->id)
		if (getType())			//0:程控，1：音乐
			testaction();		//音乐类型的才需要绘制音乐波形

		// Simulate pressing the Enter key - 用于画虚线初始化
		QKeyEvent *enterEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
		QCoreApplication::postEvent(this, enterEvent);
		QString szfilename = getMusicName();
		QString exePath = QCoreApplication::applicationDirPath();
		QString szfullname = exePath + "/data/" + szfilename + ".glw";
		m_MusicView->setCutSolidLines(szfullname);							//从glw文件读取竖直红线位置数据
		emit signaldrawLineAtPosition(0);
		m_MusicView->setmovableVerticalLinetime(getTimeLength(), 1, 100);	//设置参考线音乐总长度
	});
	//add by eagle for musci spectrum end==============
	spaceKeyPressed = false;
	bisplayinit = false;
}


void MusicWidget::setTimeLength(float time_length)
{
	this->time_length = time_length;
}

float MusicWidget::getTimeLength()
{
	return this->time_length;
}

void MusicWidget::setMusicName(QString name)
{
	this->name = name;
}

void MusicWidget::setMusicID(int id)
{
	this->id = id;
}

int MusicWidget::getMusicID()
{
	return id;
}

int MusicWidget::getType()
{
	return type;
}

QString MusicWidget::getMusicName()
{
	return name;
}


void MusicWidget::InitParamer(MusicInfo* music_info)
{
	this->type = music_info->type;
	this->id = music_info->id;
	this->name = music_info->name;
	this->time_length = music_info->time_length;
	this->music_path = music_info->music_path;
}


void MusicWidget::InitShowTeam(MusicInfo *music_info)
{
	QMultiMap<int, int> team_actions = music_info->team_actions;
	QList<int> keys = team_actions.uniqueKeys();
	for (int i = 0; i < keys.count(); ++i)
	{
		int key = keys.at(i);
		addItem(key);
		QList<int> values = team_actions.values(key);
		MyItem* parent = m_show_team[key];
		if (parent)
			InitMusicActions(values, parent);
	}
}

void MusicWidget::InitMusicActions(QList<int> actions, MyItem* parent)
{
	for (int i = 0; i < actions.size(); ++i)
	{
		int action_id = actions.at(i);
		if (action_id != -1)  //增加的编队信息，还没有加入表演动作的时候,就不加
		{
			MusicAction* music_action = StaticValue::GetInstance()->
				m_music_action[action_id];
			if (music_action)
			{
				float begin_time = music_action->begin_time;
				float end_time = music_action->end_time;
				qreal width = (end_time / 1000.0 - begin_time / 1000.0)* m_balance;
				qreal width2 = (end_time - begin_time)*(m_EditView - 30.0) / time_length / 1000.0;
				qreal height = 20.0;
				qreal pos_x = m_HeaderWidth + begin_time / 1000.0 * m_balance;
				qreal pos_x1 = begin_time*(m_EditView - 30.0) / time_length / 1000.0 + m_HeaderWidth;
				qreal pos_y = m_NextLinePosY - 5.0 - height;
				QRectF action_rect(pos_x1, pos_y, width2, height);
				m_MusicView->InitMusicActions(action_id, action_rect, begin_time, end_time, parent);
			}
		}
	}
}

void MusicWidget::InitData(MusicInfo* music_info)
{
	InitParamer(music_info);
	m_EditView = this->width() - m_HeaderWidth;
	qDebug() << "InitData this->width()" << this->width();
//	m_balance = 1.0*(m_EditView-30) / this->time_length;

	int maxx_temp = (m_EditView - 10.0) / 2.0 - 10.0;		//临时获得maxx
	m_balance = 1.0*maxx_temp / this->time_length;			//应该以0-maxx为基准，maxx=(m_EditView-10) / 2 - 10;

//	m_balance = 1.0*(m_EditView-10) / this->time_length;	//实际宽度为1450-30？

	//设置编曲区域？
	m_MusicView->resize(QSize(this->width() - 30, this->height() * 6 / 7));		//编曲部分占6/7，音乐波形占1/7，边框是30，不是10，这样区域就成了0-1570
//	m_MusicView->setSceneRect(0, 0, this->width() - 0, m_MusicView->height());	//30是右边的空白部分，暂时是固定值
	m_MusicView->setSceneRect(0, 0, m_MusicView->width(), m_MusicView->height());	//30是右边的空白部分，暂时是固定值

	//编曲头
	QGraphicsRectItem* title_rect = sceneBase->addRect(
		QRect(0, m_NextLinePosY, this->width() - 30, m_lineHeight),
		QPen(Qt::black), QBrush(QColor(252, 251, 224)));

	QFont font;
	font.setPixelSize(20);		//字体大小
	//编曲名
	QGraphicsTextItem* title_text =
		new QGraphicsTextItem(this->name, title_rect);
	title_text->setFont(font);
	m_NextLinePosY += m_lineHeight;		//指向下一行

	QGraphicsRectItem* header = sceneBase->addRect(
		QRect(0, m_NextLinePosY, m_HeaderWidth, m_lineHeight),
		QPen(Qt::black), QBrush(QColor(181, 170, 232)));

	QGraphicsTextItem* device_name =
		sceneBase->addText(tr("编队aaa名"));
	device_name->setFont(font);
	device_name->setPos(0, m_NextLinePosY);
	device_name->setParentItem(header);

	sceneBase->addRect(m_HeaderWidth, m_NextLinePosY,
		m_EditView - 30, m_lineHeight,
		QPen(Qt::black), QBrush(QColor(181, 170, 232)));
	m_NextLinePosY += m_lineHeight;

	QLineF time_line(m_HeaderWidth, m_NextLinePosY,
		m_HeaderWidth, this->width() - 30);
	QPen time_pen(QBrush(QColor(Qt::blue)), 2);
	m_line = new LineItem;
	m_line->setLine(time_line);
	sceneBase->addItem(m_line);
	m_line->SetScene(sceneBase);
	//    m_line = sceneBase->addLine(time_line,time_pen);
	sceneBase->setFocusItem(m_line);
	m_line->setZValue(2);
	m_line->setFlag(QGraphicsItem::ItemIsMovable);

	//改变参数就可以控制播放线跑的快慢，值越大跑的越慢
	qreal temp = this->width();
	temp = (this->time_length) * 1000.0 * (1 - 30.0 / (1.0*(this->width() -150.0 - 30.0)));
//	m_timeline = new QTimeLine((this->time_length+3) * 1000); //add by eagle for time play position 5ms ， 如果是10ms，这里不考虑长度就的考虑下面的播放帧
//	m_timeline = new QTimeLine(this->time_length * 1040); //add by eagle for time play position 5ms ， 如果是10ms，这里不考虑长度就的考虑下面的播放帧
	m_timeline = new QTimeLine(this->time_length * 1000);

	qDebug() << " this->width()" << this->width();
	int nlineFixed = this->width();
	//m_timeline->setFrameRange(m_HeaderWidth, this->width()-30); // QTimeLine 对象的 setFrameRange() 方法的参数。该方法用于设置动画的帧范围。
//	int temp = this->width();
//	temp = (this->time_length) * 1000 * (1 - 30 / (1.0*(this->width() - 30)));
//	m_timeline->setFrameRange(0, this->time_length * 1000 * (1- 30/1.0*(this->width() - 30)));
	m_timeline->setFrameRange(0, this->time_length * 1000);

	m_timeline->setLoopCount(1);
//	m_timeline->setUpdateInterval(20);
	m_timeline->setEasingCurve(QEasingCurve::Linear); //设置匀速
//	m_timeline->setCurrentTime(0);
	connect(m_timeline, SIGNAL(finished()), this, SLOT(TimelineFinished()));

	m_animation = new QGraphicsItemAnimation(this);
	m_animation->setItem(m_line);
	m_animation->setTimeLine(m_timeline);
	
	for (int i = 0; i < (m_EditView - 30); ++i)
		m_animation->setPosAt(1, QPointF(i, 0)); //方法用于设置动画对象在特定时间点的位置, 在每次迭代时都将动画对象的位置设置为1（动画结束时）
		//m_animation->setPosAt(i * 1.0 / (m_EditView - 30), QPointF(i, 0));
		//m_animation->setPosAt(i * 2.0 / (m_EditView - 30), QPointF(i, 0));
		//m_animation->setPosAt(i * 0.5 / (m_EditView - 30), QPointF(i, 0));
	//{
		//这里对上了，这个和定时器还是有关系的，取决于#define TIME_RES	5		//定时器的分辨率＝10毫秒   
		// 这个定时器，决定了这个更新的频率，如果加长时间，这里就的考虑m_timeline = new QTimeLine(this->time_length * 1000); //add by eagle for time play position
		// 这里的长度*2 或者 下面的progress /2 因为播放帧多少取决于时间， 比如10s 跑5次 和 10次 这里是不同的

		//qreal progress = static_cast<qreal>(i) / ((m_EditView - 30)/2); // 这里让播放线的时间和时间线严格对上。//10ms
		//qreal progress = static_cast<qreal>(i*1.00) / ((nlineFixed - 30)); // 这里让播放线的时间和时间线严格对上。//5ms
		//m_animation->setPosAt(progress, QPointF(i, 0));
		//qDebug() << "progress" << progress;

	//}
	
	//初始化表演编队
	qDebug() << "this might be the width" << m_EditView;
	qDebug() << "this might be the time_length" << time_length;

	InitShowTeam(music_info);
}

void MusicWidget::InitTimeLine()
{
	QPainter painter(this);
	painter.setPen(QPen(QBrush(QColor(Qt::white)), 1));
	//left line
	//    QPointF begin_line_top(m_HeaderWidth,m_MusicView->height());
	//    QPointF begin_line_bottom(m_HeaderWidth,this->height());

	float Adjust = 0;

	//音乐波形起始线？
	QPointF begin_line_top(m_HeaderWidth*1.0 + Adjust, 8);
	QPointF begin_line_bottom(m_HeaderWidth*1.0 + Adjust, 0);
	painter.drawLine(begin_line_top, begin_line_bottom);

	//end line，音乐波形终止线？
	QPointF end_line_top(this->width()*1.0 - 20.0-Adjust, m_MusicView->height());
	QPointF end_line_bottom(this->width()*1.0 - 20.0 - Adjust, this->height());
	painter.drawLine(end_line_top,end_line_bottom);

	//time line
	//画音乐波形上面的20段时间标尺横线
	int timeline_Y = m_MusicView->height();                 //调整时间轴高度 2018.9.5
	QPointF begin_pos(m_HeaderWidth*1.0 + Adjust, timeline_Y*1.0);
	QPointF end_pos(this->width()*1.0 - 20.0 - Adjust, timeline_Y*1.0);
	painter.drawLine(begin_pos, end_pos);

	//每段的像素距离
	float distance = (end_line_top.x() - begin_line_top.x())*1.0 / (m_PartNum*1.0);
	//每段距离代表多少秒
	float sub_time_line = (this->time_length*1.0) / (m_PartNum*1.0);
	float beginSecond = 0;
//	画音乐波形上面的20段时间标尺21根竖线
	for (int i = 0; i <= m_PartNum; ++i)
	{
		QPointF top_pos(m_HeaderWidth + distance*i + Adjust, timeline_Y - 10);
		QPointF bottom_pos(m_HeaderWidth + distance*i+Adjust, timeline_Y + 10);
		painter.drawLine(top_pos, bottom_pos);
	//	beginSecond += sub_time_line;
		QString qStrText = second2Minute(beginSecond);
		painter.setPen(QPen(QBrush(QColor(Qt::white)), 1));
		bottom_pos.setY(bottom_pos.y() + 10);
		bottom_pos.setX(bottom_pos.x() - 42);
		painter.drawText(QPointF(bottom_pos), qStrText);
		//qDebug() << qStrText; //add by eagle for 这里的逻辑是

		beginSecond += sub_time_line;
	}
}

QString MusicWidget::second2Minute(float second)
{
	//会有浮点值
//	int minNum = second / 60;
//	float secondNum = second - minNum * 60;
//	QString qStrTime = QString("%1:%2").arg(minNum).arg(secondNum);
//	return qStrTime;

	//会有浮点值
	int minNum = second / 60;
	float secondNum = second - minNum * 60;
	//QString qStrTime = QString("%1:%2").arg(minNum).arg(secondNum);
	QString qStrTime = QString("%1:%2").arg(minNum).arg(QString::number(secondNum, 'f', 3));
	return qStrTime;
}

//add show_team
void MusicWidget::addItem(int show_team_id)
{
	QMap<int, ShowTeam*> show_teams = StaticValue::GetInstance()->m_show_team;
	if (show_teams.contains(show_team_id))
	{
		QString qStrTeamName = QString::number(show_team_id) + "." +
			show_teams[show_team_id]->name;
		QRectF team_rect(m_HeaderWidth, m_NextLinePosY,
			m_EditView - 30, m_lineHeight); //220,252,252
		MyItem* team_item = new MyItem(team_rect, QColor(255, 0, 0, 0));//编队名称后面这一长条的底色
		sceneBase->addItem(team_item);
		team_item->setShowTeam(show_team_id);
		m_show_team[show_team_id] = team_item;

		MyItem* header_item = new MyItem(QRect(0, m_NextLinePosY,        //198,197,239
			m_HeaderWidth, m_lineHeight),
			QColor(0, 255, 0));//编队名称的底色
		sceneBase->addItem(header_item);
		header_item->setParentItem(team_item);

		QGraphicsTextItem* device_name = sceneBase->addText(qStrTeamName);
		device_name->setPos(0, m_NextLinePosY);
		device_name->setParentItem(team_item);

		m_NextLinePosY += m_lineHeight;
	}
}

void MusicWidget::start()
{
	m_timeline->start();
	m_MusicView->setPlayState(true);
	//这里增加开始播放音乐
	if (getType())			//0:程控，1：音乐
		playtrack();
	signalsetmovelinefocus(true); //add by eagle for 参考线焦点， 这里不太合乎规则，具体应该是在control里面发singnals
}


void MusicWidget::resume()
{
	m_timeline->resume();
	m_MusicView->setPlayState(true);
	//signalsetmovelinefocus(false); //add by eagle for 参考线焦点， 这里不太合乎规则，具体应该是在control里面发singnals
}


void MusicWidget::stop()
{
	if (QTimeLine::Running == m_timeline->state())
	{
		m_timeline->stop();
		m_MusicView->setPlayState(false);
		if (getType())			//0:程控，1：音乐
			stoptrack();
	}
}

void MusicWidget::rewind()
{
	//    m_timeline->setCurrentTime(30000);
	//    m_timeline->setStartFrame(200);
	m_timeline->setFrameRange(100, 500);
}

void MusicWidget::pointertime()
{
	QTime curtime = QTime::fromMSecsSinceStartOfDay(m_timeline->currentTime());
	QMessageBox::information(this, "时间提醒", QString("%1秒").arg(curtime.second()));
}

void MusicWidget::saveMusic()
{
	QVector<CollidingRectItem *> items = m_MusicView->getCollidingItems();
	int iCount = items.size();
	for (int i = 0; i < iCount; ++i)
	{
		CollidingRectItem *item = items.at(i);
		QPointF scene_pos = item->scenePos();
		QRectF rect = item->boundingRect();
		qreal pos_x = rect.x() + scene_pos.x();
		qreal width = rect.width();

		float begin_old = item->begin_time;
		float end_old = item->end_time;
		//老式的时间区间算法
		float begin_time1 = (pos_x - m_HeaderWidth) / m_balance * 1000;
		float end_time1 = begin_time1 + width / m_balance * 1000;

		//修正后的时间区间算法
		float begin_time = (pos_x - m_HeaderWidth)*time_length / (m_EditView - 30.0);
		float end_time = (pos_x + width - m_HeaderWidth)*time_length / (m_EditView - 30.0);
		begin_time *= 1000.0;
		end_time *= 1000.0;

		int music_action_id = item->getID();
		QSqlQuery query;
		QString qStrSql = QString("update music_action set begin_time = %1,"
			"end_time = %2 where id = %3").
			arg(begin_time).arg(end_time).arg(music_action_id);
		if (!query.exec(qStrSql))
		{
			qDebug() << "update music_action false:sql:" <<
				qStrSql << "error:" << query.lastError().text();
		}
		else
		{
			item->setTimes(begin_time, end_time);
		}
	}

}

//存储竖直红线
void MusicWidget::savecutlines()
{
	//add by eagle for 实现存储
	QString szfilename = getMusicName();
	QVector<CutSolidVerticalLine*> getcutitems = m_MusicView->getcutsolidlines();
	QString exePath = QCoreApplication::applicationDirPath();
	QString szfullname = exePath + "/data/" + szfilename + ".glw";

	//QVector<CutSolidVerticalLine*> loadedLines = loadCutSolidLines(szfullname); // 提供两个加载函数，目前都还没有使用
	//m_MusicView->setCutSolidLines(szfullname); //  需要使用这种

	if (getcutitems.size() >= 3)
	{
		saveCutSolidLines(getcutitems, szfullname);
	}

}

void MusicWidget::saveCutSolidLines(const QVector<CutSolidVerticalLine *> &lines, const QString &filename)
{
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly)) 
	{
		QDataStream out(&file);
		out.setVersion(QDataStream::Qt_5_6);
		//out << lines; Qt 5.6.3 c++ 不支持这种写法
		int size = lines.size();
		out << size;
		for (int i = 0; i < size; i++)
		{
			CutSolidVerticalLine* line = lines.at(i);
		//	out << line->getX() << line->gety1() << line->gety2();	//原来是存，x,y1,2
			out << line->gettime();			//现在只存时间，单位毫秒
		}
	}
}

QVector<CutSolidVerticalLine *> MusicWidget::loadCutSolidLines(const QString &filename)
{
	QVector<CutSolidVerticalLine*> lines;
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly)) 
	{
		QDataStream in(&file);
		in.setVersion(QDataStream::Qt_5_6);
		int size;
		in >> size;
		for (int i = 0; i < size; i++) 
		{
			qreal x, y1, y2,time;
		//	in >> x >> y1 >> y2;
			in >> time;

			x = (time/time_length)*(this->width() - m_HeaderWidth);
			y1= 30*2;
			y2= this->height();
			CutSolidVerticalLine* line = new CutSolidVerticalLine(x, y1, y2,time);
			lines.append(line);
		}
	}
	return lines;
}

void MusicWidget::paintEvent(QPaintEvent *painter)
{
	InitTimeLine();
}

void MusicWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Space)
	{
		//float i = (pausedPosition*1.0 / m_duration) * 100;
		//int ml = time_length;

		qDebug() << "escape";
		emit controler();
		spaceKeyPressed = true;
		//逻辑空格按键后，显示参考线（虚线）为焦点，左右移动，回车后进入.
		//add by eagle  for  music task 虚线位置开始播放音乐
	}
	else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
	{
		qDebug() << "Enter 键盘===========";
		float i = (pausedPosition*1.0 / m_duration) * 100;
		emit signaldrawLineAtPosition(i);
		//	spaceKeyPressed = false;
		//}
	}
}

void MusicWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		QPoint press_point = event->pos();
		int pos_x = press_point.x();
	//	int start_timer = 0;
	//	if (pos_x < m_HeaderWidth)
	//		start_timer = 0;
	//	else
	//	{
	//		start_timer = (pos_x - m_HeaderWidth) / m_balance * 1000;
	//		qDebug() << start_timer;
	//	}
	//	emit resetStartTime(start_timer);

	}
}

void MusicWidget::closeEvent(QCloseEvent *event)
{
	stoptrack();
	if (m_MusicView)
	{
		m_MusicView->setPlayState(false);
	}
	
}

bool MusicWidget::checkfile(QString szfile)
{
	QString musicFilePath = szfile;
	// Check if the file exists
	QFile file(musicFilePath);
	bool fileExists = file.exists();

	if (fileExists) {
		// The file exists, proceed with your code
		return true;
	}
	else {
		// The file does not exist, handle the situation accordingly
		return false;
	}
}

void MusicWidget::playtrack()
{
	//FMOD_RESULT result = pSystem->update();
	//unsigned int iTrackOldPos = vTracks[iCurrentlyPlayingTrackIndex]->getPositionInMS();
	//vTracks[iCurrentlyPlayingTrackIndex]->setPositionInMS(iTrackOldPos);
	//FMOD_RESULT result = pSystem->update();
	//vTracks[iCurrentlyPlayingTrackIndex]->playTrack(fCurrentVolume);	
	m_audioPlayer->setSource(music_path);
	m_audioPlayer->setPlaybackRate(1.0); // 设置速播放
	m_audioPlayer->play();
	bisplayinit = true;

}

void MusicWidget::stoptrack()
{
	if (!vTracks.empty())
	{
		vTracks[0]->stopTrack();
	}
	m_audioPlayer->stop();
}
void MusicWidget::updatePositionplay()
{
	qint64 position1 = m_audioPlayer->position();
	if (position1 > 0){
		pausedPosition = position1;

		qreal progress = static_cast<qreal>(pausedPosition) / m_duration;
		qreal startPosition = m_HeaderWidth;
		qreal endPosition = this->width() - 30;
		qreal currentX = progress * (endPosition - startPosition);
		m_line->setPos(QPointF(currentX, m_line->y()));
	}
}
void MusicWidget::onPositionChanged(qint64 position)
{
	//这里一定要加这个判断，有时候这里会返回0
	
	if (position > 0){
		
		this->pausedPosition = position;
		m_musicpos = position;
		if (m_line)
		{
			m_line->currenttime_ = static_cast<int>(position);
			m_line->update();
		}
	}
}

void MusicWidget::onAudioDurationChanged(qint64 duration)
{
	qDebug() << tr("音乐总时长:") << duration;
	m_duration = duration;
}

void MusicWidget::applyMediaPositionFromLine(qint64 position, bool autoPlay)
{
	qint64 szpos = position;		//position是m_x的值
	qint64 pos = 1.0 * (szpos - m_HeaderWidth) / m_balance * 1000;
	m_musicpos = pos;
	qint64 linepos = pos;

	qDebug() << tr("slotsetmediaposition") << m_musicpos;
	m_audioPlayer->setPosition(m_musicpos);
	if (autoPlay)
	{
		m_audioPlayer->pause();
		m_audioPlayer->play();
	}

	m_timeline->setCurrentTime(static_cast<int>((linepos)));
	emit resetStartTime(m_musicpos);
}

void MusicWidget::slotsetmediaposition(qint64 position)
{
	applyMediaPositionFromLine(position, true);
}

void MusicWidget::slotmousesetmediaposition(qint64 position)
{
	//这里获取的值*2 才能和实际的坐标对上，很奇怪，系数2 不是非常精确 要计算一下
	// 这里存在一个缓冲误差，这个误差怎么算出来，还有待思考, 526ms, 理论上不应该存在这个误差，旧播放链路缓存会更大
	//这里动态处理一下, 如果是短音乐 百分之三的误差， 如果是大与2分钟的音乐误差 百分之3.5
	float posdeviation = 0.03; //0.035 有一点点大，根据缓冲区大小换算了 
	if (this->time_length > 2 * 60) //2分钟
	{
		posdeviation = 0.035;
	}
	if (this->time_length > 15 * 60) //15分钟
	{
		posdeviation = 0.045;
	}
	if (this->time_length > 30 * 60) //35分钟
	{
		posdeviation = 0.03;
	}
	qint64 szpos = position;
	qint64 pos = 1.0 * (szpos - m_HeaderWidth) / m_balance * 1000;
	m_musicpos = pos * (2 + posdeviation);
	qDebug() << tr("slotsetmediaposition") << m_musicpos;

	m_audioPlayer->setPosition(m_musicpos);
	m_audioPlayer->pause();
	
}
void MusicWidget::onMediaStatusChanged(AudioPlayer::PlaybackState state) {

	if (state == AudioPlayer::PausedState) {
		qint64 currentPos = m_audioPlayer->position();
		if (currentPos > 0)
		{
			m_musicpos = currentPos;
		}
	}
	qDebug() << tr("MusicWidget::onMediaStatusChanged");
}

void MusicWidget::onMediaEndStatusChanged(AudioPlayer::MediaStatus status)
{
	if (status == AudioPlayer::EndOfMedia)
	{
		m_audioPlayer->stop();
		bisplayinit = false;
	}
}
void MusicWidget::pausetrack(bool issetpause)
{
	if (issetpause)
	{
		qint64 currentPos = m_audioPlayer->position();
		if (currentPos > 0)
		{
			m_musicpos = currentPos;
		}
		m_audioPlayer->pause();
		if (m_musicpos > 0)
		{
			m_audioPlayer->setPosition(m_musicpos);
		}
	}
	else
	{
		if (bisplayinit == false)
		{
			playtrack();
		}
		m_audioPlayer->play();
	}

	//vTracks[0]->pauseHandleTrack(issetpause);
	//FMOD_RESULT result;
	//result = pSystem->update();
}

void MusicWidget::playFromPosition(qint64 position)
{
	m_audioPlayer->setPosition(position);
	m_audioPlayer->play();
}

void MusicWidget::addWidgetBelow(spectrumwidget* spWidget)
{
	m_spWidget = spWidget;
	if (m_spWidget != nullptr)
	{
		m_spWidget->setLabelWidth(m_HeaderWidth);
	}
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addStretch(); // 添加一个弹性空间，将 widget 推到底部
	//主要目的减少左边的空隙 Add a left margin to the layout
	int leftMargin = 0; // Adjust this value to your desired margin
	layout->setContentsMargins(leftMargin, 0, 0, 0);

	layout->addWidget(spWidget);
	this->setLayout(layout);
}

bool MusicWidget::FMODinit()
{
	// FMOD initialization
	FMOD_RESULT result;
	result = FMOD::System_Create(&pSystem);
	if (result)
	{
		//pMainWindow->showMessageBox(true, std::string("AudioService::AudioService::FMOD::System_Create() failed. Error: ") + std::string(FMOD_ErrorString(result)));
		//pMainWindow->showMessageBox(false, "The audio system has not been started and the application will be closed.");
		// Look main.cpp (isSystemReady() function)
		// app will be closed.
		//pMainWindow->markAnError();
		return true;
	}

	result = pSystem->init(MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr);
	if (result)
	{
		//pMainWindow->showMessageBox(true, std::string("AudioService::AudioService::FMOD::System::init() failed. Error: ") + std::string(FMOD_ErrorString(result)));
		//pMainWindow->showMessageBox(false, "The audio system has not been started and the application will be closed.");
		// Look main.cpp (isSystemReady() function)
		// app will be closed.
		//pMainWindow->markAnError();
		return true;
	}

	result = pSystem->update();
	if (result)
	{
		//pMainWindow->showMessageBox(true, std::string("AudioService::AudioService::FMOD::System::update() failed. Error: ") + std::string(FMOD_ErrorString(result)));
		//pMainWindow->showMessageBox(false, "The audio system has not been started and the application will be closed.");
		// Look main.cpp (isSystemReady() function)
		// app will be closed.
		//pMainWindow->markAnError();
		return true;
	}

	// FX

	FMOD::ChannelGroup* pMaster;
	pSystem->getMasterChannelGroup(&pMaster);


	result = pSystem->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &pPitch);
	if (result)
	{
		// pMainWindow->showMessageBox(true, std::string("AudioService::AudioService::FMOD::System::createDSPByType() failed. Error: ") + std::string(FMOD_ErrorString(result)));
		pPitch = nullptr;
	}
	else
	{
		pPitch->setParameterFloat(FMOD_DSP_PITCHSHIFT_FFTSIZE, 4096);
		pPitch->setBypass(true);
		pMaster->addDSP(0, pPitch);
	}



	result = pSystem->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &pPitchForTime);
	if (result)
	{
		//pMainWindow->showMessageBox(true, std::string("AudioService::AudioService::FMOD::System::createDSPByType() failed. Error: ") + std::string(FMOD_ErrorString(result)));
		pPitchForTime = nullptr;
	}
	else
	{
		pPitchForTime->setParameterFloat(FMOD_DSP_PITCHSHIFT_FFTSIZE, 4096);
		pPitchForTime->setBypass(true);
		pMaster->addDSP(1, pPitchForTime);
	}


	result = pSystem->createDSPByType(FMOD_DSP_TYPE_SFXREVERB, &pReverb);
	if (result)
	{
		//pMainWindow->showMessageBox(true, std::string("AudioService::AudioService::FMOD::System::createDSPByType() failed. Error: ") + std::string(FMOD_ErrorString(result)));
		pReverb = nullptr;
	}
	else
	{
		pMaster->addDSP(3, pReverb);
		pReverb->setParameterFloat(FMOD_DSP_SFXREVERB_WETLEVEL, -80.0f);
		pReverb->setBypass(true);
	}


	result = pSystem->createDSPByType(FMOD_DSP_TYPE_ECHO, &pEcho);
	if (result)
	{
		//pMainWindow->showMessageBox(true, std::string("AudioService::AudioService::FMOD::System::createDSPByType() failed. Error: ") + std::string(FMOD_ErrorString(result)));
		pEcho = nullptr;
	}
	else
	{
		pMaster->addDSP(4, pEcho);
		pEcho->setParameterFloat(FMOD_DSP_ECHO_DELAY, 1000);
		pEcho->setParameterFloat(FMOD_DSP_ECHO_WETLEVEL, -80.0f);
		pEcho->setBypass(true);
	}

	pMaster->setPan(0.0f);


	bFMODStarted = true;
	return false;
}

void MusicWidget::drawGraph(size_t *iTrackIndex)
{
	mtxDrawGraph.lock();
	bDrawing = true;
	// default: 16384
	// "If FMOD_TIMEUNIT_RAWBYTES is used, the memory allocated is two times the size passed in, because fmod allocates a double buffer."
	// 131072 * 2 = 256 kB (x8 times bigger than default) to speed up our graph draw (to speed up our readData() calls)
	FMOD_RESULT fresult = pSystem->setStreamBufferSize(131072, FMOD_TIMEUNIT_RAWBYTES);

	if (fresult)
	{
		//pMainWindow->showMessageBox(true, std::string("AudioService::AudioService::FMOD::System::setStreamBufferSize() failed. Error: ") + std::string(FMOD_ErrorString(fresult)));
	}
	//pMainWindow->clearGraph();
	// m_spWidget->ui->widget_graph->graph(0)->data()->clear(); 这里清理一下图形，暂时可以忽略

	// this value combines 'iOnlySamplesInOneRead' samples in one to store less points for graph in memory
	// more than '200' on a track that is about 5 minutes long looks bad
	// for example 2.5 min track with 'iSamplesInOne' = 100, adds like 3 MB to RAM
	// but less value can fill RAM very bad
	unsigned int iOnlySamplesInOneRead = 150;
	// so we calculate 'iSamplesInOne' like this:
	// 3000 ('iSamplesInOne') - 6000 (sec.)
	// x    ('iSamplesInOne') - track length (in sec.)
	mtxGetCurrentDrawingIndex.lock();
	// here we do: 3000 * (tracks[iTrackIndex]->getLengthInMS() / 1000) / 6000, but we can replace this with just:
	//iOnlySamplesInOneRead = vTracks[*iTrackIndex]->getLengthInMS() * 3 / 6000;
	iOnlySamplesInOneRead = vTracks[*iTrackIndex]->getLengthInMS() / 2000;
	if (iOnlySamplesInOneRead == 0) iOnlySamplesInOneRead = 1;
	// Set max on graph
	std::string format = vTracks[*iTrackIndex]->getPCMFormat();
	unsigned int iTempMax;
	if (format == "PCM16")
	{
		iTempMax = vTracks[*iTrackIndex]->getLengthInPCMbytes() / 4 / iOnlySamplesInOneRead;
	}
	else
	{
		iTempMax = vTracks[*iTrackIndex]->getLengthInPCMbytes() / 6 / iOnlySamplesInOneRead;
	}

	// pMainWindow->setXMaxToGraph(iTempMax);
	emit signalMaxspec(iTempMax);
	//  ui->widget_graph->xAxis->setRange(0, iMaxX);
	//  m_spWidget->SetXMaxToGraph(iTempMax);
	vTracks[*iTrackIndex]->setMaxPosInGraph(iTempMax);

	vTracks[*iTrackIndex]->createDummySound();
	mtxGetCurrentDrawingIndex.unlock();

	// Buffer Size = 2 MB
	unsigned int iBufferSize = 2097152;
	unsigned int iGraphMax = 0;
	char pcmFormat = 0;
	char result = 1;

	do
	{
		char* pSamplesBuffer = new char[iBufferSize];
		unsigned int iActuallyReadBytes;
		// Read
		mtxGetCurrentDrawingIndex.lock();
		result = vTracks[*iTrackIndex]->getPCMSamples(pSamplesBuffer, iBufferSize, &iActuallyReadBytes, &pcmFormat);
		mtxGetCurrentDrawingIndex.unlock();
		// Check for errors
		if (result == 0)
		{
			delete[] pSamplesBuffer;
			break;
		}
		else if ((result == -1) && (iActuallyReadBytes == 0))
		{
			delete[] pSamplesBuffer;
			break;
		}
		// Convert to proper massive format
		if (pcmFormat == 16)
		{
			// PCM16
			if (iActuallyReadBytes % 4 != 0)
			{
				// Wil probably appear on end of file
				while (iActuallyReadBytes % 4 != 0)
				{
					iActuallyReadBytes--;
				}
			}
			float* pSamples = rawBytesToPCM16_0_1(pSamplesBuffer, iActuallyReadBytes);
			// Delete raw buffer
			delete[] pSamplesBuffer;
			// Send to Main Window new buffer
			iGraphMax += iActuallyReadBytes / 4 / iOnlySamplesInOneRead;
			//add by eagle for draw kernal logic
			addDataToGraph(pSamples, iActuallyReadBytes / 2, iOnlySamplesInOneRead);
		}
		else
		{
			// PCM24
			if (iActuallyReadBytes % 6 != 0)
			{
				// Wil probably appear on end of file
				while (iActuallyReadBytes % 6 != 0)
				{
					iActuallyReadBytes--;
				}
			}
			float* pSamples = rawBytesToPCM24_0_1(pSamplesBuffer, iActuallyReadBytes);
			// Delete raw buffer
			delete[] pSamplesBuffer;

			// Send to Main Window new buffer
			iGraphMax += iActuallyReadBytes / 6 / iOnlySamplesInOneRead;
			//add by eagle for kernel research
			addDataToGraph(pSamples, iActuallyReadBytes / 3, iOnlySamplesInOneRead);
		}

	} while ((result == 1) && (bDrawing));


	mtxGetCurrentDrawingIndex.lock();
	if (bDrawing)
	{
		// pMainWindow->setXMaxToGraph(iGraphMax);
		emit signalMaxspec(iTempMax);
		vTracks[*iTrackIndex]->setMaxPosInGraph(iGraphMax);
	}
	vTracks[*iTrackIndex]->releaseDummySound();
	mtxGetCurrentDrawingIndex.unlock();
	// set to default: 16384
	fresult = pSystem->setStreamBufferSize(16384, FMOD_TIMEUNIT_RAWBYTES);
	if (fresult)
	{
		// pMainWindow->showMessageBox(true, std::string("AudioService::AudioService::FMOD::System::setStreamBufferSize() failed. Error: ") + std::string(FMOD_ErrorString(fresult)));
	}
	bDrawing = false;
	mtxDrawGraph.unlock();

}

bool MusicWidget::addTrack(const std::wstring &sFilePath)
{
	Track* pNewTrack = new Track(sFilePath, getTrackName(sFilePath), pSystem);
	if (!pNewTrack->setupTrack())
	{
		delete pNewTrack;
		return true;
	}

	std::wstring wPathStr(sFilePath);


	// Get track name

	std::wstring trackName = getTrackName(sFilePath);



	// Get track info
	std::wstring trackInfo = L"";
	for (size_t i = wPathStr.size() - 4; i < wPathStr.size(); i++)
	{
		if (wPathStr[i] != L'.')
		{
			trackInfo += wPathStr[i];
		}
	}
	trackInfo += L", ";
	trackInfo += std::to_wstring(static_cast<int>(pNewTrack->getFrequency()));
	trackInfo += L"hz";
	int channels, bits;
	if (pNewTrack->getChannelsAndBits(&channels, &bits))
	{
		trackInfo += L", channels: ";
		trackInfo += std::to_wstring(channels);
		trackInfo += L", ";
		trackInfo += std::to_wstring(bits);
		trackInfo += L" bits";
	}
	trackInfo += L", ";
	std::wstringstream stream;
	stream << std::fixed << std::setprecision(2) << pNewTrack->getFileSizeInBytes() / 1024.0 / 1024;
	trackInfo += stream.str();
	trackInfo += L" MB";



	// Get track time
	std::string trackTime = "";
	unsigned int iMS = pNewTrack->getLengthInMS();
	unsigned int iSeconds = iMS / 1000;
	unsigned int iMinutes = iSeconds / 60;
	iSeconds -= (iMinutes * 60);
	trackTime += std::to_string(iMinutes);
	trackTime += ":";
	if (iSeconds < 10) trackTime += "0";
	trackTime += std::to_string(iSeconds);

	pNewTrack->setSpeedByFreq(fCurrentSpeedByPitch);
	pNewTrack->setSpeedByTime(fCurrentSpeedByTime);

	vTracks.push_back(pNewTrack);

	// pMainWindow->addNewTrack(trackName, trackInfo, trackTime);

	if (vTracks.size() == 1)
	{
		//bMonitorTracks = true;
		// std::thread monitor(&AudioService::monitorTrack, this);
		// monitor.detach();
	}

	FMOD_RESULT result = pSystem->update();
	if (result)
	{
		return true;
	}

	return false;
}
std::wstring MusicWidget::getTrackName(const std::wstring& pFilePath)
{
	std::wstring wPathStr(pFilePath);

	size_t iNameStartIndex = 0;
	for (size_t i = wPathStr.size() - 1; i >= 1; i--)
	{
		if (wPathStr[i] == L'/' || wPathStr[i] == L'\\')
		{
			iNameStartIndex = i + 1;
			break;
		}
	}

	std::wstring trackName = L"";
	for (size_t i = iNameStartIndex; i < wPathStr.size() - 4; i++)
	{
		trackName += wPathStr[i];
	}
	if (trackName.back() == L'.') trackName.pop_back();


	return trackName;
}

unsigned int MusicWidget::getMusicLengthbypath(QString szpath)
{
	//return 200;
	unsigned int length = 0;
	FMOD::System *system;
	FMOD::Sound *sound;
	FMOD_RESULT result;


	// Create FMOD system instance
	result = FMOD::System_Create(&system);

	// Initialize FMOD system
	result = system->init(32, FMOD_INIT_NORMAL, nullptr);

	// Create sound object and load music file
	result = system->createSound(szpath.toStdString().c_str(), FMOD_DEFAULT, 0, &sound);

	// Get the length of the sound object
	result = sound->getLength(&length, FMOD_TIMEUNIT_MS);

	// Release resources
	sound->release();
	system->close();
	system->release();

	// Convert length from milliseconds to seconds
	length = length;

	return length;
}
//切换编曲时，需要将当前编曲的所有表演动作加入到待表演列表中，方便在timer中遍历表演
QMap<int, CollidingRectItem*> MusicWidget::getMusicActions()
{
	QMap<int, CollidingRectItem*> music_actions;
	QVector<CollidingRectItem*> items = m_MusicView->getCollidingItems();
	int iSize = items.size();
	for (int i = 0; i < iSize; ++i)
	{
		CollidingRectItem* item = items.at(i);
		int id = item->getID();
		music_actions[id] = item;
	}
	return music_actions;
}
//tag by eagle for time change 
void MusicWidget::UpdataTime(int currenttime)
{
	m_line->currenttime_ = currenttime;
	//emit signaldrawLineAtPosition(m_line->currentxpos_); //delete by eagle for  time postion. 这里的对齐并不准，不采用时间比算距离
	//qDebug() << "MusicWidget::UpdataTime currenttime" << currenttime;
}

float *MusicWidget::rawBytesToPCM16_0_1(char *pBuffer, unsigned int iBufferSizeInBytes)
{
	float* pSamples = new float[iBufferSizeInBytes / 2];

	size_t iPosInSamples = 0;

	for (size_t i = 0; i < iBufferSizeInBytes; i += 4)
	{
		short int iSampleL = 0;

		memcpy(reinterpret_cast<char*>(&iSampleL), &pBuffer[i], 1);
		memcpy(reinterpret_cast<char*>(&iSampleL) + 1, &pBuffer[i + 1], 1);

		//unsigned short int iSampleLUnsigned = static_cast<unsigned short int> (iSampleL + USHRT_MAX / 2.0f);

		//pSamples[iPosInSamples] = static_cast<float>(iSampleLUnsigned) / USHRT_MAX;
		pSamples[iPosInSamples] = static_cast<float>(iSampleL) / SHRT_MAX;
		iPosInSamples++;



		short int iSampleR = 0;

		memcpy(reinterpret_cast<char*>(&iSampleR), &pBuffer[i + 2], 1);
		memcpy(reinterpret_cast<char*>(&iSampleR) + 1, &pBuffer[i + 3], 1);

		//unsigned short int iSampleRUnsigned = static_cast<unsigned short int> (iSampleR + USHRT_MAX / 2.0f);

		//pSamples[iPosInSamples] = static_cast<float>(iSampleRUnsigned) / USHRT_MAX;
		pSamples[iPosInSamples] = static_cast<float>(iSampleR) / SHRT_MAX;
		iPosInSamples++;
	}

	return pSamples;
}

float *MusicWidget::rawBytesToPCM24_0_1(char *pBuffer, unsigned int iBufferSizeInBytes)
{
	float* pSamples = new float[iBufferSizeInBytes / 3];

	size_t iPosInSamples = 0;

	int iPow2To24 = pow(2, 24);

	for (size_t i = 0; i < iBufferSizeInBytes; i += 6)
	{
		int iSampleL = interpret24bitAsInt32(pBuffer[i + 2], pBuffer[i + 1], pBuffer[i]);

		//unsigned int iSampleLUnsigned = static_cast<unsigned int> (iSampleL + iMin24BitSigned);

		//pSamples[iPosInSamples] = static_cast<float>(iSampleLUnsigned / pow(2, 24));
		pSamples[iPosInSamples] = static_cast<float>(iSampleL) / iPow2To24;
		iPosInSamples++;



		int iSampleR = interpret24bitAsInt32(pBuffer[i + 5], pBuffer[i + 4], pBuffer[i + 3]);

		//unsigned int iSampleRUnsigned = static_cast<unsigned int> (iSampleR + iMin24BitSigned);

		//pSamples[iPosInSamples] = static_cast<float>(iSampleRUnsigned / pow(2, 24));
		pSamples[iPosInSamples] = static_cast<float>(iSampleR) / iPow2To24;
		iPosInSamples++;
	}


	return pSamples;
}

int MusicWidget::interpret24bitAsInt32(char byte0, char byte1, char byte2)
{
	// copy-paste from stackoverflow
	return ((byte0 << 24) | (byte1 << 16) | (byte2 << 8)) >> 8;
}

void MusicWidget::addDataToGraph(float *pData, unsigned int iSizeInSamples, unsigned int iSamplesInOne)
{
	emit signalAddDataToGraph(pData, iSizeInSamples, iSamplesInOne);
}

void MusicWidget::testaction()
{
	// Get the path to the executable
	//QString exePath = QCoreApplication::applicationDirPath();

	// Construct the path to the MP3 file in the "music" folder
	QString musicFilePath = music_path; // QDir(exePath).filePath("music/1.mp3");
	if (checkfile(musicFilePath) == false){
		//QMessageBox::warning(nullptr, "文件检测不到", "音乐文件不存在或者路径无效,音乐波形不绘制.");		
		if (m_spWidget != nullptr)
		{
			m_spWidget->setLabel(tr("音乐文件不存在或者路径无效，音乐波形不绘制。"));
		}
		return;
	}
	else
	{
		if (m_spWidget != nullptr)
		{
			QString szlabel = tr("音乐：%1\n时长：%2 秒")
				.arg(getMusicName())
				.arg(QString::number(getTimeLength()));
			m_spWidget->setLabel(szlabel);
		}
	}
	std::wstring filePath = musicFilePath.toStdWString();

	addTrack(filePath);
	if (!bDrawing)
	{
		std::thread drawGraphThread(&MusicWidget::drawGraph, this, iCurrentlyDrawingTrackIndex);
		drawGraphThread.detach();
	}
}

void MusicWidget::slotMaxspec(unsigned int iTempMax)
{
	if (m_spWidget != nullptr)
	{
		m_spWidget->setslotMaxspec(iTempMax);
	}
}
void MusicWidget::slotAddDataToGraph(float *pData, unsigned int iSizeInSamples, unsigned int iSamplesInOne)
{
	//add by eagle for Draw mp3 spectrum

	QVector<double> x;
	QVector<double> yLeft;
	QVector<double> yRight;

	// Here we need to show only 'iSamplesInOne' samples out of all 'iSizeInSamples'.

	//                                            here '2' because 2 channels (TODO: get rid of this constant value),
	//                                            and '-1' because we read two values in one cycle.
	for (unsigned int i = 0; i < iSizeInSamples - (2 * iSamplesInOne - 1); i += (2 * iSamplesInOne))
	{
		x.push_back(static_cast<double>(iCurrentXPosOnGraph));
		iCurrentXPosOnGraph++;

		float fLeftPlus = -1.0f;
		float fLeftMinus = 1.0f;
		float fRightPlus = -1.0f;
		float fRightMinus = 1.0f;
		bool bLeftPlusFirst = false;
		bool bRightPlusFirst = false;

		// Average all 'iSamplesInOne' samples in one sample.
		for (unsigned int j = 0; j < (2 * iSamplesInOne); j += 2)
		{
			// L + R channels.
			float fLeft = pData[i + j];
			float fRight = pData[i + j + 1];

			if (j == 0)
			{
				bLeftPlusFirst = (fLeft > 0.0f);
				bRightPlusFirst = (fRight > 0.0f);
			}

			if (fLeft > 0.0f)
			{
				if (fLeft > fLeftPlus)
				{
					fLeftPlus = fLeft;
				}
			}
			else
			{
				if (fLeft < fLeftMinus)
				{
					fLeftMinus = fLeft;
				}
			}

			if (fRight > 0.0f)
			{
				if (fRight > fRightPlus)
				{
					fRightPlus = fRight;
				}
			}
			else
			{
				if (fRight < fRightMinus)
				{
					fRightMinus = fRight;
				}
			}
		}

		fLeftPlus *= 0.5f;
		fLeftPlus = 0.5f + fLeftPlus;

		fLeftMinus *= 0.5f;
		fLeftMinus += 0.5f;

		fRightPlus *= 0.5f;
		fRightPlus = 0.5f + fRightPlus;

		fRightMinus *= 0.5f;
		fRightMinus += 0.5f;

		if (bLeftPlusFirst)
		{
			yLeft.push_back(static_cast<double>(fLeftPlus));
		}
		else
		{
			yLeft.push_back(static_cast<double>(fLeftMinus));
		}

		if (bRightPlusFirst)
		{
			yRight.push_back(static_cast<double>(fRightPlus));
		}
		else
		{
			yRight.push_back(static_cast<double>(fRightMinus));
		}
	}

	qDebug() << "x:" << x.size() << "yL:" << yLeft.size() << "yR:" << yRight.size();
	if (m_spWidget != nullptr)
	{
		m_spWidget->addWaveData(x, yLeft, yRight);
	}

	delete[] pData;
}
