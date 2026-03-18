#ifndef DIALOGLOGINDRAW_H
#define DIALOGLOGINDRAW_H

#include <QDialog>
#include <QAbstractButton>
#include <QPushButton>
#pragma execution_character_set("utf-8")
namespace Ui {
class DialogLoginDraw;
}

class DialogLoginDraw : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLoginDraw(QWidget *parent = 0);
    ~DialogLoginDraw();

private slots:

    void on_pushButton_Yes_clicked();

    void on_pushButton_No_clicked();

private:
    Ui::DialogLoginDraw *ui;
};

#endif // DIALOGLOGINDRAW_H
