#include "ClockDialog.h"
#include "ui_ClockDialog.h"
#include <QFileDialog>
#include <QFontDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QRgb>
#include <QDebug>
#include "qpainter.h"
#include <QTime>

//QString imgfolder = "pictures";
ClockDialog::ClockDialog(QWidget *parent) :
QDialog(parent),
ui(new Ui::ClockDialog)
{
	ui->setupUi(this);
//	m_ImagePath = "";

	m_LoopTime = -1;		//循环次数
	m_DelayTime = 10;		//扫描时间
	m_IsYangWen = true;		//阳文

	m_ClockStyleSel = 0;	//时钟样式，缺省0，表盘模式
	m_TimeLeadValue = 0;	//时间提前量，暂时缺省0ms

	ui->reverse->setText(tr("阳文"));
	ui->scanTime->setText("10");
	ui->loop_time->setText("-1");

	ui->TimeLeadValue->setText("0");

	ui->ClockStyle_select->addItem("传统表盘样式");
	ui->ClockStyle_select->addItem("数字样式");
	ui->ClockStyle_select->setCurrentIndex(0);

	m_ClockTimer = new QTimer(this);
	connect(m_ClockTimer, SIGNAL(timeout()), this, SLOT(ClockTimeOut()));
	m_ClockTimer->start(1000);

	//创建默认图片目录
/*	QDir curdir = QDir::current();
	if (!curdir.exists(imgfolder))
	{
		curdir.mkdir(imgfolder);
	} */
}

ClockDialog::~ClockDialog()
{
	m_ClockTimer->stop();

    delete ui;
}

void ClockDialog::on_textFormat_clicked()
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

void ClockDialog::setData(int clockstylesel, int timeleadvalue, int delay_time,		//初始化对话框内容(再编辑的时候)
	int loop_time, bool bIsYangWen)
