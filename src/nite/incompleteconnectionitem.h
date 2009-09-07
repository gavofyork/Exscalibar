#pragma once

#include <QtGui>
#include <QtXml>

class OutputItem;

class IncompleteConnectionItem: public QGraphicsPathItem
{
public:
	IncompleteConnectionItem(OutputItem* _from);

	void setTo(QPointF _to);
	OutputItem* from() const;

	enum { Type = UserType + 5 };
	virtual int type() const { return Type; }

private:
	QPointF	m_to;
};
