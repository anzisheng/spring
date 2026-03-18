#include "OrbitDialog.h"
#include "ui_OrbitDialog.h"
#include "StaticValue.h"
#include "ClassDefined.h"
#include <QMap>
#include <QPalette>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>

OrbitDialog::OrbitDialog(int itemID,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OrbitDialog)
{
    ui->setupUi(this);

    m_PressID = 0;
    orbit_begin_point.setX(40);
    orbit_begin_point.setY(140);
    orbit_width = 600;
    orbit_height = 300;

    m_id = itemID;
    Parse();

    //起始点的坐标
    QPoint begin_pos(orbit_begin_point.x(),
                     orbit_begin_point.y() + orbit_height);
    //终点的坐标
    QPoint end_pos(orbit_begin_point.x() + orbit_width,
                   orbit_begin_point.y() + orbit_height);
    if(m_PointList.size() < 2)
        m_PointList.push_back(begin_pos);
    if(m_PointList.size() < 2)
        m_PointList.push_back(end_pos);
}

OrbitDialog::~OrbitDialog()
{
    delete ui;
}

void OrbitDialog::Parse()
{
    if(StaticValue::GetInstance()->m_show_orbit.contains(m_id))
    {
        Orbit* orbit_info =
                StaticValue::GetInstance()->m_show_orbit.value(m_id);
        ui->id->setValue(orbit_info->id);
        ui->name->setText(orbit_info->name);
        ui->time_length->setText(
                    QString::number(orbit_info->time_length));
        m_time = orbit_info->time_length;
        ui->delay_time->setText(
                    QString::number(orbit_info->delay_time));
        if(orbit_info->reversal)
            ui->reversal->setChecked(true);
        if(orbit_info->reversal_output)
            ui->reversal_ouput->setChecked(true);
        m_PointList = TimeValue2OrbitPoint(orbit_info->show_orbit);
    }
}


void OrbitDialog::paintEvent(QPaintEvent* event)
{
    drawBackGround();
    ReDrawLines();
}

void OrbitDialog::mousePressEvent(QMouseEvent *event)
{
    QPoint press_pos = event->pos();
    if(event->button() == Qt::LeftButton)
    {
        if(!IsOutOfOrbitRect(press_pos))
        {
            m_PressID = addPoint(press_pos);
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        if(!IsOutOfOrbitRect(press_pos))
        {
            int index = getPointID(press_pos);
            if(index != -1 && index != 0 &&
               index != m_PointList.size()-1)
            {
                m_PointList.removeAt(index);
                m_PressID = 0;
            }
        }
    }
    update();
    QDialog::mousePressEvent(event);
}

void OrbitDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        QPoint release_pos = event->pos();
        if(!IsOutOfOrbitRect(release_pos))
        {
            addPoint(release_pos);
            update();  //这个是qwidget类的Public Slots槽。
        }
    }
}

void OrbitDialog::mouseMoveEvent(QMouseEvent *event)
{
    //限定移动区域
    QPoint pos = event->pos();
    if(pos.x() < orbit_begin_point.x())
        pos.setX(orbit_begin_point.x());
    if(pos.x() > orbit_begin_point.x() + orbit_width)
        pos.setX(orbit_begin_point.x() + orbit_width);
    if(pos.y() < orbit_begin_point.y())
        pos.setY(orbit_begin_point.y());
    if(pos.y() > orbit_begin_point.y() + orbit_height)
        pos.setY(orbit_begin_point.y() + orbit_height);

    int iSize = m_PointList.size() - 1;
    if(m_PressID == 0 || m_PressID == iSize)
    {
        m_PointList[m_PressID].setY(pos.y());
    }
    else
    {
        if(m_PointList[m_PressID-1].x() > pos.x())
        {
            if(m_PressID == 1)
            {
                m_PointList[m_PressID].setX(m_PointList[0].x());
                m_PointList[m_PressID].setY(pos.y());
            }
            else
            {
                QPoint temp = m_PointList[m_PressID-1];
                m_PointList[m_PressID-1] = pos;
                m_PointList[m_PressID] = temp;
                m_PressID--;
            }
        }
        else if(m_PointList[m_PressID+1].x() < pos.x())
        {
            if(m_PressID == iSize - 1)
            {
                m_PointList[m_PressID].setX(m_PointList[iSize].x());
                m_PointList[m_PressID].setY(pos.y());
            }
            else
            {
                QPoint temp = m_PointList[m_PressID+1];
                m_PointList[m_PressID+1] = pos;
                m_PointList[m_PressID] = temp;
                m_PressID++;
            }

        }
        else
        {
            if(pos.y() < orbit_begin_point.y())
            {
                m_PointList[m_PressID].
                        setY(orbit_begin_point.y());
                m_PointList[m_PressID].setX(pos.x());
            }
            else if(pos.y() > orbit_begin_point.y() + orbit_height)
            {
                m_PointList[m_PressID].
                        setY(orbit_begin_point.y() + orbit_height);
                m_PointList[m_PressID].setX(pos.x());
            }
            else
                m_PointList[m_PressID] = pos;
        }
    }
    update();  //这个是qwidget类的Public Slots槽
}

