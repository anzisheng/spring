#ifndef SELECTIMAGEDIALOG_H
#define SELECTIMAGEDIALOG_H

#include <QDialog>
#pragma execution_character_set("utf-8")
namespace Ui {
class SelectImageDialog;
}

class SelectImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectImageDialog(QWidget *parent = 0);
    ~SelectImageDialog();

    void setData(QString qStrPath, int delay_time,
                 int loop_time,bool bIsYangWen);
    void loadImage();

private slots:
    void on_textFormat_clicked();

    void on_addImage_clicked();

    void on_reverse_clicked();

    void on_buttonBox_accepted();

    void on_verticalSlider_valueChanged(int value);

public:
    QString m_ImagePath; //图片路径
    int m_LoopTime;   //循环次数
    int m_DelayTime;  //扫描时间
    int m_IsYangWen;  //是否是阳文
    int m_Balance;     //缩放比例

    QImage m_image;    //加载的图片对象
    QImage new_image;  //缩放后的图片对象



private:
    Ui::SelectImageDialog *ui;

};

#endif // SELECTIMAGEDIALOG_H
