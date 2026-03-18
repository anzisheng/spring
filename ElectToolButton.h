#ifndef ELECTTOOLBUTTON_H
#define ELECTTOOLBUTTON_H

#include <QToolButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QScrollArea>

class ElectToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ElectToolButton(QWidget * parent = 0);

    void setID(int ID);
    void setName(QString qStrName);
    void setType(QString qStrType);
    void setX(int posX);
    void setY(int posY);
    void setZ(int posZ);

    void showEvent(QShowEvent * event);
signals:

public slots:

private:
    int m_ID;
    QString m_Name;
    QString m_Type;
    int m_PosX;
    int m_PosY;
    int m_PosZ;
    QScrollArea* m_Scroll;

};

#endif // ELECTTOOLBUTTON_H
