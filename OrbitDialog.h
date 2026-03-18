#ifndef ORBITDIALOG_H
#define ORBITDIALOG_H

#include <QDialog>
#include <QMap>
#pragma execution_character_set("utf-8")
namespace Ui {
class OrbitDialog;
}

class OrbitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrbitDialog(int itemID,QWidget *parent = 0);
    ~OrbitDialog();

private:
    void Parse(); //从数据库中读取信息到界面显示
    //得到press所在容器中的ID,如果没有，就新加，并返回ID
    int  getPointID(QPoint press);
    int  addPoint(QPoint press);
    void ReDrawLines(); //重画所有的线
    bool Colliding(QPoint,QPoint); //碰撞检测
    void drawBackGround();  //画轨迹的背景图
    bool IsOutOfOrbitRect(QPoint pos);
    void drawYnumber();
    QMap<int,int> OrbitPoint2TimeValue(QList<QPoint> point_list);
    QList<QPoint> TimeValue2OrbitPoint(QMap<int,int> time_value);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

signals:


public slots:


private slots:
    void on_buttonBox_accepted();

private:
    Ui::OrbitDialog *ui;
    int m_id;   //当前轨迹ID号
    int m_time; //轨迹的时间长度
    QList<QPoint> m_PointList;  //当前轨迹列表的所有坐标点
    QMap<int,int> m_time_value; //所有轨迹坐标对应的时间和输出值
    int m_PressID;   //当前摁下的轨迹节点

    QPoint orbit_begin_point;  //轨迹起始坐标
    int orbit_width;           //轨迹图的宽度
    int orbit_height;          //轨迹图的高度

};

#endif // ORBITDIALOG_H
