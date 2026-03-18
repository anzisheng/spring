#ifndef DMXLIGHTDIALOG_H
#define DMXLIGHTDIALOG_H

#include <QDialog>
#include "ClassDefined.h"
#include <QMap>
#pragma execution_character_set("utf-8")
namespace Ui {
class DMXLightDialog;
}

class DMXLightDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DMXLightDialog(QWidget *parent = 0);
    ~DMXLightDialog();

    void setData(QMap<int,RouteInfo*> dmx_info);


private:
    QVector<int> getSelectRows();
    void InitOrbitInfo();

private slots:
    void on_action_select_currentIndexChanged(const QString &arg1);
    void on_buttonBox_accepted();
    void on_submit_clicked();
    void cellClick(int x,int y);

public:
	QMap<int,RouteInfo*> m_route_setting;	//灯光所有通道的设置（保存新增时用）
	QMap<int,RouteInfo*> user_route_setting;  //初始化对话框时读取的数据库里的通道设置

	int m_iKeepTime;
	int m_iDelayTime;
	int m_iPerialTime;
	int m_iLoopTimes;

	int m_iOutputValue1;
	int m_iOutputValue2;
	int m_iStartDelay;

	int m_iFadeIn;
	int m_iFadeOut;

	int m_iReverseFlag;
	int m_iFromLeft;

private:
    Ui::DMXLightDialog *ui;

};

#endif // DMXLIGHTDIALOG_H