void OrbitDialog::drawBackGround()
{
    drawYnumber();
}

//Y轴百分比和10行矩形
void OrbitDialog::drawYnumber()
{
    QPainter paint(this);
    QBrush brush;
    m_time = ui->time_length->text().toInt();
    int row_height = orbit_height/10;
    int time_width = orbit_width / 10;  //每段时间轴的长度
    for(int i = 0,x = 0,y = 100; i < 10; ++i,y-=10,x+=m_time/10)
    {
        QString qStrYText = QString("%1%").arg(y);
        QString qStrXText = QString("%1").arg(x);
        brush.setStyle(Qt::SolidPattern);
        if(i%2 == 0)
            brush.setColor(QColor(Qt::white));
        else
            brush.setColor(QColor(220,252,252));
        QRect rect(orbit_begin_point.x(),
                   orbit_begin_point.y() + i*row_height,
                   orbit_width,row_height);
        paint.fillRect(rect,brush);
        paint.drawRect(rect);

        //Y轴百分比文本
        paint.drawText(QPointF(orbit_begin_point.x()-25,
                               orbit_begin_point.y() + i*row_height),
                       qStrYText);
        //X轴时间文本
        paint.drawText(QPointF(orbit_begin_point.x() + i*time_width,
                               orbit_begin_point.y() + orbit_height+15),
                       qStrXText);
    }
}

QList<QPoint> OrbitDialog::TimeValue2OrbitPoint(QMap<int,int> time_value)
{
    QList<QPoint> point_list;
    float light_value = 255.0;
    float timeLength = m_time;   //总时长
    float time_balance =  orbit_width / timeLength ;  //每毫秒对应的像素长度
    float value_balance = orbit_height / light_value ;//0-255每个值对应的像素长度
    QList<int> keys = time_value.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        int time = keys.at(i);
        int value = time_value[time];
        int pos_x = time*time_balance + 40;
        int pos_y = 140 + (orbit_height - value*value_balance);
        point_list.push_back(QPoint(pos_x,pos_y));
    }
    return point_list;
}

QMap<int, int> OrbitDialog::OrbitPoint2TimeValue(QList<QPoint> point_list)
{
    QMap<int,int> time_value;
    float light_value = 255.0;
    float timeLength = m_time;   //总时长
    float time_balance = timeLength / orbit_width;  //每个像素对应的的时间长度
    float value_balance = light_value / orbit_height;//每个像素点对应的0~255的值
    int iSize = point_list.size();     //所有的轨迹点
    for(int i = 0; i < iSize; ++i)
    {
        QPoint point = point_list.at(i);
        int x = point.x();
        int y = point.y();
        //当前轨迹节点的x轴坐标减去起始点坐标40，就是他实际时间长度
        int time = (x-40)*time_balance;
        //当前轨迹节点y轴坐标减去起始坐标140，就是他实际的value值，从上到下为255-0
        int value = light_value - (y-140)*value_balance;
        time_value[time] = value;
    }
    return time_value;
}

void OrbitDialog::ReDrawLines()
{
    QPainter paint(this);
    paint.setPen(QPen(QBrush(QColor(Qt::blue)),2));
    int point_size = m_PointList.size();
    for (int j= 0;j<point_size-1;j++){
        paint.drawEllipse(m_PointList[j],6,6);
        paint.drawLine(m_PointList[j], m_PointList[j+1]);
    }
    if(point_size > 1)
        paint.drawEllipse(m_PointList[point_size-1],6,6);
}

bool OrbitDialog::Colliding(QPoint point1, QPoint point2)
{
    if((point1.x() > point2.x() - 6) &&
            (point1.x() < point2.x() + 6) &&
            (point1.y() > point2.y() - 6) &&
            (point1.y() < point2.y() + 6))
        return true;
    return false;
}

int OrbitDialog::getPointID(QPoint press)
{
    int iSize = m_PointList.size();
    for(int i = 0; i < iSize; ++i)
    {
        QPoint point = m_PointList.at(i);
        if(Colliding(point,press))  //找到当前节点，就返回该节点的ID值
        {
            return i;
        }
    }
    return -1;
}

