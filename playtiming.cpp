#include "playtiming.h"
#include "ui_playtiming.h"
#include "StaticValue.h"
#include <QTableWidgetItem>


QMultiMap<QDate,timinginfo> playtiming::timinglist_;

playtiming::playtiming(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::playtiming)
{
    ui->setupUi(this);
    InitPlaylist();

    //设置日历表示范围（默认一年）
    ui->calendarWidget->setMinimumDate(QDate::currentDate());
    connect(ui->timinginfo->verticalHeader(),&QHeaderView::sectionClicked,this,&playtiming::on_rowselected);
    ui->calendarWidget->setMaximumDate(QDate::currentDate().addYears(5));
}

playtiming::~playtiming()
{
    delete ui;
}

void playtiming::InitPlaylist()
{
    QMap<int,MusicInfo*>  music_info = StaticValue::GetInstance()->m_music_info;
    foreach (MusicInfo* info, music_info)
    {
        ui->listWidget->addItem(info->name);
    }
}

void playtiming::updateplaylist(QDate selDate)
{

    ui->timinginfo->clearContents();

    QList<timinginfo> values = timinglist_.values(selDate);

    ui->timinginfo->setRowCount(values.size());

    for (int i = 0; i < values.size(); ++i)
    {
        int colnum  =   0;
        ui->timinginfo->setItem(i,colnum++,new QTableWidgetItem(values.at(i).start_.toString("yy-M-d h:m:s")));
        ui->timinginfo->setItem(i,colnum++,new QTableWidgetItem(values.at(i).end_.toString("yy-M-d h:m:s")));
        ui->timinginfo->setItem(i,colnum++,new QTableWidgetItem(values.at(i).musicname_));
        ui->timinginfo->setItem(i,colnum++,new QTableWidgetItem(values.at(i).ControlMode_));
    }
}

void playtiming::updateplaylist()
{
    ui->timinginfo->clearContents();

    ui->timinginfo->setRowCount(timinglist_.size());

    int rownum  =   0;
    for (QMultiMap<QDate, timinginfo>::const_iterator itr = timinglist_.cbegin(); itr != timinglist_.cend(); ++itr)
    {
        int colnum  =   0;
        ui->timinginfo->setItem(rownum,colnum++,new QTableWidgetItem(itr.value().start_.toString("yy-M-d h:m:s")));
        ui->timinginfo->setItem(rownum,colnum++,new QTableWidgetItem(itr.value().end_.toString("yy-M-d h:m:s")));
        ui->timinginfo->setItem(rownum,colnum++,new QTableWidgetItem(itr.value().musicname_));
        ui->timinginfo->setItem(rownum,colnum++,new QTableWidgetItem(itr.value().ControlMode_));
        ++rownum;
    }


//    foreach (timinginfo, timinglist_)
//    {
//        int rownum  =   0;
//        int colnum  =   0;
//        ui->timinginfo->setItem(rownum,colnum++,new QTableWidgetItem(tr("%1").arg(timinginfo.start_.toString("yy-m-d h:m:s"))));
//        ui->timinginfo->setItem(rownum,colnum++,new QTableWidgetItem(tr("%1").arg(timinginfo.end_.toString("yy-m-d h:m:s"))));
//        ui->timinginfo->setItem(rownum,colnum++,new QTableWidgetItem(tr("%1").arg(timinginfo.musicname_)));
//        ui->timinginfo->setItem(rownum,colnum++,new QTableWidgetItem(tr("%1").arg(timinginfo.ControlMode_)));
    //    }
}

void playtiming::remove(QDate date, QDateTime start, QDateTime end)
{
    if(timinglist_.contains(date))
    {
        QList<timinginfo> infolist  =    timinglist_.values(date);
        foreach (timinginfo ti, infolist)
        {
            if(ti.start_==start&&ti.end_==end)
            {
                timinglist_.remove(date,ti);
            }
        }
    }
}

