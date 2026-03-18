#ifndef QFILEHELPER_H
#define QFILEHELPER_H

#include <QObject>
#include <qmap.h>
#pragma execution_character_set("utf-8")
class QFileHelper : public QObject
{
	Q_OBJECT

public:
	QFileHelper(QObject *parent);
	~QFileHelper();

public:
	static bool IsFileExist(QString qStrFilePath);
	static bool DelFile(QString qStrFilePath);
	static QString GetFileSuffix(QString qStrFilePath);
	static int GetFileSize(QString qStrFilePath);
	static QString GetFileName(QString qStrFilePath);
	static QString GetFilePath(QString qStrFullPath);
	static QString GetFileContent(QString qStrFilePath);
	static bool WriteFileContent(QString qStrFilePath,QString qStrFileContent);

	static void SetIniKeyValue(QString qStrFilePath,QString qStrKey,QString qStrValue);
	static QString GetIniKeyValue(QString qStrFilePath,QString qStrKey);
	static QMap<QString,QString> GetAllIniKeyValue(QString qStrFilePath);

	static void addSubFolderFiles(QString path,QStringList filters);

	static QString getFileMd5(QString filePath);
};

#endif // QFILEHELPER_H
