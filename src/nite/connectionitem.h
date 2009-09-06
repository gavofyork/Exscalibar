#pragma once

#include <QtGui>
#include <QtXml>

class ConnectionItem : public QGraphicsLineItem
{
public:
	ConnectionItem(Processor* _from, Processor* _to);

	enum { Type = UserType + 4 };
	virtual int type() const { return Type; }

private:
	Processor* m_from;
	Processor* m_to;
};