int OrbitDialog::addPoint(QPoint press)
{
    if(m_PointList.isEmpty())
    {
        m_PointList.append(press);
        return m_PointList.size() - 1;
    }
    int index = getPointID(press);
    if(index != -1)
        return index;
    int iSize = m_PointList.size();
    for(int i = 0; i < iSize; ++i)
    {
        QPoint point = m_PointList.at(i);
        if(point.x() > press.x())  //如果当前节点x值大于鼠标摁下节点，
        {
            m_PointList.insert(i,press);
            return i;
        }
        if(i == (iSize - 1))  //当到达列表最后且上两个条件成立，则加到列表最后端
        {
            m_PointList.append(press);
            return i+1;
        }
        //如果鼠标摁下节点x值比当前节点大，但比下个节点x值小，则插入当前节点之后
        if(point.x() < press.x() &&
                m_PointList.at(i+1).x() > press.x())
        {
            m_PointList.insert(i+1,press);
            return i+1;
        }
    }
    return 0;
}

bool OrbitDialog::IsOutOfOrbitRect(QPoint pos)
{
    if(pos.x() > orbit_begin_point.x()-3 &&
            pos.x() < orbit_begin_point.x() + orbit_width + 3 &&
            pos.y() > orbit_begin_point.y() - 3 &&
            pos.y() < orbit_begin_point.y() + orbit_height + 3)
    {
        return false;
    }
    return true;
}

void OrbitDialog::on_buttonBox_accepted()
{
    int id = ui->id->text().toInt();
    QString name = ui->name->text();
    int time_length = ui->time_length->text().toInt();
    int delay_time = ui->delay_time->text().toInt();
    bool reversal = false;
    bool reversal_output = false;
    if(ui->reversal->isChecked())
        reversal = true;
    if(ui->reversal_ouput->isChecked())
        reversal_output = true;

    QString qStrSql = QString("select name from orbit where id = %1").arg(id);
    QSqlQuery query;
    if(!query.exec(qStrSql))
    {
        qDebug() <<"find orbit false! sql:"<<qStrSql<<"error:"<<query.lastError().text();
        return ;
    }
    if(query.next())
    {
        qStrSql = QString("update orbit set name = '%1',time_length = %2,delay_time = %3,"
                          "reversal = %4,reversal_ouput = %5 where id = %6").arg(name).
                arg(time_length).arg(delay_time).arg(reversal).arg(reversal_output).arg(id);
        if(!query.exec(qStrSql))
        {
            qDebug() <<"update orbit false! sql:"<<qStrSql<<"error:"<<query.lastError().text();
            return ;
        }
    }
    else
    {
        qStrSql = QString("insert into orbit(id,name,time_length,delay_time,reversal,reversal_ouput)"
                          " values('%1',%2,%3,%4,%5,%6)").arg(name).arg(time_length).
                arg(delay_time).arg(reversal).arg(reversal_output).arg(id);
        if(!query.exec(qStrSql))
        {
            qDebug() <<"insert orbit false! sql:"<<qStrSql<<"error:"<<query.lastError().text();
            return ;
        }
    }
    qStrSql = QString("delete from time_orbit where orbit_id = %1").arg(id);
    if(!query.exec(qStrSql))
    {
        qDebug() <<"update orbit false! sql:"<<qStrSql<<"error:"<<query.lastError().text();
        return ;
    }
    QMap<int,int> time_value = OrbitPoint2TimeValue(m_PointList);
    QList<int> keys = time_value.keys();
    for(int i = 0 ; i < keys.size();++i)
    {
        int time = keys.at(i);
        int value = time_value[time];
        qStrSql = QString("insert into time_orbit(orbit_id,time,proportion)"
                          "values(%1,%2,%3)").arg(id).arg(time).arg(value);
        if(!query.exec(qStrSql))
        {
            qDebug() <<"update orbit false! sql:"<<qStrSql
                    <<"error:"<<query.lastError().text();
            return ;
        }
    }
//    for(int i = 0; i < m_PointList.size(); ++i)
//    {
//        qStrSql = QString("insert into time_orbit(orbit_id,time,proportion)"
//                          "values(%1,%2,%3)").arg(id).arg(m_PointList.at(i).x()).
//                arg(m_PointList.at(i).y());
//        if(!query.exec(qStrSql))
//        {
//            qDebug() <<"update orbit false! sql:"<<qStrSql
//                    <<"error:"<<query.lastError().text();
//            return ;
//        }
//    }
    StaticValue::GetInstance()->m_show_orbit[id]->show_orbit = time_value;
}
