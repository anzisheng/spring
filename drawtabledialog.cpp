#include "drawtabledialog.h"
#include "ui_drawtabledialog.h"
#include "dialoglogindraw.h"
#include <QGuiApplication>
#include <QScreen>


DrawTableDialog::DrawTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DrawTableDialog)
{
    ui->setupUi(this);
//    int dlg_width = this->width();
//    int dlg_height = this->height();

    QScreen *screen = QGuiApplication::primaryScreen();
    int width = screen ? screen->geometry().width() : this->width();
    ui->buttonBox->move(width-200,20);             //固定ok控件位置

    ui->horizontalSlider->move(width-800,20);       //固定slider位置
    ui->horizontalSlider->setRange(1,5);

    ui->line_black->move(width-800+ui->horizontalSlider->width()+50,17);   //固定line位置
    QPalette palette = ui->line_black->palette();
    palette.setColor(QPalette::Dark, Qt::black);
    ui->line_black->setPalette(palette);
    ui->line_black->setLineWidth(5);      //缺省的宽度为5，即填充一个格子（应该根据分辨率来确定每个格子的尺寸）

    ui->line_fenge->setMinimumSize(width,4);    //分割线固定
    ui->line_fenge->move(0,100);




 }

DrawTableDialog::~DrawTableDialog()
{
    delete ui;
}

void DrawTableDialog::setLineWidth(int value)
{
      ui->line_black->setLineWidth(value*5);

}


void DrawTableDialog::on_horizontalSlider_valueChanged(int value)
{
    setLineWidth(value);
}

void DrawTableDialog::on_toolButton_clicked()
{
    DialogLoginDraw log_dialog;
    log_dialog.setModal(true);
    log_dialog.exec();
}
