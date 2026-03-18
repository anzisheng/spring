#include "addquedialog.h"
#include "ui_addquedialog.h"

AddqueDialog::AddqueDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddqueDialog)
{
    ui->setupUi(this);
 //   spinbox->setButtonSymbols(QAbstractSpinBox::NoButtons);
 //   ui->spin_move->setButtonSymbols(QAbstractSpinBox::
}

AddqueDialog::~AddqueDialog()
{
    delete ui;
}
