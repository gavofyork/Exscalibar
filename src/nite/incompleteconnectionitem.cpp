#include "outputitem.h"
#include "connectionitem.h"

IncompleteConnectionItem::IncompleteConnectionItem(OutputItem* _from): QGraphicsLineItem(_from)
{
}

OutputItem* IncompleteConnectionItem::from() const
{
	return qgraphicsitem_cast<OutputItem*>(parentItem());
}

void IncompleteConnectionItem::setTo(QPointF _to)
{
	m_to = mapFromScene(_to);
	setLine(QLineF(from()->tip(), m_to));
}
