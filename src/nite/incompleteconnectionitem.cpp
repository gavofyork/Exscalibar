#include "outputitem.h"
#include "connectionitem.h"

IncompleteConnectionItem::IncompleteConnectionItem(OutputItem* _from): QGraphicsPathItem(_from)
{
	setPen(QPen(QColor(0, 0, 0, 128), 2));
}

OutputItem* IncompleteConnectionItem::from() const
{
	return qgraphicsitem_cast<OutputItem*>(parentItem());
}

void IncompleteConnectionItem::setTo(QPointF _to)
{
	m_to = mapFromScene(_to);
	QPainterPath p;
	p.moveTo(from()->tip());
	QPointF c1((m_to.x() * 3 + from()->tip().x()) / 4.0, from()->tip().y());
	QPointF c2((m_to.x() + from()->tip().x() * 3) / 4.0, m_to.y());
	p.cubicTo(c1, c2, m_to);
	setPath(p);
}
