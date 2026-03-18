#ifndef PROGRAMCONTROL_H
#define PROGRAMCONTROL_H

#include <QDialog>
#pragma execution_character_set("utf-8")
namespace Ui {
class ProgramControl;
}

class ProgramControl : public QDialog
{
    Q_OBJECT

public:
    explicit ProgramControl(QWidget *parent = 0);
    ~ProgramControl();

private slots:
    void on_OK_clicked();

public:
    float m_timeLength;
    QString m_name;

private:
    Ui::ProgramControl *ui;
};

#endif // PROGRAMCONTROL_H
