#include "senddatathread.h"
#include <QDebug>
//在高精度的时间定时器的情况下，不要使用qDebug(), 容易产生较大的误差
SendDataThread::SendDataThread(QObject *parent)
    : QThread(parent) {}

void SendDataThread::sendData(const QByteArray &SLData, const QString &qStrIPAddress, int qStrPort)
{
    QMutexLocker locker(&m_mutex);
    m_dataQueue.enqueue(qMakePair(SLData, qMakePair(qStrIPAddress, qStrPort)));
    //m_waitCondition.wakeOne();
}
SendDataThread::~SendDataThread()
{
	{
		QMutexLocker locker(&m_mutex);
		m_stop = true;
		//m_waitCondition.wakeOne();
	}
	wait();
}
void SendDataThread::run()
{
    QUdpSocket udp_dmnsocketonly;
	udp_dmnsocketonly.setSocketOption(QAbstractSocket::LowDelayOption, true);

    while (!m_stop)
    {
        m_mutex.lock();
	//	if (m_dataQueue.isEmpty())
	//	{
			//m_waitCondition.wait(&m_mutex);
	//	}

        if (!m_dataQueue.isEmpty())
        {
            auto data = m_dataQueue.dequeue();
            m_mutex.unlock();

            QByteArray SLData = data.first;
            QString qStrIPAddress = data.second.first;
            int qStrPort = data.second.second;

			//qDebug() << "SendDataThread::run ipAddress;" << qStrIPAddress << "dada" << SLData;
            QHostAddress ipAddress(qStrIPAddress);
            udp_dmnsocketonly.writeDatagram(SLData, SLData.size(), ipAddress, qStrPort);
            //QDateTime currentDateTime = QDateTime::currentDateTime();
            //qDebug() << "Current DateTime:" << currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
        }
        else
        {
            m_mutex.unlock();
        }
    }
}