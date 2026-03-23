#include "StaticValue.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include "CommonFile.h"
#include "QFileHelper.h"
#include <QMessageBox>
StaticValue* StaticValue::m_Instance = NULL;

StaticValue::StaticValue(QObject *parent) :
    QObject(parent)
{

}

StaticValue* StaticValue::GetInstance()
{
    if(m_Instance == NULL)
        m_Instance = new StaticValue();
    return m_Instance;
}

void StaticValue::Parse()   //从数据库获取所有的数据到软件缓存中
{
    InitSysConfig();
    getShowTamInfo();
    getOutPutDevice();		//卡信息
    getDeviceInfo();		//map和延迟值的映射
    getRouteMapping();		//好像是DMX的通道对应表？
    getShowOrbit();
    getMusicInfo();
    getMusicAction();
    getMusicMapping();
    InitDeviceState();
    readImageSet();
}

QString StaticValue::getRepeakTeams(ShowTeam* showTeam)
{
    QList<int> team_id;
    int minNum = showTeam->minNum;
    int maxNum = showTeam->maxNum;

    if(showTeam->isReverse)
    {
        for(int i = maxNum; i >= minNum; --i)
        {
            team_id.push_back(i);
        }
    }
    else
    {
        for(int i = minNum; i <= maxNum; ++i)
        {
            team_id.push_back(i);
        }
    }

    int iArrayNum = showTeam->arrayNum; //分组个数
    int iSize = team_id.size();     //水阀个数
    if(showTeam->isRound)
        iSize *= 2;
    int rowNum = iSize/iArrayNum;   //每行的个数

    QStringList show_teams;
    QStringList sub_team;
    for(int i = 0 ; i < team_id.size(); ++i)
    {
        if(show_teams.size() == iArrayNum)
        {
            QString begin_string = show_teams[0];
            begin_string += ",";
            begin_string += QString::number(team_id.at(i));
            show_teams[0] = begin_string;
        }
        else
        {
            if(sub_team.size() == rowNum)
            {
                QString qStrSubNum = sub_team.join(",");
                show_teams.push_back(qStrSubNum);
                sub_team.clear();
            }
            sub_team.push_back(QString::number(team_id.at(i)));
        }
    }

    if(!sub_team.isEmpty())
    {
        QString qStrSubNum = sub_team.join(",");
        show_teams.push_back(qStrSubNum);
    }

    if(showTeam->isRound)
    {
        QStringList qStrTemp = show_teams;
        int iSize = qStrTemp.size();
        for(int i = iSize-1; i >= 0; --i)
        {
            QString sub_team = qStrTemp.at(i);
            show_teams.push_back(sub_team);
        }
    }

    return show_teams.join("\n");
}

QString StaticValue::getMirrorImageTeams(ShowTeam* showTeam)
{
    QList<int> team_id;
    int minNum = showTeam->minNum;
    int maxNum = showTeam->maxNum;
    if(showTeam->isReverse)
    {
        for(int i = maxNum; i >= minNum; --i)
        {
            team_id.push_back(i);
        }
    }
    else
    {
        for(int i = minNum; i <= maxNum; ++i)
        {
            team_id.push_back(i);
        }
    }

    int iArrayNum = showTeam->arrayNum; //分组个数
    int iSize = team_id.size();     //水阀个数
    if(showTeam->isRound)
        iSize *= 2;
    int rowNum = iSize/iArrayNum;   //每组的个数
    QStringList show_teams;
    QStringList sub_team;
    for(int i = 0,j = team_id.size()-1; i <= j; ++i,--j)
    {
        if(show_teams.size() == iArrayNum)
        {
            QString begin_string = show_teams[0];
            begin_string += ",";
            begin_string += QString::number(team_id.at(i));
            begin_string += ",";
            begin_string += QString::number(team_id.at(j));
            show_teams[0] = begin_string;
        }
        else
        {
            if(sub_team.size() == rowNum)
            {
                QString qStrSubNum = sub_team.join(",");
                show_teams.push_back(qStrSubNum);
                sub_team.clear();
            }
            sub_team.push_back(QString::number(team_id.at(i)));
            sub_team.push_back(QString::number(team_id.at(j)));
        }

    }
    if(!sub_team.isEmpty())
    {
        QString qStrSubNum = sub_team.join(",");
        show_teams.push_back(qStrSubNum);
    }
    if(showTeam->isRound)
    {
        QStringList qStrTemp = show_teams;
        int iSize = qStrTemp.size();
        for(int i = iSize-1; i >= 0; --i)
        {
            QString sub_team = qStrTemp.at(i);
            show_teams.push_back(sub_team);
        }
    }
    return show_teams.join("\n");
}

