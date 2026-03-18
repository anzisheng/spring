#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QDialog>
#include "globalparams.h"

class QCPAxisRect;
class QCPGraph;

namespace Ui {
class spectrumwidget;
}

class spectrumwidget : public QDialog
{
    Q_OBJECT

public:
    explicit spectrumwidget(QWidget *parent = 0);
    ~spectrumwidget();
    void setLabel(QString szLabel);
    void setLabelWidth(int width);
    void SetXMaxToGraph(unsigned int iMaxX);
public:
    void drawmusic(QString path);
    void addWaveData(const QVector<double>& x, const QVector<double>& yLeft, const QVector<double>& yRight);
private:
    Ui::spectrumwidget *ui;
    unsigned int iCurrentXPosOnGraph;
    QCPAxisRect* m_topRect;
    QCPAxisRect* m_bottomRect;
    QCPGraph* m_graphLeft;
    QCPGraph* m_graphRight;

public slots:
    void slotAddDataToGraph(float* pData, unsigned int iSizeInSamples,  unsigned int iSamplesInOne);
    void setslotMaxspec(unsigned int iTempMax);
};

#endif // SPECTRUMWIDGET_H
