#ifndef IPADIMAGESETDIALOG_H
#define IPADIMAGESETDIALOG_H

#include <QDialog>
#pragma execution_character_set("utf-8")
namespace Ui {
class IpadImageSetDialog;
}

class IpadImageSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IpadImageSetDialog(QWidget *parent = 0);
    ~IpadImageSetDialog();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::IpadImageSetDialog *ui;
};

#endif // IPADIMAGESETDIALOG_H