void playtiming::remove(QDate date, int index)
{
    if(timinglist_.contains(date))
    {
        QList<timinginfo> infolist  =    timinglist_.values(date);
        int curindex    =   0;
        foreach (timinginfo ti, infolist)
        {
            if(curindex==index)
            {
                timinglist_.remove(date,ti);
            }
            curindex++;
        }
    }
}

void playtiming::on_currentDate_clicked()
{
    //数据校验
    if(ui->listWidget->currentItem()==nullptr)
    {
        return;
    }
    if(ui->starttimeEdit->time()>ui->endtimeEdit->time())
    {
        return;
    }

    timinginfo  info;
    QDate seldate   =   ui->calendarWidget->selectedDate();
    info.start_ =   QDateTime(seldate,ui->starttimeEdit->time());
    info.end_ =   QDateTime(seldate,ui->endtimeEdit->time());
    info.musicname_ =   ui->listWidget->currentItem()->text();
    info.ControlMode_   =   ui->ControlMode->currentText();
    timinglist_.insert(ui->calendarWidget->selectedDate(),info);

    updateplaylist(seldate);
}

void playtiming::on_pushButton_2_clicked()
{
    if(ui->listWidget->currentItem()==nullptr)
    {
        return;
    }
    if(ui->starttimeEdit->time()>ui->endtimeEdit->time())
    {
        return;
    }

    QDate itr   =   ui->calendarWidget->minimumDate();
    while (itr<=ui->calendarWidget->maximumDate())
    {
        timinginfo  info;
        QDate seldate   =   ui->calendarWidget->selectedDate();
        info.start_ =   QDateTime(itr,ui->starttimeEdit->time());
        info.end_ =   QDateTime(itr,ui->endtimeEdit->time());
        info.musicname_ =   ui->listWidget->currentItem()->text();
        info.ControlMode_   =   ui->ControlMode->currentText();

        if(ui->MONcheckBox->isChecked()&&itr.dayOfWeek()==1)
        {
            timinglist_.insert(itr,info);
        }
        else if (ui->TUEcheckBox->isChecked()&&itr.dayOfWeek()==2)
        {
            timinglist_.insert(itr,info);
        }
        else if (ui->WEDcheckBox->isChecked()&&itr.dayOfWeek()==3)
        {
            timinglist_.insert(itr,info);
        }
        else if (ui->THUcheckBox->isChecked()&&itr.dayOfWeek()==4)
        {
            timinglist_.insert(itr,info);
        }
        else if (ui->FRIcheckBox->isChecked()&&itr.dayOfWeek()==5)
        {
            timinglist_.insert(itr,info);
        }
        else if (ui->SATcheckBox->isChecked()&&itr.dayOfWeek()==6)
        {
            timinglist_.insert(itr,info);
        }
        else if (ui->SUNcheckBox->isChecked()&&itr.dayOfWeek()==7)
        {
            timinglist_.insert(itr,info);
        }

        itr =   itr.addDays(1);
    }
}


void playtiming::on_calendarWidget_clicked(const QDate &date)
{
    if(timinglist_.contains(date))
    {
        updateplaylist(date);
    }
    else
    {
        ui->timinginfo->clearContents();
    }
}

void playtiming::on_pushButton_clicked()
{
    timinglist_.clear();
    if(ui->calendarWidget->selectedDate().isValid())
    {
        updateplaylist(ui->calendarWidget->selectedDate());
    }
}


void playtiming::on_rowselected(int rownum)
{
    qDebug()<<"rownum is "<<rownum;
}

void playtiming::on_pushButton_3_clicked()
{
    QModelIndex selindex    =   ui->timinginfo->verticalHeader()->currentIndex();
    if(selindex.isValid())
    {
        ui->timinginfo->removeRow(selindex.row());
    }
    remove(ui->calendarWidget->selectedDate(),selindex.row());
}
