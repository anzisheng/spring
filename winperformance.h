#ifndef TURING_WIN_PERFORMANCETIMER_H
#define TURING_WIN_PERFORMANCETIMER_H

#include <QObject>

#include "windows.h"
/**
 * @brief The Turing_Win_PerformanceTimer class
 * Win平台下，高精度毫秒级定时器
 * 这里使用了win的api，所以在linux下无法使用
 */
class Turing_Win_PerformanceTimer : public QObject
{
    Q_OBJECT
public:
    explicit Turing_Win_PerformanceTimer(QObject *parent = nullptr);
    ~Turing_Win_PerformanceTimer();

signals:
    void timeout();
public slots:
    void start(int timeInterval);
    void stop();
    //friend void WINAPI CALLBACK PeriodCycle(uint,uint,DWORD_PTR,DWORD_PTR,DWORD_PTR);
    friend void CALLBACK PeriodCycle(uint,uint,DWORD_PTR,DWORD_PTR,DWORD_PTR);
private:
    int m_interval;
    int m_id;

};

#endif // TURING_WIN_PERFORMANCETIMER_H
