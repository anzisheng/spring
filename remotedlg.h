#ifndef REMOTEDLG_H
#define REMOTEDLG_H

#include <QDialog>
#include "ClassDefined.h"
#pragma execution_character_set("utf-8")
namespace Ui {
class RemoteDlg;
}

class RemoteDlg : public QDialog
{
    Q_OBJECT
private:

    remotecontrol* curset_;
public:
    explicit RemoteDlg(remotecontrol* cursetting ,QWidget *parent = 0);
    ~RemoteDlg();
private slots:
    void on_buttonBox_accepted();

private:
    Ui::RemoteDlg *ui;
};

#endif // REMOTEDLG_H
