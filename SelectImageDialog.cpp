#include "SelectImageDialog.h"
#include "ui_SelectImageDialog.h"
#include <QFileDialog>
#include <QFontDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QRgb>
#include <QDebug>
QString imgfolder = "pictures";
SelectImageDialog::SelectImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectImageDialog)
{
    ui->setupUi(this);
    m_ImagePath = "";
    m_LoopTime = 0;
    m_DelayTime = 0;
    m_IsYangWen = true;
    ui->reverse->setText(tr("阳文"));
    ui->scanTime->setText("10");
    ui->loop_time->setText("-1");

    //创建默认图片目录
    QDir curdir  =   QDir::current();
    if(!curdir.exists(imgfolder))
    {
        curdir.mkdir(imgfolder);
    }
}

SelectImageDialog::~SelectImageDialog()
{
    delete ui;
}

void SelectImageDialog::on_textFormat_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(
                &ok, QFont( "Helvetica [Cronyx]", 10 ), this );
    if ( ok ) {
        // font被设置为用户选择的字体
    } else {
        // 用户取消这个对话框，font被设置为初始值，在这里就是Helvetica [Cronyx], 10
    }

}

void SelectImageDialog::setData(QString qStrPath,int delay_time,
                     int loop_time,bool bIsYangWen)
{
    m_ImagePath = qStrPath;
    ui->text->setText(m_ImagePath);
    m_DelayTime = delay_time;
    ui->scanTime->setText(QString::number(m_DelayTime));
    m_LoopTime = loop_time;
    ui->loop_time->setText(QString::number(m_LoopTime));
    if(bIsYangWen)
        ui->reverse->setText(tr("阳文"));
    else
        ui->reverse->setText(tr("阴文"));
    loadImage();
}

void SelectImageDialog::loadImage()
{
 //   QImage* img=new QImage();
    if(! ( m_image.load(m_ImagePath) ) ) //加载图像
    {
        QMessageBox::information(this,
                                 tr("打开图像失败"),
                                 tr("打开图像失败!"));
        return;
    }
    ui->showImage->setPixmap(QPixmap::fromImage(m_image));
}

void SelectImageDialog::on_addImage_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "./pic", tr("Image Files (*.png *.jpg *.bmp)"));

    if(fileName.isEmpty())
    {
        return;
    }
    //默认图片所在路径
    QDir picfolder  =   QDir::current();
    picfolder.cd(imgfolder);                    //肯定会存在 因为已经在构造函数中创建

    if(!fileName.contains(picfolder.dirName()))
    {
        //用户选择了其他路径
        QMessageBox::information(this, tr("提示"), tr("请在pictures目录中选择图片"));
        return;
    }
    QFileInfo imginfo(fileName);
    QDir  imgdir    =   imginfo.dir();
    while(imgdir.dirName()!=imgfolder)
    {
        imgdir.cdUp();
    }
    imgdir.cdUp();
    fileName.replace(imgdir.absolutePath(),"");
    fileName.prepend(".");
    if(!fileName.isEmpty())
        m_ImagePath = fileName;
    ui->text->setText(m_ImagePath);
    loadImage();
}

void SelectImageDialog::on_reverse_clicked()
{
    if(ui->reverse->text().compare(tr("阴文")) == 0)
    {
        m_IsYangWen = true;
        ui->reverse->setText(tr("阳文"));
    }
    else
    {
        m_IsYangWen = false;
        ui->reverse->setText(tr("阴文"));
    }
}

void SelectImageDialog::on_buttonBox_accepted()
{
    m_ImagePath = ui->text->text();
	if (m_ImagePath.isEmpty())
	{
		QMessageBox::information(this, "路径", "请选择图片");
	}
    m_DelayTime = ui->scanTime->text().toInt();
    m_LoopTime = ui->loop_time->text().toInt();
}

void SelectImageDialog::on_verticalSlider_valueChanged(int value)
{
    ui->spinBox->setValue(value);
    int height = m_image.height();
    height = height*value/100;
 //   QImage new_image = m_image.scaled(100,299,Qt::IgnoreAspectRatio);
    QImage new_image = m_image.scaledToHeight(height,Qt::SmoothTransformation);
    ui->showImage->setPixmap(QPixmap::fromImage(new_image));

}
