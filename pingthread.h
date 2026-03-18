#ifndef PINGTHREAD_H
#define PINGTHREAD_H

#include <QThread>
#include <QMap>
#include <QString>
#include <QUdpSocket>
#include <QMutex>
#include <QFutureWatcher>

class PingThread : public QThread {
    Q_OBJECT

public:
    PingThread(QMap<int, QString> deviceList, QObject *parent = nullptr);
	//void start();
	//选择UDP
	QUdpSocket *udpSocket;

signals:
    void pingResult(double percentage);
signals:
	void stopPing();

protected:
    void run() override;

private slots:
    void checkDevices();
	void onStopPing();

private:
    bool pingDevice(const QString &ipAddress, int timeout = 2000);
    QMap<int, QString> m_deviceList;
	void TransHeart(unsigned int ccc);
	int m_devicerate;
	int m_heartlastnet;
private:
	QMutex m_mutex;
	QFutureWatcher<void> m_watcher;
	bool m_stop;
};

#endif // PINGTHREAD_H