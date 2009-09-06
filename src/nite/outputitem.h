#pragma once

#include <QtGui>

class ProcessorItem;

class OutputItem: public QGraphicsPolygonItem
{
public:
	OutputItem(int _i, ProcessorItem* _p);

	inline ProcessorItem* processorItem() const;

	enum { Type = UserType + 3 };
	virtual int type() const { return Type; }

private:
	int			m_index;
	QSizeF		m_size;
};
