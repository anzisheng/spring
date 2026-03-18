#include "drawprodialog.h"
#include "ui_drawprodialog.h"

DrawProDialog::DrawProDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DrawProDialog)
{
    ui->setupUi(this);
}

DrawProDialog::~DrawProDialog()
{
    delete ui;
}
