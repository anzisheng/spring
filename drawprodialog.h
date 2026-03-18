#ifndef DRAWPRODIALOG_H
#define DRAWPRODIALOG_H

#include <QDialog>
#pragma execution_character_set("utf-8")
namespace Ui {
class DrawProDialog;
}

class DrawProDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DrawProDialog(QWidget *parent = 0);
    ~DrawProDialog();

private:
    Ui::DrawProDialog *ui;
};

#endif // DRAWPRODIALOG_H
