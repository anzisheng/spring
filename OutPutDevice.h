#ifndef OUTPUTDEVICE_H
#define OUTPUTDEVICE_H

#include <QObject>
#include <QMap>

class OutPutDevice : public QObject
{
    Q_OBJECT
public:
    explicit OutPutDevice(QObject *parent = 0);

signals:

public slots:

public:
    int id;
    QString name;
    QString type;
    QString serialPorts;
    QString ipAddress;
    QString port;
    QString Address;
    bool isSixteenths;
    int slotNum;
    QString comment;
    QMap<int,int> m_mapping;

};

#endif // OUTPUTDEVICE_H
