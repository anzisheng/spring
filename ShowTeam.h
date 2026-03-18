#ifndef SHOWTEAM_H
#define SHOWTEAM_H

#include <QObject>

class ShowTeam : public QObject
{
    Q_OBJECT
public:
    explicit ShowTeam(QObject *parent = 0);

signals:

public slots:

public:
    int id;
    QString name;
    int rowType;
    int minNum;
    int maxNum;
    int arrayNum;
    QString repeckType;
    int continueNum;
    int blankNum;
    bool isReverse;
    bool isRound;
    QString water_array;
    QString comment;
};

#endif // SHOWTEAM_H
