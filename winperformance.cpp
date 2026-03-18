#include "winperformance.h"


#ifdef __MINGW32__
#define TIME_KILL_SYNCHRONOUS 0x0100
#endif

Turing_Win_PerformanceTimer::Turing_Win_PerformanceTimer(QObject *parent)
    : QObject{parent}
{
    m_id=0;
}

Turing_Win_PerformanceTimer::~Turing_Win_PerformanceTimer()
{
    stop();
}
void  CALLBACK PeriodCycle(uint timerId,uint,DWORD_PTR user,DWORD_PTR,DWORD_PTR)
{
    Q_UNUSED(timerId)
    Turing_Win_PerformanceTimer *t=reinterpret_cast<Turing_Win_PerformanceTimer *>(user);
    emit t->timeout();
}
void Turing_Win_PerformanceTimer::start(int timeInterval)
{
    m_id = timeSetEvent(timeInterval,1,PeriodCycle,(DWORD_PTR)this,TIME_CALLBACK_FUNCTION|TIME_PERIODIC|TIME_KILL_SYNCHRONOUS);
}
void Turing_Win_PerformanceTimer::stop()
{
    if(m_id)
    {
        timeKillEvent(m_id);
        m_id=0;
    }
}


