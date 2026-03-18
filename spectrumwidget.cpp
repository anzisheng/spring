#include "spectrumwidget.h"
#include "ui_spectrumwidget.h"

spectrumwidget::spectrumwidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::spectrumwidget)
{
    ui->setupUi(this);
    if (ui->gridLayout)
    {
        ui->gridLayout->setContentsMargins(0, 0, 0, 0);
    }
    if (ui->horizontalLayout)
    {
        ui->horizontalLayout->setContentsMargins(0, 0, 0, 0);
        ui->horizontalLayout->setSpacing(0);
    }
    ui->sp_label->setFixedWidth(150);
    ui->sp_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Graph
    m_topRect = new QCPAxisRect(ui->widget_graph);
    m_bottomRect = new QCPAxisRect(ui->widget_graph);
    ui->widget_graph->plotLayout()->clear();
    ui->widget_graph->plotLayout()->addElement(0, 0, m_topRect);
    ui->widget_graph->plotLayout()->addElement(1, 0, m_bottomRect);
    ui->widget_graph->plotLayout()->setRowStretchFactor(0, 1);
    ui->widget_graph->plotLayout()->setRowStretchFactor(1, 1);
    ui->widget_graph->plotLayout()->setRowSpacing(0);
    ui->widget_graph->plotLayout()->setColumnSpacing(0);

    QPen pen;
    pen.setWidth(1);
    pen.setColor(QColor(220, 220, 220));
    m_graphLeft = ui->widget_graph->addGraph(m_topRect->axis(QCPAxis::atBottom), m_topRect->axis(QCPAxis::atLeft));
    m_graphLeft->setPen(pen);
    m_graphLeft->setLineStyle(QCPGraph::LineStyle::lsLine);

    QPen penRight = pen;
    penRight.setColor(QColor(156, 220, 254));
    m_graphRight = ui->widget_graph->addGraph(m_bottomRect->axis(QCPAxis::atBottom), m_bottomRect->axis(QCPAxis::atLeft));
    m_graphRight->setPen(penRight);
    m_graphRight->setLineStyle(QCPGraph::LineStyle::lsLine);

    // color and stuff
    ui->widget_graph->setBackground(QColor(30, 30, 30));
    m_topRect->axis(QCPAxis::atBottom)->setRange(0, MAX_X_AXIS_VALUE);
    m_topRect->axis(QCPAxis::atLeft)->setRange(0.0, MAX_Y_AXIS_VALUE);
    m_bottomRect->axis(QCPAxis::atBottom)->setRange(0, MAX_X_AXIS_VALUE);
    m_bottomRect->axis(QCPAxis::atLeft)->setRange(0.0, MAX_Y_AXIS_VALUE);

    m_topRect->axis(QCPAxis::atBottom)->grid()->setVisible(false);
    m_topRect->axis(QCPAxis::atLeft)->grid()->setVisible(false);
    m_bottomRect->axis(QCPAxis::atBottom)->grid()->setVisible(false);
    m_bottomRect->axis(QCPAxis::atLeft)->grid()->setVisible(false);

    m_topRect->axis(QCPAxis::atBottom)->setTicks(false);
    m_topRect->axis(QCPAxis::atLeft)->setTicks(false);
    m_bottomRect->axis(QCPAxis::atBottom)->setTicks(false);
    m_bottomRect->axis(QCPAxis::atLeft)->setTicks(false);

    m_topRect->setAutoMargins(QCP::msNone);
    m_topRect->setMargins(QMargins(0, 0, 0, 0));
    m_bottomRect->setAutoMargins(QCP::msNone);
    m_bottomRect->setMargins(QMargins(0, 0, 0, 0));

	iCurrentXPosOnGraph = 0;
}

spectrumwidget::~spectrumwidget()
{
    delete ui;
}

void spectrumwidget::setLabel(QString szLabel)
{
	// Enable word wrapping
	ui->sp_label->setWordWrap(true);
	ui->sp_label->setToolTip(szLabel);
    ui->sp_label->setText(szLabel);
}

void spectrumwidget::setLabelWidth(int width)
{
	ui->sp_label->setFixedWidth(width);
}

