#include "QFileHelper.h"
#include <qfile.h>
#include <QStringList.h>
#include <qsettings.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <Windows.h>
#include <QFileInfo>
#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QStringConverter>

QFileHelper::QFileHelper(QObject *parent)
	: QObject(parent)
{

}

QFileHelper::~QFileHelper()
{

}

bool QFileHelper::IsFileExist(QString qStrFilePath)
{
	QFile file(qStrFilePath);
	if (!file.exists())
    {
		return false;
	}
	return true;
}


bool QFileHelper::DelFile(QString qStrFilePath)
{
	QFile file(qStrFilePath);
	if (!file.exists())
    {
		return false;
	}

	return QFile::remove(qStrFilePath);
}

QString QFileHelper::GetFileSuffix(QString qStrFilePath)
{
	return qStrFilePath.right(qStrFilePath.size() - qStrFilePath.lastIndexOf(".")-1);
}

int QFileHelper::GetFileSize(QString qStrFilePath)
{
	int iFileLen = 0;
	QFile file(qStrFilePath);
	if (!file.exists())
    {
		return iFileLen;
	}
	
	if (!file.open(QIODevice::ReadOnly))
    {
		return iFileLen;
	}

	iFileLen = file.size();
	return iFileLen;
}

QString QFileHelper::GetFilePath(QString qStrFullPath)
{
	QString qStrFilePath = "";
	int iFind = qStrFullPath.lastIndexOf("\\");
	qStrFilePath = qStrFullPath.left(iFind);
	return qStrFilePath;
}

QString QFileHelper::GetFileName(QString qStrFilePath)
{
	QStringList qList = qStrFilePath.split("\\");
	QString qStrName = qList[qList.size() - 1];
	return qStrName;
}

QString QFileHelper::GetFileContent(QString qStrFilePath)
{
	QString qContent;
	QFile qf(qStrFilePath);
	QString qStrInfo("");
	if (!qf.exists())
    {
		return qContent;
	}

	if (!qf.open(QIODevice::ReadOnly))
    {
		return qContent;
	}
	
	if (qf.size() > 0)
	{
        QStringDecoder decoder(QStringConverter::System);
        qContent = decoder.decode(qf.readAll());
	}
	qf.close();
	return qContent;
}

bool QFileHelper::WriteFileContent(QString qStrFilePath,QString qStrFileContent)
{
	bool bRet = false;
	QString qStrInfo("");
	if (qStrFileContent.isNull() || qStrFileContent.isEmpty())
    {
		return bRet;
	}
	QFile qf(qStrFilePath);

	if (!qf.open(QIODevice::WriteOnly))
    {
		return bRet;
	}

	qf.write(qStrFileContent.toStdString().c_str());
	qf.close();
	return true;
}


void QFileHelper::SetIniKeyValue(QString qStrFilePath,QString qStrKey,QString qStrValue)
{
	QSettings setting(qStrFilePath,QSettings::IniFormat);
	QStringList qKeys = setting.allKeys();
//	if (qKeys.contains(qStrKey))
	{
		setting.setValue(qStrKey.toLocal8Bit(),qStrValue);//设置日志文件存放路径
	}
}

QString QFileHelper::GetIniKeyValue(QString qStrFilePath,QString qStrKey)
{
	QSettings setting(qStrFilePath,QSettings::IniFormat);
	return setting.value(qStrKey).toString();
}

QMap<QString,QString> QFileHelper::GetAllIniKeyValue(QString qStrFilePath)
{
	QMap<QString,QString> qIniMap;
	QSettings setting(qStrFilePath.toStdString().c_str(),QSettings::IniFormat);
	char szTemp[260] = {0};
	QStringList qStrGroup = setting.allKeys();
	QStringList qStrChildGroup = setting.childGroups();
	for (int i = 0; i < qStrChildGroup.size();++i)//遍历Ini文件
	{
		QString qStrAppName = qStrChildGroup.at(i);
		setting.beginGroup(qStrChildGroup.at(i));
		QStringList qStrChildKey = setting.childKeys();
		for (int j = 0; j < qStrChildKey.size(); ++j)
		{
			QString qStrKey = qStrChildKey.at(j);
//			GetPrivateProfileStringA(qStrAppName.toStdString().c_str(),
//				qStrKey.toStdString().c_str(),
//				"",
//				szTemp,
//				sizeof(szTemp),
//				qStrFilePath.toStdString().c_str());
            QString qStrValue = setting.value(qStrKey).toString();
			qIniMap.insert(qStrKey,qStrValue);
		}
		setting.endGroup();
	}
	return qIniMap;
}

void QFileHelper::addSubFolderFiles(QString path,QStringList filters)
{
	QDir dir(path);
	if(!dir.exists())
	{
		return;
    }
	QDirIterator dir_iterator(path,
		filters,
		QDir::Files | QDir::NoSymLinks,
		QDirIterator::Subdirectories);
	QStringList string_list;
	while(dir_iterator.hasNext())
	{
		dir_iterator.next();
		QFileInfo file_info = dir_iterator.fileInfo();
		QString absolute_file_path = file_info.absoluteFilePath();
	}
}

