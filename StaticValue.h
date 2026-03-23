#ifndef STATICVALUE_H
#define STATICVALUE_H

#include <QObject>
#include <QMutex>
#include "ClassDefined.h"
#include <QMap>
#include "ElecDeviceRectItem.h"
#include "CollidingRectItem.h"
#pragma execution_character_set("utf-8")
class CollidingRectItem;

class StaticValue : public QObject
{
    Q_OBJECT
public:
    explicit StaticValue(QObject *parent = 0);

    static StaticValue* GetInstance();

    void Parse();
    QString getShowTeamList(ShowTeam* show_team);
    void ZeroMemoryEx(char*,int);
public:
	void updateMusicPath(int musicId, const QString &newMusicPath); //用于更新数据库，当音乐不存在或者换机器了

private:
    //以下方法均为从数据库中读取信息，映射到程序缓存中
    void InitSysConfig();
    void getShowTamInfo();        //获取表演编队信息
    void getDeviceInfo();         //获取设备信息
    void getOutPutDevice();       //获取输出设备信息
    void getShowOrbit();          //获取表演轨迹信息
    void getMusicInfo();          //获取编曲信息
    void getMusicAction();        //获取编曲动作信息
    void getMusicMapping();       //获取编曲映射信息
    void getRouteInfo(int music_action_id);  //获取表演动作的通道信息
    void InitDeviceState();    //初始化所有的电气阀设备为0
    void InitDMXState();       //初始化所有的灯光通道为0
    void getRouteMapping();    //获取所有通道号和设备号的映射

    void readImageSet();        //获得IPAD设定图片尺寸

    QString getLinear(ShowTeam *show_team);   //获取线性编队分组
    QString getCustomDefine(ShowTeam *show_team);  //获取用户自定义分组
    QString getOpposeTeams(ShowTeam *show_team);
    QString getMirrorImageTeams(ShowTeam *show_team);
    QString getRepeakTeams(ShowTeam *show_team);


signals:

public slots:

public:
    static StaticValue* m_Instance;
    QMap<int,DeviceInfo*> m_device_map;				//输出阀信息
    QMultiMap<int,int> m_route_mapping;				//设备号-通道号1:3
    QMap<int,OutPutDevice*> m_output_device;		//输出设备
    QMap<int,ShowTeam*> m_show_team;				//表演编队
    QMap<int,MusicAction*> m_music_action;			//所有编曲的所有表演动作，根据id查找编队和编曲
    QMap<int,Orbit*> m_show_orbit;					//轨迹
    QMap<int,MusicInfo*> m_music_info;				//编曲列表
    QMap<int,ElecDeviceRectItem*> m_elecDeviceMap;	//电气设备
    QMap<int,CollidingRectItem*> m_MusicActions;	//正在播放的表演动作列表
    QMap<int,int> m_device_state;					//电气设备的状态(所有电气设备的值，设备号-值)
    QMap<QString,QString> m_SysConfig;				//系统配置文件
    //int TWIST_IMAGE;

    QString str_ipaimageW;				//ipad图片数据的宽度
    QString str_ipaimageH;				//ipad图片数据的高度
    QString str_ipaimageTimes;			//ipad图片数据的总时长？
    QString str_ipaimageDelayTime;		//ipad图片数据的间隔时间


};

#endif // STATICVALUE_H
