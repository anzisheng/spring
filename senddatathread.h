#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDateTime>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class SendDataThread : public QThread
{
    Q_OBJECT

public:
    SendDataThread(QObject *parent = nullptr);
	~SendDataThread(); // 添加析构函数声明

    void sendData(const QByteArray &SLData, const QString &qStrIPAddress, int qStrPort);

protected:
    void run() override;

private:
    QQueue<QPair<QByteArray, QPair<QString, int>>> m_dataQueue;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    bool m_stop = false;
};