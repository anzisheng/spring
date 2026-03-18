#ifndef MUSICWIDGET_H
#define MUSICWIDGET_H

#include <QWidget>
#include "MusicGraphicsView.h"
#include <QGraphicsLineItem>
#include <QGraphicsItemAnimation>
#include <QPropertyAnimation>
#include "Refactor/Audio/AudioPlayer.h"

#include "ClassDefined.h"
#include "StaticValue.h"
#include "lineitem.h"
#include "spectrumwidget.h"


// STL
#include <vector>
#include <mutex>
#include <random>

#include "./ext/FMOD/inc/fmod.h"
#include "./ext/FMOD/inc/fmod.hpp"
#include "./ext/FMOD/inc/fmod_errors.h"
#include "globalparams.h"
#include "./Model/Track/track.h"
#pragma execution_character_set("utf-8")

class MusicWidget : public QWidget
{
	Q_OBJECT	
public:
	explicit MusicWidget(QWidget *parent = 0);
	void setTimeLength(float time_length);
	float getTimeLength();
	void setMusicName(QString name);
	void setMusicID(int id);
	int  getMusicID();
	int  getType();
	QString getMusicName();

	void InitParamer(MusicInfo* music_info);
	void InitData(MusicInfo* music_info); //init GraphicsView
	void addItem(int show_team_id);  //add show_team
	void InitTimeLine();             //init bottom time line

	void start();
	void resume();
	void stop();
	void rewind();
	void pointertime();

	void saveMusic();
    void savecutlines(); //add by eagle for 固定线存储
    void saveCutSolidLines(const QVector<CutSolidVerticalLine*> &lines, const QString &filename);
	QVector<CutSolidVerticalLine*> loadCutSolidLines(const QString &filename);
	QMap<int, CollidingRectItem*> getMusicActions();  //获取所有的表演动作
	void UpdataTime(int currenttime);
	QTimeLine* m_timeline;
	//add by eagle for update music position
	QTimer *mediatimer;
	void updatePositionplay();

private:
	QString second2Minute(float second);
	void InitShowTeam(MusicInfo *music_info);
	void InitMusicActions(QList<int> action_id, MyItem* parent);
	void applyMediaPositionFromLine(qint64 position, bool autoPlay);

protected:
	void paintEvent(QPaintEvent *);


	void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
signals:
	void deleteAction();
	void controler();
	void resetStartTime(int begintTime);
	void TimelineFinished();
signals:
	void signaldrawLineAtPosition(float xPosition); //发送播放位置
	void signalsetmovelinefocus(bool bset);
	void signalsinitlinedata();
public slots:

public:
	int id;                //编曲在数据库中的ID
	float time_length;     //当前编曲时长
	int m_HeaderWidth;
	float m_balance;       //每一秒多少像素
	int m_EditView;
//add by eagle for close event for stop music
protected:
    void closeEvent(QCloseEvent *event) override;
//add for close event end
private:

	int m_NextLinePosY;
	int type;              //编曲的类型
	QString name;          //编曲的名称

	QString music_path;    //音乐文件路径

	int m_PartNum;         //时间轴分多少段

	int m_lineHeight;


	MusicGraphicsView* m_MusicView;
	QGraphicsScene* sceneBase;

	LineItem* m_line; //add by eagle 这个很像时间线了. 也有时间，待Debug后确定.

	QGraphicsItemAnimation* m_animation;
	QPropertyAnimation *m_animationex;


	QMap<int, MyItem*> m_show_team;
	QMap<int, CollidingRectItem*> m_MusicActions;   //当前编曲所有的表演动作
	//add by eagle for some API check 
	bool checkfile(QString szfile);
	//add 
	//add by eagle for music play and stop and pause
public:
	    void playtrack(void);
	    void stoptrack(void);
	    void pausetrack(bool issetpause);
	    bool              bIsSomeTrackPlaying;
	    void playFromPosition(qint64 position); //实现任意位置播放

	AudioPlayer *m_audioPlayer; //播放音乐
	qint64 pausedPosition; //当前播放位置. 可以考虑用它同步上下位置.
	qint64 m_duration; //音乐总时长，毫秒
	bool bisplayinit; //判断是不是已经播放了，停止以后设置为false， 初始化为false，只要被暂停或者playtrack都是True
	bool spaceKeyPressed; // 按键信息判断
	
	qint64 m_musicpos; //每次虚线停止的音乐位置
public slots:
		
private slots:
	void onMediaStatusChanged(AudioPlayer::PlaybackState state); //此部分用于设置播放点，为虚线的位置
	void onMediaEndStatusChanged(AudioPlayer::MediaStatus status); //音乐结束需要另外的消息响应
	void onPositionChanged(qint64 position); //这个参数只会通过消息传递出来，并且是动态更新，暂停之后更新这个位置.
	void onAudioDurationChanged(qint64 duration);
	void slotsetmediaposition(qint64 position);
	void slotmousesetmediaposition(qint64 position);


    //
public:
	void addWidgetBelow(spectrumwidget *spwidget);
	spectrumwidget* m_spWidget;
	//add end
	//add by eagle for music data process
	FMOD::System*     pSystem;
	bool FMODinit();

	// FX
	FMOD::DSP*        pPitch;
	FMOD::DSP*        pPitchForTime;
	FMOD::DSP*        pReverb;
	FMOD::DSP*        pEcho;
	bool              bFMODStarted;

	///
	// Oscillogram  drawing
	std::mutex        mtxDrawGraph;
	std::mutex        mtxGetCurrentDrawingIndex;
	size_t*           iCurrentlyDrawingTrackIndex;
	bool              bDrawing;
	void drawGraph(size_t* iTrackIndex);


    float             fCurrentVolume;
    int  iCurrentlyPlayingTrackIndex;
	float             fCurrentSpeedByPitch;
	float             fCurrentSpeedByTime;

	// Tracks
	std::vector<Track*> vTracks;				// 通过这个处理文件，但是不处理历史记录。因为每个widget绑定一个音乐
	// std::vector<Track*> vTracksHistory;
	bool   addTrack(const std::wstring& sFilePath);
	std::wstring getTrackName(const std::wstring& pFilePath);
    unsigned int getMusicLengthbypath(QString szpath); //获取长度，单独初始化释放，这个需要测试是否影响波形初始化和释放，COM应该不影响


	// Used in drawGraph()
	float* rawBytesToPCM16_0_1(char* pBuffer, unsigned int iBufferSizeInBytes);
	float* rawBytesToPCM24_0_1(char* pBuffer, unsigned int iBufferSizeInBytes);
	int    interpret24bitAsInt32(char byte0, char byte1, char byte2);

	void addDataToGraph(float *pData, unsigned int iSizeInSamples, unsigned int iSamplesInOne);
	void testaction();
	unsigned int iCurrentXPosOnGraph;
signals:
	void signalAddDataToGraph(float* pData, unsigned int iSizeInSamples, unsigned int iSamplesInOne); //发送信号，驱动处理画线Slot
	void signalMaxspec(unsigned int iTempMax);

public slots:
	void slotAddDataToGraph(float* pData, unsigned int iSizeInSamples, unsigned int iSamplesInOne); //处理画线数据
	void slotMaxspec(unsigned int iTempMax);
	//add by eagle for music data process

};

#endif // MUSICWIDGET_H
