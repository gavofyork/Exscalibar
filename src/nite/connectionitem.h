#pragma once

#include <QtGui>
#include <QtXml>

class InputItem;
class OutputItem;

class ConnectionItem : public QGraphicsLineItem
{
public:
	ConnectionItem(InputItem* _to, OutputItem* _from);

	enum { Type = UserType + 4 };
	virtual int type() const { return Type; }

private:
	OutputItem* m_from;
};
