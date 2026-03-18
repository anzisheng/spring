#include "DominoSetting.h"
#include "ui_DominoSetting.h"

DominoSetting::DominoSetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DominoSetting)
{
    ui->setupUi(this);

    m_iKeepTime = 1000;
    m_iDelayTime = 100;
    m_iPerialTime = 2000;
    m_iLoopTimes = -1;

	m_iOutputValue1 =255;
	m_iOutputValue2 =0;
	m_iStartDelay =0;

	m_iReverseFlag = 0;
	m_iFromLeft = 0;

	setData(1000,100,2000,-1,255,0,0,0,0);
}

DominoSetting::~DominoSetting()
{
    delete ui;
}

void DominoSetting::setData(int keep_time,int delay_time,
							int perial_time,int loop_time,
							int outputvalue1,int outputvalue2,int startdelay,
							int reverse_flag,int from_left)
{
    ui->KeepTime->setText(QString::number(keep_time));
    ui->DelayTime->setText(QString::number(delay_time));
    ui->PeriodTime->setText(QString::number(perial_time));
    ui->LoopTimes->setText(QString::number(loop_time));

	ui->OutputValue1->setText(QString::number(outputvalue1));
	ui->OutputValue2->setText(QString::number(outputvalue2));
	ui->StartDelay->setText(QString::number(startdelay));

	if (reverse_flag)
		ui->ReverseFlag->setChecked(true);
	else
		ui->ReverseFlag->setChecked(false);

	if (from_left)
		ui->FromLeft->setChecked(true);
	else
		ui->FromLeft->setChecked(false);
}

void DominoSetting::on_buttonBox_accepted()
{
    m_iKeepTime = ui->KeepTime->text().toInt();
    m_iDelayTime = ui->DelayTime->text().toInt();
    m_iPerialTime = ui->PeriodTime->text().toInt();
    m_iLoopTimes = ui->LoopTimes->text().toInt();

	m_iOutputValue1 = ui->OutputValue1->text().toInt();
	m_iOutputValue2 = ui->OutputValue2->text().toInt();
	m_iStartDelay = ui->StartDelay->text().toInt();

	if (ui->ReverseFlag->isChecked())
		m_iReverseFlag = true;
	else
		m_iReverseFlag = false;

	if (ui->FromLeft->isChecked())
		m_iFromLeft = true;
	else
		m_iFromLeft = false;
}
