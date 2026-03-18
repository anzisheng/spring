#include "pingthread.h"
#include <QTimer>
#include <QProcess>
#include <QDebug>

#include <QByteArray>
#include <QString>
#include <QDateTime>

#include <QCoreApplication>
#include <QSettings>
#include <QDir>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
PingThread::PingThread(QMap<int, QString> deviceList, QObject *parent)
    : QThread(parent), m_deviceList(deviceList) {
	m_stop = false;
}

void PingThread::run() {
	connect(this, &PingThread::stopPing, this, &PingThread::onStopPing, Qt::DirectConnection);
	QString exePath = QCoreApplication::applicationDirPath();
	QString iniPath = QDir(exePath).filePath("pingcfg.ini");
	QSettings settings(iniPath, QSettings::IniFormat);

	settings.beginGroup("pingcfg");
	m_devicerate = settings.value("devicerate", 90).toInt();
	m_heartlastnet = settings.value("heartlastnet", 88).toInt();
	settings.endGroup();

	//qDebug() << "devicerate:" << m_devicerate;
	//qDebug() << "heartlastnet:" << m_heartlastnet;

    QTimer::singleShot(50, this, &PingThread::checkDevices);
	udpSocket = new QUdpSocket();
    exec();
}

void PingThread::checkDevices() {
	if (m_stop)
	{
		return;
	}
	m_mutex.lock();
	int totalCount = m_deviceList.size();
	QVector<QFuture<bool>> m_futures;

	for (const QString &ip : m_deviceList.values()) {
		QFuture<bool> future = QtConcurrent::run([this, ip]() {
			return this->pingDevice(ip);
		});
		m_futures.append(future);
	}
	m_mutex.unlock();
	int m_successCount = 0;
	for (auto &future : m_futures) {
		QFutureWatcher<bool> watcher;	

		QTimer timer;
		timer.setSingleShot(true);
		timer.setInterval(2000); // 2 seconds timeout

		bool finished = false;
		QObject::connect(&watcher, &QFutureWatcher<bool>::finished, [&]() {
			finished = true;
			timer.stop();
		});
		watcher.setFuture(future); // connnect 之后调用防止卡住，挂起
		timer.start();
		while (!finished && timer.isActive()) {
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
		}

		if (finished) {
			//qDebug() << "finished within the 2 seconds timeout";
		}
		else {
			qDebug() << "not finished within the 2 seconds timeout";
		}
		//取ping 通的加起来，获取百分比
		if (future.result()) {
			m_successCount++;
		}
	}

	double percentage = static_cast<double>(m_successCount) / totalCount * 100;
	if (m_stop==false)
	{
		emit pingResult(percentage);
	}
	

	QDateTime currentDateTime = QDateTime::currentDateTime();
	QString currentTime = currentDateTime.toString("hh:mm:ss");
	//qDebug() << "Current time:" << currentTime;
	//qDebug() << "percent" << percentage;

	if (percentage >= m_devicerate && m_stop == false) {
		TransHeart(m_heartlastnet);
	}
	if (m_stop)
	{

	}
	else
	{
		QTimer::singleShot(3000, this, &PingThread::checkDevices);
	}
	
}

bool PingThread::pingDevice(const QString &ipAddress, int timeout) {	
	QString strIpAddress = ipAddress;	// Assigns the IP address to strIpAddress
	QString nParameter = "-n";			// Sets the parameter for the number of echo requests to send
	QString pingCount = "1";			// Sets the number of echo requests to send
	QString wParameter = "-w";			// Sets the parameter for the timeout in milliseconds to wait for each reply
	QString pingWaitTime = "200";		// Sets the timeout in milliseconds to wait for each reply, 超时设定为0.5秒
	QProcess pingProcess;				// Creates a new QProcess object
	QStringList arguments;				// Creates a QStringList to store the arguments for the ping command
	arguments << strIpAddress << nParameter << pingCount << wParameter << pingWaitTime; // Adds the arguments to the QStringList
	int exitCode = pingProcess.execute("ping", arguments); // Executes the ping command with the specified arguments and stores the exit code

	return exitCode == 0;
}

void PingThread::TransHeart(unsigned int ccc)
{
	unsigned char temp = 0;
	int i;
	unsigned char t = 0;

	QString strIPaddress = "192.168.1.";
	char stemp[10];

	const int ETH_LEN = 4; // Adjust this value according to your needs
	unsigned char t_buf[ETH_LEN];
	memset(t_buf, 0, ETH_LEN);

	t_buf[0] = 'A'; //标志头1
	t_buf[1] = 'H'; //标志头2

	t_buf[2] = 0x60; //CMD=0，心跳包

	t = 0;
	for (i = 0; i < 3; i++)
		t = t + t_buf[i];

	t_buf[3] = t;

	strIPaddress = "192.168.1.";

	for (i = 0; i < 10; i++) //这句话有用，初始化stemp所有值为0
		stemp[i] = 0;
	sprintf(stemp, "%u", ccc); //地址
	strIPaddress = strIPaddress + QString::fromLatin1(stemp);
	
	QByteArray datagram(reinterpret_cast<char*>(t_buf), 4);
	udpSocket->writeDatagram(datagram, QHostAddress(strIPaddress), 6000);
}
void PingThread::onStopPing()
{
	m_stop = true;
}