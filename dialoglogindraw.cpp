#include "dialoglogindraw.h"
#include "ui_dialoglogindraw.h"
#include "drawprodialog.h"
#include <QMessageBox>





DialogLoginDraw::DialogLoginDraw(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLoginDraw)
{
    ui->setupUi(this);
}

DialogLoginDraw::~DialogLoginDraw()
{
    delete ui;
}



void DialogLoginDraw::on_pushButton_Yes_clicked()
{
    if(ui->lineEdit_password->text()=="admin123")
    {
        this->destroy(true);
        DrawProDialog pro_dialog;
        pro_dialog.setModal(true);
        pro_dialog.exec();

    }

    else
    {
        ui->label_mess->setText(tr("密码错误，请重新输入!"));
    }
}

void DialogLoginDraw::on_pushButton_No_clicked()
{
    this->destroy(true);
}
