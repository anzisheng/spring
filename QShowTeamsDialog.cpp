#include "QShowTeamsDialog.h"
#include "ui_QShowTeamsDialog.h"
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "CommonFile.h"

QShowTeamsDialog::QShowTeamsDialog(int itemID,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QShowTeamsDialog)
{
    ui->setupUi(this);

    QPushButton* btn_ok = ui->push_button->button(QDialogButtonBox::Ok);
    btn_ok->setText(tr("确定"));
    QPushButton* btn_Cancel = ui->push_button->button(QDialogButtonBox::Cancel);
    btn_Cancel->setText(tr("取消"));

    ui->rowType->addItem(tr("线性"));
    ui->rowType->addItem(tr("自定义"));

    QStringList comboBox_list;
    comboBox_list << tr("重复方式") <<
                     tr("镜像重复") <<
                     tr("反向镜像");
    ui->repeckType->addItems(comboBox_list);

    m_ItemID = itemID;
    show_team = StaticValue::GetInstance()->m_show_team.value(m_ItemID);
    InitData();
}


QShowTeamsDialog::~QShowTeamsDialog()
{
    delete ui;
}

void QShowTeamsDialog::InitData()
{
    ui->arrays_list->clear();
    if(show_team->rowType == LINEAR_TYPE)
    {
        ui->minNum->setValue(show_team->minNum);
        ui->maxNum->setValue(show_team->maxNum);
        ui->arrayNum->setValue(show_team->arrayNum);
        ui->continueNum->setValue(show_team->continueNum);
        ui->blankNum->setValue(show_team->blankNum);
        if(show_team->isReverse)
            ui->reverse->setChecked(true);
        else
            ui->reverse->setChecked(false);
        if(show_team->isRound)
            ui->round->setChecked(true);
        else
            ui->round->setChecked(false);
        ui->repeckType->setCurrentIndex(show_team->repeckType);
    }
    ui->ID->setValue(show_team->id);
    ui->name->setText(show_team->name);
    ui->rowType->setCurrentIndex(show_team->rowType);
    ui->arrays_list->setText(show_team->showTeamList);
}


void QShowTeamsDialog::on_find_array_clicked()
{
    ShowTeam show_team;
    show_team.rowType = ui->rowType->currentIndex();
    if(show_team.rowType == LINEAR_TYPE)
    {
        ui->arrays_list->clear();
        show_team.minNum = ui->minNum->text().toInt();
        show_team.maxNum = ui->maxNum->text().toInt();
        show_team.arrayNum = ui->arrayNum->text().toInt();
        show_team.repeckType = ui->repeckType->currentIndex();
        show_team.continueNum = ui->continueNum->text().toInt();
        show_team.blankNum = ui->blankNum->text().toInt();
        if(ui->reverse->isChecked())
            show_team.isReverse = true;
        else
            show_team.isReverse = false;
        if(ui->round->isChecked())
            show_team.isRound = true;
        else
            show_team.isRound = false;
        QString array_list = StaticValue::GetInstance()->
                getShowTeamList(&show_team);
        ui->arrays_list->clear();
        ui->arrays_list->setText(array_list);
    }
}

void QShowTeamsDialog::on_push_button_accepted()
{
    QString qStrSQL("");
    show_team->id = ui->ID->value();
    show_team->name = ui->name->text();
    show_team->rowType = ui->rowType->currentIndex();
    show_team->showTeamList = ui->arrays_list->toPlainText();
    if(ui->rowType->currentIndex() == LINEAR_TYPE)
    {
        show_team->minNum = ui->minNum->value();
        show_team->maxNum = ui->maxNum->value();
        show_team->arrayNum = ui->arrayNum->value();
        show_team->continueNum = ui->continueNum->value();
        show_team->blankNum = ui->blankNum->value();
        if(ui->reverse->isChecked())
            show_team->isReverse = true;
        else
            show_team->isReverse = false;
        if(ui->round->isChecked())
            show_team->isRound = true;
        else
            show_team->isRound = false;
        show_team->repeckType = ui->repeckType->currentIndex();

        qStrSQL = QString("update show_team set name = '%1',rowType = %2,"
                                  "minNum = %3,maxNum = %4,arrayNum = %5,repeckType = %6,"
                                  "continueNum = %7,blankNum = %8,isReverse = %9,isRound = %10,"
                                  "showTeamList = '%11' where id = %12").
                arg(show_team->name).arg(show_team->rowType).arg(show_team->minNum).
                arg(show_team->maxNum).arg(show_team->arrayNum).arg(show_team->repeckType).
                arg(show_team->continueNum).arg(show_team->blankNum).arg(show_team->isReverse).
                arg(show_team->isRound).arg(show_team->showTeamList).arg(show_team->id);
    }
    else
    {
        qStrSQL = QString("update show_team set name = '%1',rowType = %2,showTeamList = '%3'"
                          " where id = %4").arg(show_team->name).arg(show_team->rowType).
                arg(show_team->showTeamList).arg(show_team->id);
    }

    QSqlQuery query;
    if(!query.exec(qStrSQL))
    {
        qDebug() <<"update show_team false!sql:"<<qStrSQL<<
                   " error:"<<query.lastError().text();
    }
}
