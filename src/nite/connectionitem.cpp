#include "inputitem.h"
#include "outputitem.h"
#include "connectionitem.h"

ConnectionItem::ConnectionItem(InputItem* _to, OutputItem* _from):
	QGraphicsLineItem	(_to),
	m_from				(_from)
{
	setLine(QLineF(mapFromItem(_from, _from->tip()), _to->tip()));
}

void ConnectionItem::rejigEndPoints()
{
	setLine(QLineF(mapFromItem(m_from, m_from->tip()), line().p2()));
}

ProcessorItem* ConnectionItem::fromProcessor() const
{
	return qgraphicsitem_cast<ProcessorItem*>(m_from->parentItem());
}

ProcessorItem* ConnectionItem::toProcessor() const
{
	return qgraphicsitem_cast<ProcessorItem*>(parentItem()->parentItem());
}

