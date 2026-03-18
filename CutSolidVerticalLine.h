#include <QGraphicsItem>
#include <QPainter>
#include <QObject>
#include <QPainterPath>
class CutSolidVerticalLine : public QObject, public QGraphicsItem
{
	Q_OBJECT
public:
	enum { Type = UserType + 101 }; // Define the custom type
//	CutSolidVerticalLine(qreal x, qreal y1, qreal y2, QGraphicsItem *parent = nullptr);
	CutSolidVerticalLine(qreal x, qreal y1, qreal y2, qreal time, QGraphicsItem *parent = nullptr);

	int type() const override { return Type; } // Override the type() function to return the custom type
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    qreal getX() const;
	qreal gety1() const;
	qreal gety2() const;
	qreal gettime() const;

	void setX(qreal x);
	void sety1(qreal y1);
	void sety2(qreal y2);
	void settime(qreal time);//竖直红线对应的时间值，单位毫秒
	void setMusiclength(float ftime_length);
protected:
    void keyPressEvent(QKeyEvent *event) override;
    //void keyReleaseEvent(QKeyEvent *event) override;
    QPainterPath shape() const override;

private:
    qreal m_x;
    qreal m_y1;
	qreal m_y2;
	qreal m_time;	//竖直红线对应的时间值，单位毫秒
	float m_timelength;     //当前编曲时长
};
