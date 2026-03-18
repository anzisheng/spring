#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>

class DeviceInfo : public QObject
{
    Q_OBJECT
public:
    explicit DeviceInfo(QObject *parent = 0);

signals:

public slots:

public:
    int id;
    QString name;
    QString type;
    int pos_x;
    int pos_y;
    int pos_z;
    bool isDefault;

};

#endif // DEVICEINFO_H
