#include "editiddialog.h"
#include "ui_editiddialog.h"

EditIDDialog::EditIDDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EditIDDialog)
{
	ui->setupUi(this);

}

EditIDDialog::~EditIDDialog()
{
	delete ui;
}

void EditIDDialog::setData(int old_id)
{
	ui->OldID->setText(QString::number(old_id));
	ui->NewID->setText(QString::number(old_id));
}

void EditIDDialog::on_buttonBox_accepted()
{
	m_iNewID = ui->NewID->text().toInt();
}
