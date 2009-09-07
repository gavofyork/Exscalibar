#include "inputitem.h"
#include "outputitem.h"
#include "connectionitem.h"

ConnectionItem::ConnectionItem(InputItem* _to, OutputItem* _from):
	QGraphicsPathItem	(_to),
	m_from				(_from)
{
	setPen(QPen(Qt::black, 2));
	rejigEndPoints();
}

void ConnectionItem::rejigEndPoints()
{
	QPainterPath p;
	QPointF to = qgraphicsitem_cast<InputItem*>(parentItem())->tip();
	QPointF from = mapFromItem(m_from, m_from->tip());
	p.moveTo(from);
	QPointF c1((to.x() * 3 + from.x()) / 4.0, from.y());
	QPointF c2((to.x() + from.x() * 3) / 4.0, to.y());
	p.cubicTo(c1, c2, to);
	setPath(p);
//	setLine(QLineF(mapFromItem(m_from, m_from->tip()), qgraphicsitem_cast<InputItem*>(parentItem())->tip()));
}

ProcessorItem* ConnectionItem::fromProcessor() const
{
	return qgraphicsitem_cast<ProcessorItem*>(m_from->parentItem());
}

ProcessorItem* ConnectionItem::toProcessor() const
{
	return qgraphicsitem_cast<ProcessorItem*>(parentItem()->parentItem());
}

