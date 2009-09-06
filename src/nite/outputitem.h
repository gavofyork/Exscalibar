#pragma once

#include <QtGui>

class ProcessorItem;

class OutputItem: public QGraphicsPolygonItem
{
public:
	OutputItem(int _i, ProcessorItem* _p);

	ProcessorItem* processorItem() const;
	inline uint index() const { return m_index; }

	enum { Type = UserType + 3 };
	virtual int type() const { return Type; }

private:
	uint			m_index;
	QSizeF			m_size;
};