QString StaticValue::getOpposeTeams(ShowTeam* showTeam)
{
    QList<int> team_id;
    int minNum = showTeam->minNum;
    int maxNum = showTeam->maxNum;
    if(showTeam->isReverse)
    {
        for(int i = maxNum; i >= minNum; --i)
        {
            team_id.push_back(i);
        }
    }
    else
    {
        for(int i = minNum; i <= maxNum; ++i)
        {
            team_id.push_back(i);
        }
    }
    int iArrayNum = showTeam->arrayNum; //分组个数
    int iSize = team_id.size();     //水阀个数
    if(showTeam->isRound)  //往返
        iSize *= 2;
    int rowNum = iSize/iArrayNum;   //每组的个数
    QStringList show_teams;
    QStringList sub_team;
    for(int i = team_id.size()/2-1,j = team_id.size()/2; i >= 0,j<=team_id.size()-1; --i,++j)
    {
        if(show_teams.size() == iArrayNum)
        {
            QString begin_string = show_teams[0];
            begin_string += ",";
            begin_string += QString::number(team_id.at(i));
            begin_string += ",";
            begin_string += QString::number(team_id.at(j));
            show_teams[0] = begin_string;
        }
        else
        {
            if(sub_team.size() == rowNum)
            {
                QString qStrSubNum = sub_team.join(",");
                show_teams.push_back(qStrSubNum);
                sub_team.clear();
            }
            sub_team.push_back(QString::number(team_id.at(i)));
            sub_team.push_back(QString::number(team_id.at(j)));
        }

    }
    if(!sub_team.isEmpty())
    {
        QString qStrSubNum = sub_team.join(",");
        show_teams.push_back(qStrSubNum);
    }
    if(showTeam->isRound) //往返
    {
        QStringList qStrTemp = show_teams;
        int iSize = qStrTemp.size();
        for(int i = iSize-1; i >= 0; --i)
        {
            QString sub_team = qStrTemp.at(i);
            show_teams.push_back(sub_team);
        }
    }
    return show_teams.join("\n");
}

QString StaticValue::getLinear(ShowTeam* show_team)
{
    QString qStrTeams("");
    if(SHOW_REPEAK == show_team->repeckType)    //重复
    {
        qStrTeams = getRepeakTeams(show_team);
    }
    else if(SHOW_MIRRORIMAGE == show_team->repeckType)  //镜像
    {
        qStrTeams = getMirrorImageTeams(show_team);
    }
    else if(SHOW_OPPOSEMIRROR == show_team->repeckType)   //反向镜像
    {
        qStrTeams = getOpposeTeams(show_team);
    }
    return qStrTeams;
}

QString StaticValue::getCustomDefine(ShowTeam* show_team)
{
    return show_team->showTeamList;
}


QString StaticValue::getShowTeamList(ShowTeam* show_team)
{
    QString team_info;
    if(show_team->rowType == 0)  //线性
    {
        team_info = getLinear(show_team);
    }
    else  //自定义
    {
        team_info = getCustomDefine(show_team);
    }

    return team_info;
}

void StaticValue::ZeroMemoryEx(char *pBuffer,int len)
{
    for(int i = 0; i < len; ++i)
    {
        pBuffer[i] = 0;
    }
}

void StaticValue::updateMusicPath(int musicId, const QString &newMusicPath)
{
    QSqlQuery query;
      query.prepare("UPDATE music SET music_path = :music_path WHERE id = :id");
      query.bindValue(":music_path", newMusicPath);
      query.bindValue(":id", musicId);

      if (!query.exec())
      {
          qDebug() << tr("更新音乐路径失败！error:")
                   << query.lastError().text();
          return;
      }
}

void StaticValue::InitSysConfig()
{
    m_SysConfig = QFileHelper::GetAllIniKeyValue("config.ini");

    
    
}

void StaticValue::getShowTamInfo()
{
    QSqlQuery query("select * from show_team");
    if(!query.exec())
    {
        qDebug() << tr("获取表演编队信息失败！") << query.lastError().text();
        return ;
    }
    while(query.next())
    {
        ShowTeam* show_team = new ShowTeam();
        show_team->id = query.value("id").toInt();
        show_team->name = query.value("name").toString();
        show_team->rowType = query.value("rowType").toInt();
        show_team->minNum = query.value("minNum").toInt();
        show_team->maxNum = query.value("maxNum").toInt();
        show_team->arrayNum = query.value("arrayNum").toInt();
        show_team->repeckType = query.value("repeckType").toInt();
        show_team->continueNum = query.value("continueNum").toInt();
        show_team->blankNum =query.value("blankNum").toInt();
        show_team->isReverse =query.value("isReverse").toBool();
        show_team->isRound = query.value("isRound").toBool();
        show_team->showTeamList = query.value("showTeamList").toString();
        m_show_team[show_team->id] = show_team;
    }
}