void spectrumwidget::SetXMaxToGraph(unsigned int iMaxX)
{
    if (m_topRect)
    {
        m_topRect->axis(QCPAxis::atBottom)->setRange(0, iMaxX);
    }
    if (m_bottomRect)
    {
        m_bottomRect->axis(QCPAxis::atBottom)->setRange(0, iMaxX);
    }
}

void spectrumwidget::drawmusic(QString path)
{

}
void spectrumwidget::addWaveData(const QVector<double>& x, const QVector<double>& yLeft, const QVector<double>& yRight)
{
    if (m_graphLeft && m_graphRight)
    {
        m_graphLeft->addData(x, yLeft, true);
        m_graphRight->addData(x, yRight, true);
        ui->widget_graph->replot();
    }
}
void spectrumwidget::setslotMaxspec(unsigned int iTempMax)
{
	if (ui != nullptr)
	{
        if (m_topRect)
        {
            m_topRect->axis(QCPAxis::atBottom)->setRange(0, iTempMax);
        }
        if (m_bottomRect)
        {
            m_bottomRect->axis(QCPAxis::atBottom)->setRange(0, iTempMax);
        }
	}
	else
	{
		// Handle the case when ui or ui->widget_graph is null
	}
}
void spectrumwidget::slotAddDataToGraph(float *pData, unsigned int iSizeInSamples, unsigned int iSamplesInOne)
{
	//add by eagle for Draw mp3 spectrum

	QVector<double> x;
	QVector<double> yLeft;
	QVector<double> yRight;

	// Here we need to show only 'iSamplesInOne' samples out of all 'iSizeInSamples'.

	//                                            here '2' because 2 channels (TODO: get rid of this constant value),
	//                                            and '-1' because we read two values in one cycle.
	for (unsigned int i = 0; i < iSizeInSamples - (2 * iSamplesInOne - 1); i += (2 * iSamplesInOne))
	{
		x.push_back(static_cast<double>(iCurrentXPosOnGraph));
		iCurrentXPosOnGraph++;

		float fLeftPlus = -1.0f;
		float fLeftMinus = 1.0f;
		float fRightPlus = -1.0f;
		float fRightMinus = 1.0f;
		bool bLeftPlusFirst = false;
		bool bRightPlusFirst = false;

		// Average all 'iSamplesInOne' samples in one sample.
		for (unsigned int j = 0; j < (2 * iSamplesInOne); j += 2)
		{
			// L + R channels.
			float fLeft = pData[i + j];
			float fRight = pData[i + j + 1];

			if (j == 0)
			{
				bLeftPlusFirst = (fLeft > 0.0f);
				bRightPlusFirst = (fRight > 0.0f);
			}

			if (fLeft > 0.0f)
			{
				if (fLeft > fLeftPlus)
				{
					fLeftPlus = fLeft;
				}
			}
			else
			{
				if (fLeft < fLeftMinus)
				{
					fLeftMinus = fLeft;
				}
			}

			if (fRight > 0.0f)
			{
				if (fRight > fRightPlus)
				{
					fRightPlus = fRight;
				}
			}
			else
			{
				if (fRight < fRightMinus)
				{
					fRightMinus = fRight;
				}
			}
		}

		fLeftPlus *= 0.5f;
		fLeftPlus = 0.5f + fLeftPlus;

		fLeftMinus *= 0.5f;
		fLeftMinus += 0.5f;

		fRightPlus *= 0.5f;
		fRightPlus = 0.5f + fRightPlus;

		fRightMinus *= 0.5f;
		fRightMinus += 0.5f;

		if (bLeftPlusFirst)
		{
			yLeft.push_back(static_cast<double>(fLeftPlus));
		}
		else
		{
			yLeft.push_back(static_cast<double>(fLeftMinus));
		}

		if (bRightPlusFirst)
		{
			yRight.push_back(static_cast<double>(fRightPlus));
		}
		else
		{
			yRight.push_back(static_cast<double>(fRightMinus));
		}
	}

	qDebug() << "x:" << x.size() << "yL:" << yLeft.size() << "yR:" << yRight.size();
    addWaveData(x, yLeft, yRight);

	delete[] pData;
}
