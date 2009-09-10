#include "inputitem.h"
#include "outputitem.h"
#include "processoritem.h"
#include "connectionitem.h"

ConnectionItem::ConnectionItem(InputItem* _to, OutputItem* _from):
	QGraphicsPathItem	(_to),
	m_isValid			(true),
	m_from				(_from)
{
	setPen(QPen(m_isValid ? Qt::black : Qt::red, 2));
	rejigEndPoints();
}

QPointF ConnectionItem::wouldAdjust() const
{
	QPointF to = qgraphicsitem_cast<InputItem*>(parentItem())->tip();
	QPointF from = mapFromItem(m_from, m_from->tip());
	return QPointF(1e99, from.y() - to.y());
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
}

ProcessorItem* ConnectionItem::fromProcessor() const
{
	return qgraphicsitem_cast<ProcessorItem*>(m_from->parentItem());
}

ProcessorItem* ConnectionItem::toProcessor() const
{
	return qgraphicsitem_cast<ProcessorItem*>(parentItem()->parentItem());
}

void ConnectionItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	OutputItem* oi = 0;
	InputItem* ii = 0;
	foreach (ProcessorItem* pi, filter<ProcessorItem>(_scene->items()))
		if (pi->processor() && pi->processor()->name() == _element.attribute("from"))
			foreach (OutputItem* i, filter<OutputItem>(pi->childItems()))
				if (i->index() == (uint)_element.attribute("fromindex").toInt())
					oi = i;
				else {}
		else if (pi->processor() && pi->processor()->name() == _element.attribute("to"))
			foreach (InputItem* i, filter<InputItem>(pi->childItems()))
				if (i->index() == (uint)_element.attribute("toindex").toInt())
					ii = i;
	if (!ii || !oi)
		return;
	new ConnectionItem(ii, oi);
}

void ConnectionItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement proc = _doc.createElement("connection");

	proc.setAttribute("from", fromProcessor()->processor()->name());
	proc.setAttribute("fromindex", m_from->index());
	proc.setAttribute("to", toProcessor()->processor()->name());
	proc.setAttribute("toindex", qgraphicsitem_cast<InputItem*>(parentItem())->index());

	_root.appendChild(proc);
}
