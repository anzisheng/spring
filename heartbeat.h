#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <QDialog>
#pragma execution_character_set("utf-8")
namespace Ui {
class heartbeat;
}

class heartbeat : public QDialog
{
    Q_OBJECT

public:
    explicit heartbeat(QWidget *parent = 0);
    ~heartbeat();
private slots:
    void on_checkBox_clicked(bool checked);

private:
    Ui::heartbeat *ui;
};

#endif // HEARTBEAT_H
