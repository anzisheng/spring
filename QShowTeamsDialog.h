#ifndef QSHOWTEAMSDIALOG_H
#define QSHOWTEAMSDIALOG_H

#include <QDialog>
#include "ClassDefined.h"
#include "StaticValue.h"
#include <QVector>
#pragma execution_character_set("utf-8")
namespace Ui {
class QShowTeamsDialog;
}

class QShowTeamsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QShowTeamsDialog(int itemID,QWidget *parent = 0);
    ~QShowTeamsDialog();

private:
    void InitData();
//    void ShowTeamList(int iMinNum, int iMaxNum, int iarrayNum);

private slots:
    void on_push_button_accepted();
    void on_find_array_clicked();

private:
    Ui::QShowTeamsDialog *ui;
    int m_ItemID;
    ShowTeam* show_team;

};

#endif // QSHOWTEAMSDIALOG_H
