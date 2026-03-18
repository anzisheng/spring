#ifndef DOMINOSETTING_H
#define DOMINOSETTING_H

#include <QDialog>
#pragma execution_character_set("utf-8")
namespace Ui {
class DominoSetting;
}

class DominoSetting : public QDialog
{
    Q_OBJECT

public:
    explicit DominoSetting(QWidget *parent = 0);
    ~DominoSetting();

//    void setData(int keep_time, int delay_time,
//                 int perial_time, int loop_time);

//	void setData(int keep_time,int delay_time,
//				int perial_time,int loop_time,
//				int outputvalue1,int outputvalue2,int startdelay);

	void setData(int keep_time, int delay_time,
		int perial_time, int loop_time,
		int outputvalue1, int outputvalue2, int startdelay,
		int reverse_flag, int from_left);


private slots:
    void on_buttonBox_accepted();


public:
    int m_iKeepTime;
    int m_iDelayTime;
    int m_iPerialTime;
    int m_iLoopTimes;

	int m_iOutputValue1;
	int m_iOutputValue2;
	int m_iStartDelay;

	int m_iReverseFlag;
	int m_iFromLeft;
private:
    Ui::DominoSetting *ui;

};

#endif // DOMINOSETTING_H
