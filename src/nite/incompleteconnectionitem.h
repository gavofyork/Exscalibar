#pragma once

#include <QtGui>
#include <QtXml>

class OutputItem;
class ProcessorItem;

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

class IncompleteMultipleConnectionItem: public QGraphicsPathItem
{
public:
	IncompleteMultipleConnectionItem(ProcessorItem* _from);

	void setTo(QPointF _to);
	ProcessorItem* from() const;

	enum { Type = UserType + 10 };
	virtual int type() const { return Type; }

private:
	QPointF	m_to;
};
