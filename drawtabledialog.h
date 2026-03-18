#ifndef DRAWTABLEDIALOG_H
#define DRAWTABLEDIALOG_H

#include <QDialog>
#include <QSlider>
#pragma execution_character_set("utf-8")


namespace Ui {
class DrawTableDialog;
}

class DrawTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DrawTableDialog(QWidget *parent = 0);
    ~DrawTableDialog();
signals:
public   slots:
    void setLineWidth(int value);

private slots:
 //   void on_horizontalSlider_actionTriggered(int action);

 //   void on_horizontalSlider_sliderMoved(int position);

    void on_horizontalSlider_valueChanged(int value);

    void on_toolButton_clicked();

private:
    Ui::DrawTableDialog *ui;

    QSlider *slider;


};

#endif // DRAWTABLEDIALOG_H
