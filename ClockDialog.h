#ifndef CLOCKDIALOG_H
#define CLOCKDIALOG_H

#include <QDialog>
#include <QTimer>		//需要再这里包含，才能使用QTimer类

#pragma execution_character_set("utf-8")
namespace Ui {
class ClockDialog;
}

class ClockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClockDialog(QWidget *parent = 0);
    ~ClockDialog();

//    void setData(QString qStrPath, int delay_time,		//初始化对话框内容(再编辑的时候)
//					int loop_time,bool bIsYangWen);
//	void setData(int clockstylesel, int timeleadvalue, int delay_time,		//初始化对话框内容(再编辑的时候)
//					int loop_time, bool bIsYangWen);
	void setData(int clockstylesel, int timeleadvalue, int delay_time,		//初始化对话框内容(再编辑的时候)
		int loop_time, bool bIsYangWen);
    void loadImage();

private slots:
	void on_ClockStyle_select_currentIndexChanged(const QString &arg1);

    void on_textFormat_clicked();	//字体格式按钮的响应函数

//	void on_addImage_clicked();

    void on_reverse_clicked();		//阳文阴文的选择

    void on_buttonBox_accepted();	//OK按钮的响应函数

    void on_verticalSlider_valueChanged(int value);	//缩放滚轮的响应函数

	void ClockTimeOut();

public:
	QString m_ImagePath;	//图片路径名称
	int m_ClockStyleSel;	//时钟样式选择
	int m_TextFormatSel;	//字体格式选择，大小
	int m_TimeLeadValue;	//时间提前量

	int m_LoopTime;			//循环次数
	int m_DelayTime;		//扫描时间
	int m_IsYangWen;		//是否是阳文
	int m_Balance;			//缩放比例

	QImage m_image;			//加载的图片对象
	QImage new_image;		//缩放后的图片对象

private:
    Ui::ClockDialog *ui;

	QTimer* m_ClockTimer;	//对话框时钟定时器，1秒一次

	void draw(QPainter *painter);

};

#endif // SELECTIMAGEDIALOG_H