//void ClockDialog::setData(QString qStrPath, int delay_time,
//                     int loop_time,bool bIsYangWen)
{
//	m_ImagePath = qStrPath;
//	ui->text->setText(m_ImagePath);

	m_ClockStyleSel = clockstylesel;
	ui->ClockStyle_select->setCurrentIndex(m_ClockStyleSel);

	m_TimeLeadValue = timeleadvalue;
	ui->TimeLeadValue->setText(QString::number(m_TimeLeadValue));

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

void ClockDialog::loadImage()
{
 //   QImage* img=new QImage();
/*	if(! ( m_image.load(m_ImagePath) ) ) //加载图像
	{
		QMessageBox::information(this,
								tr("打开图像失败"),
								tr("打开图像失败!"));
		return;
	}
	ui->showImage->setPixmap(QPixmap::fromImage(m_image));  */
}

/*
void ClockDialog::on_addImage_clicked()
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
} */

void ClockDialog::on_reverse_clicked()
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

void ClockDialog::on_buttonBox_accepted()
{
/*	m_ImagePath = ui->text->text();
	if (m_ImagePath.isEmpty())
	{
		QMessageBox::information(this, "路径", "请选择图片");
	} */
//	m_ClockStyleSel = ui->scanTime->text().toInt();			//扫描时间
	m_TimeLeadValue = ui->TimeLeadValue->text().toInt();	//时间提前量，单位ms

    m_DelayTime = ui->scanTime->text().toInt();		//扫描时间
    m_LoopTime = ui->loop_time->text().toInt();		//循环次数
}

void ClockDialog::on_verticalSlider_valueChanged(int value)
{
	ui->spinBox->setValue(value);
	int height = m_image.height();
	height = height*value/100;
//	QImage new_image = m_image.scaled(100,299,Qt::IgnoreAspectRatio);
	QImage new_image = m_image.scaledToHeight(height,Qt::SmoothTransformation);
	ui->showImage->setPixmap(QPixmap::fromImage(new_image));
}

void ClockDialog::on_ClockStyle_select_currentIndexChanged(const QString &arg1)
{
	//dmx dominuo
	if (arg1.compare("传统表盘样式") == 0)
	{
		m_ClockStyleSel = 0;
	}
	else if (arg1.compare("数字样式") == 0)
	{
		m_ClockStyleSel = 1;
	}
}

//这个函数用于在对话框内预览钟表样式
void ClockDialog::ClockTimeOut()
{
//	int width = this->width();
//	int height = this->height();
//-----------------------------------------------------------------------------------------
//绘制到界面窗口
//-----------------------------------------------------------------------------------------

/*	QPen thickPen(palette().windowText(), 1.5);
	QPen thinPen(palette().windowText(), 0.5);

	//绘制准备工作,启用反锯齿,启用图片平滑缩放
	QPainter painter(this);

	const int clockSize = 400;
	QImage clock(clockSize, clockSize, QImage::Format_RGB32);
	clock.fill(Qt::white); // 白底
	QPainter painterbmp(&clock);

	painterbmp.setRenderHint(QPainter::Antialiasing, true);
	int side = qMin(width(), height());
	//qDebug()<<QString::number(side);//300
	painterbmp.setViewport((width() - side) / 2, (height() - side) / 2, side, side);
	painterbmp.setWindow(0, 0, 200, 200);

	//	QPen thickPen(palette().windowText(), 1.5);
	//	QPen thinPen(palette().windowText(), 0.5);
	for (int i = 1; i <= 60; ++i)
	{
		painterbmp.save();
		painterbmp.translate(100, 100);
		painterbmp.rotate(6 * i);
		if (i % 5 == 0)
		{
			painterbmp.setPen(thickPen);
			painterbmp.drawLine(0, -98, 0, -82);
			painterbmp.drawText(-20, -82, 40, 40, Qt::AlignHCenter | Qt::AlignTop, QString::number(i / 5));
		}
		else
		{
			painterbmp.setPen(thinPen);
			painterbmp.drawLine(0, -98, 0, -88);
		}
		painterbmp.restore();
	}

	draw(&painterbmp);  */

/*	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	int side = qMin(width(), height());
	//qDebug()<<QString::number(side);//300
	painter.setViewport((width() - side) / 2, (height() - side) / 2, side, side);
	painter.setWindow(0, 0, 200, 200);

	QPen thickPen(palette().windowText(), 1.5);
	QPen thinPen(palette().windowText(), 0.5);
	for (int i = 1; i <= 60; ++i)
	{
		painter.save();
		painter.translate(100, 100);
		painter.rotate(6 * i);
		if (i % 5 == 0)
		{
			painter.setPen(thickPen);
			painter.drawLine(0, -98, 0, -82);
			painter.drawText(-20, -82, 40, 40, Qt::AlignHCenter | Qt::AlignTop, QString::number(i / 5));
		}
		else
		{
			painter.setPen(thinPen);
			painter.drawLine(0, -98, 0, -88);
		}
		painter.restore();
	}

	draw(&painter); */

//-----------------------------------------------------------------------------------------
//绘制到文件
//-----------------------------------------------------------------------------------------
	ui->showImage->width();

	const int clockSize = 400;
	QImage clock(clockSize, clockSize, QImage::Format_RGB32);
	clock.fill(Qt::white); // 白底
	QPainter painterbmp(&clock);

	painterbmp.setRenderHint(QPainter::Antialiasing, true);
	int side = qMin(width(), height());
	//qDebug()<<QString::number(side);//300
	painterbmp.setViewport((width() - side) / 2, (height() - side) / 2, side, side);
	painterbmp.setWindow(0, 0, 200, 200);

	QPen thickPen(palette().windowText(), 1.5);
	QPen thinPen(palette().windowText(), 0.5);
	for (int i = 1; i <= 60; ++i)
	{
		painterbmp.save();
		painterbmp.translate(100, 100);
		painterbmp.rotate(6 * i);
		if (i % 5 == 0)
		{
			painterbmp.setPen(thickPen);
			painterbmp.drawLine(0, -98, 0, -82);
			painterbmp.drawText(-20, -82, 40, 40, Qt::AlignHCenter | Qt::AlignTop, QString::number(i / 5));
		}
		else
		{
			painterbmp.setPen(thinPen);
			painterbmp.drawLine(0, -98, 0, -88);
		}
		painterbmp.restore();
	}

	draw(&painterbmp);

	clock.save("Clock.BMP");

	m_ImagePath = "Clock.BMP";
	m_image.load(m_ImagePath);

	int height = m_image.height();
	QImage new_image = m_image.scaledToHeight(height, Qt::SmoothTransformation);
	ui->showImage->setPixmap(QPixmap::fromImage(new_image));

//	ui->showImage->setPixmap(QPixmap::fromImage(m_image));
}

void ClockDialog::draw(QPainter *painter)
{
	static const QPointF sed[4] = { //note the element range!!
		QPointF(0.0, 5.0),
		QPointF(-2.5, 0.0),
		QPointF(0.0, -80.0),
		QPointF(2.5, 0.0)
	};
	static const QPointF min[4] = {
		QPointF(0.0, 5.0),
		QPointF(-5.0, 0.0),
		QPointF(0.0, -60.0),
		QPointF(5.0, 0.0)

	};
	static const QPointF hour[4] = {
		QPointF(0.0, 5.0),
		QPointF(-5.0, 0.0),
		QPointF(0.0, -50.0),
		QPointF(5.0, 0.0)
	};
	//QPen thickPen(palette().windowText(),1.5);
	//QPen thinPen(palette().windowText(),0.5);

	QTime time = QTime::currentTime();
	painter->translate(100, 100);
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setBrush(Qt::red);
	painter->setPen(Qt::red);
	painter->save();
	painter->rotate(6.0*time.second());
	painter->drawConvexPolygon(sed, 4);
	painter->restore();

	painter->setBrush(Qt::blue);
	painter->setPen(Qt::blue);
	painter->save();
	painter->rotate(6.0*(time.minute() + time.second() / 60));
	painter->drawConvexPolygon(min, 4);
	painter->restore();

	painter->setBrush(Qt::black);
	painter->setPen(Qt::black);
	painter->save();
	painter->rotate(30.0*(time.hour() + time.minute() / 60.0));
	painter->drawConvexPolygon(hour, 4);
	painter->restore();
}
