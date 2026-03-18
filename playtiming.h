#ifndef PLAYTIMING_H
#define PLAYTIMING_H

#include <QDialog>
#include <QDateTime>
#include <QMap>
#pragma execution_character_set("utf-8")
namespace Ui {
class playtiming;
}


class timinginfo
{
public:
    timinginfo() {}

    bool operator==(const timinginfo &other) const
    {
        return start_ == other.start_ && end_ == other.end_ && musicname_== other.musicname_&& ControlMode_==other.ControlMode_;
    }
public:
    QDateTime start_;
    QDateTime end_;
    QString   musicname_;
    QString   ControlMode_;
};
class playtiming : public QDialog
{
    Q_OBJECT

public:
    explicit playtiming(QWidget *parent = 0);
    ~playtiming();
    static     QMultiMap<QDate,timinginfo> timinglist_;
private slots:
    void on_currentDate_clicked();

    void on_pushButton_2_clicked();

    void on_calendarWidget_clicked(const QDate &date);

    void on_pushButton_clicked();

    void on_rowselected(int rownum);

    void on_pushButton_3_clicked();

private:
    Ui::playtiming *ui;
    void InitPlaylist();
    void    updateplaylist(QDate selDate);
    void    updateplaylist();
    void    remove(QDate date,QDateTime start,QDateTime end);
    void    remove(QDate date,int index);

};

#endif // PLAYTIMING_H
