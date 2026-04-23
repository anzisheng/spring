#pragma once
#include <QGraphicsItem>

#include <QPainter>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>

class MovableVerticalLine : public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem) // Add this line

public:
	enum { Type = UserType + 200 }; // Define the custom type
    MovableVerticalLine(qreal x, qreal y1, qreal y2, QGraphicsItem *parent = nullptr);
	
	int type() const override { return Type; } // Override the type() function to return the custom type
    
	QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    qreal getX() const;
	qreal gety1() const;
	qreal gety2() const;
	void setX(qreal posx);
	void sety1(qreal posy1);
	void sety2(qreal posy2);
	void setlineMaxX(qreal posmaxx);
	qreal getlineMaxX(void);
	void setTimeText(qreal musicLength); //用于显示移动参考线 和 音乐的时间点， 很重要，作曲的时候用到
	void setMusiclength(float ftime_length);

protected:
    void keyPressEvent(QKeyEvent *event) override;
//	void keyReleaseEvent(QKeyEvent *event) override;

private:
    qreal m_x;
    qreal m_y1;
    qreal m_y2;
	qreal m_maxx;
	qreal maxx;

	QGraphicsSimpleTextItem *textmusictimeitem; //显示当前位置对应的音乐
	QGraphicsSimpleTextItem *textmusictimeitem1; //显示当前位置对应的音乐

	float m_timelength;     //当前编曲时长
signals:
	void signalcutsolid(float position);
	//void signaldrawLineAtPosition(float xPosition); //发送播放位置
};
