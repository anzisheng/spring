#include "ProgramControl.h"
#include "ui_ProgramControl.h"

ProgramControl::ProgramControl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgramControl)
{
    ui->setupUi(this);
}

ProgramControl::~ProgramControl()
{
    delete ui;
}

void ProgramControl::on_OK_clicked()
{
    m_timeLength = ui->time_length->text().toFloat();
    m_name = ui->control_name->text();
}
