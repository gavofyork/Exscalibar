#pragma once

#include <QtGui>

class ProcessorItem;

class OutputItem: public QGraphicsItem
{
public:
	OutputItem(int _i, ProcessorItem* _p);

	ProcessorItem* processorItem() const;
	inline uint index() const { return m_index; }

	inline InputItem* inputItem() const { return m_inputItem; }
	inline void setInputItem(InputItem* _ii = 0) { m_inputItem = _ii; }

	QPointF tip() const;

	enum { Type = UserType + 3 };
	virtual int type() const { return Type; }

private:
	QPolygonF polygon() const;

	virtual QRectF boundingRect() const { return polygon().boundingRect(); }
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent*);

	uint			m_index;
	bool			m_hover;
	InputItem*		m_inputItem;
};