void StaticValue::getOutPutDevice()
{
    QSqlQuery query("select * from output_device");
    if(!query.exec())
    {
        qDebug() << tr("获取输出设备信息失败！") << query.lastError().text();
        return ;
    }
    while(query.next())
    {
        OutPutDevice* out_put_device = new OutPutDevice();
        out_put_device->id = query.value("id").toInt();
        out_put_device->name = query.value("name").toString();
        out_put_device->type = query.value("type").toString();
        out_put_device->serialPorts = query.value("serialPorts").toString();
        out_put_device->ipAddress = query.value("ipAddress").toString();
        out_put_device->port = query.value("port").toInt();
        out_put_device->Address = query.value("Address").toString();
        out_put_device->isSixteenths = query.value("isSixteenths").toBool();
        out_put_device->slotNum = query.value("slotNum").toInt();
        out_put_device->comment = query.value("comment").toString();
        m_output_device[out_put_device->id] = out_put_device;
    }
}

void StaticValue::getShowOrbit()
{
    QSqlQuery query("select * from orbit");
    if(!query.exec())
    {
        qDebug() << tr("获取表演轨迹信息失败！error:")
                 <<query.lastError().text();
        return ;
    }
    while(query.next())
    {
        Orbit* show_orbit = new Orbit();
        show_orbit->id = query.value(0).toInt();
        show_orbit->name = query.value(1).toString();
        show_orbit->time_length = query.value(2).toInt();
        show_orbit->delay_time = query.value(3).toInt();
        show_orbit->reversal = query.value(4).toBool();
        show_orbit->reversal_output = query.value(5).toBool();
        m_show_orbit[show_orbit->id] = show_orbit;
    }
    QList<int> keys = m_show_orbit.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        QString qStrSql = QString("select time,proportion from time_orbit "
                                  "where orbit_id = %1 order by time asc").arg(keys.at(i));
        if(!query.exec(qStrSql))
        {
            qDebug() << tr("获取表演轨迹信息失败！error:")
                     <<query.lastError().text();
            return ;
        }

        while(query.next())
        {
            m_show_orbit[i+1]->show_orbit[query.value(0).toInt()] =
                    query.value(1).toInt();
        }
    }
}

void StaticValue::getMusicInfo()
{
    QSqlQuery query("select * from music");
    if(!query.exec())
    {
        qDebug() << tr("获取编曲信息失败！error:")
                 <<query.lastError().text();
        return ;
    }
    while(query.next())
    {
        MusicInfo* music_info = new MusicInfo();
        music_info->id = query.value(0).toInt();
        music_info->type = query.value(1).toInt();
        music_info->name = query.value(2).toString();
        music_info->music_path = query.value(3).toString();
        music_info->time_length = query.value(4).toInt();
        m_music_info[music_info->id] = music_info;
    }
}

void StaticValue::getMusicMapping()
{
    QSqlQuery query("select * from music_mapping");
    if(!query.exec())
    {
        qDebug() << tr("获取编曲、编队、编队动作映射失败！error:")
                 <<query.lastError().text();
        return ;
    }
    while(query.next())
    {
        int music_id = query.value(0).toInt();
        int show_team_id = query.value(1).toInt();
        int music_action_id = query.value(2).toInt();
        m_music_info[music_id]->team_actions.
                insert(show_team_id,music_action_id);
    }
}

//从数据库DB读取Music_Action，里面有很多没用的项
void StaticValue::getMusicAction()
{
    QSqlQuery query("select * from music_action where begin_time is not null and end_time is not null");
    if(!query.exec())
    {
        qDebug() << tr("获取编曲动作失败！error:")
                 <<query.lastError().text();
        return ;
    }
    while(query.next())
    {
        MusicAction* music_action = new MusicAction();
        music_action->id = query.value(0).toInt();
        music_action->type = query.value(1).toInt();
        music_action->begin_time = query.value(2).toFloat();
        music_action->end_time = query.value(3).toFloat();
        m_music_action[music_action->id] = music_action;

        getRouteInfo(music_action->id);
    }
}

