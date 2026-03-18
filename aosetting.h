#ifndef AOSETTING_H
#define AOSETTING_H

#include <QDialog>
#include <QTimer>
#include <QUdpSocket>

#pragma execution_character_set("utf-8")

namespace Ui {
class AOsetting;
}

class AOsetting : public QDialog
{
    Q_OBJECT

public:
    explicit AOsetting(QWidget *parent = 0);
    ~AOsetting();
    virtual void accept();

    qreal lowVoltage_;

    qreal HighVoltage_;

    QString ipaddr_;

    bool   enable_;

    qreal lowM_;        //低电压，手动模式时保存

    qreal highM_;        //高电压，手动模式时保存

    void resetsetting();
private slots:
    void on_pushButton_clicked();

    void ManualTimeup();

private:
    Ui::AOsetting *ui;

    QTimer* Manultimer_ = nullptr;

    QUdpSocket*     AOvolset_   =   nullptr;

    void sendvol(int vol);

    int curDuration_;       //当前手动补水时间
    void closeEvent(QCloseEvent * event);

};

#endif // AOSETTING_H
