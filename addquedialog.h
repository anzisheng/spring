#ifndef ADDQUEDIALOG_H
#define ADDQUEDIALOG_H

#include <QDialog>

#include <QAbstractButton>
#include <QPushButton>
#pragma execution_character_set("utf-8")
namespace Ui {
class AddqueDialog;
}

class AddqueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddqueDialog(QWidget *parent = 0);
    ~AddqueDialog();


private:
    Ui::AddqueDialog *ui;
};

#endif // ADDQUEDIALOG_H