//从DB数据库读route表
void StaticValue::getRouteInfo(int music_action_id)
{
    QString qStrSql = QString("select route_id,type,out_value1,out_value2,keep_time,delay_time,"
                              "perial_time,loop_time,start_delay,orbit_id,change_speed,img_path,"
                              "yang_wen ,len1,len2,speed,circle,fade_in,fade_out,reverse_flag,from_left,ClockStyle,TimeLeadValue from route where music_action_id = %1").
								  arg(music_action_id);
    QSqlQuery query;
    if(!query.exec(qStrSql))
    {
        qDebug() << tr("获取编曲通道失败！error:")
                 << qStrSql
                 <<query.lastError().text();
        return ;
    }
    while(query.next())
    {
        RouteInfo* route = new RouteInfo();
        if(!query.value(0).isNull())
            route->id = query.value(0).toInt();
        if(!query.value(1).isNull())
            route->type = query.value(1).toInt();
        if(!query.value(2).isNull())
            route->out_value1 = query.value(2).toInt();
        if(!query.value(3).isNull())
            route->out_value2 = query.value(3).toInt();
        if(!query.value(4).isNull())
            route->keep_time = query.value(4).toInt();
        if(!query.value(5).isNull())
            route->delay_time = query.value(5).toInt();
        if(!query.value(6).isNull())
            route->perial_time = query.value(6).toInt();
        if(!query.value(7).isNull())
            route->loop_time = query.value(7).toInt();
        if(!query.value(8).isNull())
            route->start_delay = query.value(8).toInt();
        if(!query.value(9).isNull())
            route->orbit_id = query.value(9).toInt();
        if(!query.value(10).isNull())
            route->change_speed = query.value(10).toInt();
        if(!query.value(11).isNull())
            route->img_path = query.value(11).toString();
        if(!query.value(12).isNull())
            route->bIsYangWen = query.value(12).toBool();
        if(!query.value(13).isNull())
            route->len1 = query.value(13).toInt();
        if(!query.value(14).isNull())
            route->len2 = query.value(14).toInt();
        if(!query.value(15).isNull())
            route->speed = query.value(15).toInt();
		if (!query.value(16).isNull())
			route->circle = query.value(16).toInt();
		if (!query.value(17).isNull())
			route->fade_in = query.value(17).toInt();
		if (!query.value(18).isNull())
			route->fade_out = query.value(18).toInt();
		if (!query.value(19).isNull())
			route->reverse_flag = query.value(19).toInt();
		if (!query.value(20).isNull())
			route->from_left = query.value(20).toInt();
		if (!query.value(21).isNull())
			route->ClockStyle = query.value(21).toInt();
		if (!query.value(22).isNull())
			route->TimeLeadValue = query.value(22).toInt();

        m_music_action[music_action_id]->route_info[route->id] = route;
    }
}

void StaticValue::getDeviceInfo()
{
    QSqlQuery query;

    int iSize = m_output_device.size();
    QList<int> keys = m_output_device.keys();
    for(int i = 0 ; i < iSize; ++i)
    {
        int out_put_key = keys.at(i);
        QString qStrSql = QString("select * from device_info where output_device_id = %1").
                arg(out_put_key);
        if(!query.exec(qStrSql))
        {
            qDebug() << tr("获取电气阀信息失败！") << query.lastError().text();
            return ;
        }
        while(query.next())
        {
            DeviceInfo* device_info = new DeviceInfo();
            device_info->id = query.value(0).toInt();
            device_info->elec_device_id = query.value(1).toInt();
            device_info->output_device_id = query.value(2).toInt();
            m_output_device[out_put_key]->
                    m_mapping.insert(device_info->elec_device_id,device_info->id);

            device_info->name = query.value(3).toString();
            device_info->type = query.value(4).toString();
            device_info->pos_x = query.value(5).toInt();
            device_info->pos_y = query.value(6).toInt();
            device_info->pos_z = query.value(7).toInt();
            device_info->isDefault = query.value(8).toBool();



            device_info->delayinms = query.value(9).toInt();



			m_output_device[out_put_key]->
				m_delay_ms.insert(device_info->elec_device_id, device_info->delayinms);		//从数据库获得延时值

            m_device_map[device_info->id] = device_info;
        }
    }
}

void StaticValue::getRouteMapping()
{
    QSqlQuery query;
    if(!query.exec("select * from route_mapping"))
    {
        qDebug() <<"find route_mapping false!"<<query.lastError().text();
        return ;
    }
    while(query.next())
    {
        int route_id = query.value(0).toInt();
        int device_id = query.value(1).toInt();
        m_route_mapping.insert(device_id,route_id);
    }
}

//初始化所有的电气阀设备为0
void StaticValue::InitDeviceState()
{
    QList<int> keys = m_device_map.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        int device_id = keys.at(i);
        DeviceInfo* device_info = m_device_map[device_id];
        if (device_info->type.compare(tr("水帘阀")) == 0)
        {
            m_device_state[device_id] = 0;
        }
    }
}

void StaticValue::readImageSet()
{
    QSettings *configIniRead = new QSettings("imageset.ini", QSettings::IniFormat);
       //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
    QString m_strImageW = configIniRead->value("/size/first").toString();
    QString m_strImageH = configIniRead->value("/size/second").toString();
    QString m_strImageTimes = configIniRead->value("/other/first").toString();
    QString m_strImageDelay = configIniRead->value("/other/second").toString();

       //读入入完成后删除指针
    delete configIniRead;

    str_ipaimageW = m_strImageW;
    str_ipaimageH = m_strImageH;
    str_ipaimageTimes = m_strImageTimes;
    str_ipaimageDelayTime = m_strImageDelay;

}
