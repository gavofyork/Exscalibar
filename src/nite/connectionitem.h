#pragma once

#include <QtGui>
#include <QtXml>

class InputItem;
class OutputItem;
class ProcessorItem;

class ConnectionItem: public QGraphicsPathItem
{
public:
	ConnectionItem(InputItem* _to, OutputItem* _from);

	enum { Type = UserType + 4 };
	virtual int type() const { return Type; }

	OutputItem* from() const { return m_from; }

	ProcessorItem* toProcessor() const;
	ProcessorItem* fromProcessor() const;

	void rejigEndPoints();

private:
	OutputItem* m_from;
};
