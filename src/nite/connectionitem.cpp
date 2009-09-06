#include "inputitem.h"
#include "outputitem.h"
#include "connectionitem.h"

ConnectionItem::ConnectionItem(InputItem* _to, OutputItem* _from):
	QGraphicsLineItem	(_to),
	m_from				(_from)
{
	setLine(QLineF(mapFromItem(_from, _from->tip()), _to->tip()));
}
