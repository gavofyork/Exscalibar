#include "inputitem.h"
#include "outputitem.h"
#include "processoritem.h"
#include "connectionitem.h"

ConnectionItem::ConnectionItem(InputItem* _to, OutputItem* _from):
	QGraphicsPathItem	(_to),
	m_isValid			(true),
	m_from				(_from)
{
	setPen(QPen(m_isValid ? Qt::black : Qt::red, 8));
	rejigEndPoints();
	setFlags(ItemIsFocusable | ItemIsSelectable);
	setCursor(Qt::ArrowCursor);
	setZValue(-1);
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

void ConnectionItem::focusInEvent(QFocusEvent* _e)
{
	foreach (QGraphicsItem* i, scene()->selectedItems())
		if (i != this)
			i->setSelected(false);
	setSelected(true);
	assert(isSelected());
	update();
	QGraphicsItem::focusInEvent(_e);
}

void ConnectionItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (isSelected())
	{
		_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), 8));
		_p->drawPath(path());
		_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), 5));
		_p->drawPath(path());
	}
	_p->setPen(QPen(Qt::black, 2));
	_p->drawPath(path());
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

MultipleConnectionItem::MultipleConnectionItem(ProcessorItem* _to, ProcessorItem* _from):
	QGraphicsPathItem	(_to),
	m_isValid			(true),
	m_from				(_from)
{
	setPen(QPen(m_isValid ? Qt::black : Qt::red, 8));
	rejigEndPoints();
	setFlags(ItemIsFocusable | ItemIsSelectable);
	setCursor(Qt::ArrowCursor);
	setZValue(-1);
}

void MultipleConnectionItem::rejigEndPoints()
{
	QPainterPath p;
	QPointF to = QPointF(0, 0);
	QPointF from = mapFromItem(m_from, QPointF(0, 0)/*m_from->pos()*/);
	p.moveTo(from);
	QPointF c1((to.x() * 3 + from.x()) / 4.0, from.y());
	QPointF c2((to.x() + from.x() * 3) / 4.0, to.y());
	p.cubicTo(c1, c2, to);
	setPath(p);
}

void MultipleConnectionItem::focusInEvent(QFocusEvent* _e)
{
	foreach (QGraphicsItem* i, scene()->selectedItems())
		if (i != this)
			i->setSelected(false);
	setSelected(true);
	assert(isSelected());
	update();
	QGraphicsItem::focusInEvent(_e);
}

void MultipleConnectionItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (isSelected())
	{
		_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), 8));
		_p->drawPath(path());
		_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), 5));
		_p->drawPath(path());
	}
	_p->setPen(QPen(Qt::black, 2));
	_p->drawPath(path());
}


ProcessorItem* MultipleConnectionItem::fromProcessor() const
{
	return qgraphicsitem_cast<ProcessorItem*>(m_from);
}

ProcessorItem* MultipleConnectionItem::toProcessor() const
{
	return qgraphicsitem_cast<ProcessorItem*>(parentItem());
}

void MultipleConnectionItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	ProcessorItem* op = 0;
	ProcessorItem* ip = 0;
	foreach (ProcessorItem* pi, filter<ProcessorItem>(_scene->items()))
		if (pi->processor() && pi->processor()->name() == _element.attribute("from"))
			op = pi;
		else if (pi->processor() && pi->processor()->name() == _element.attribute("to"))
			ip = pi;
	if (!ip || !op)
		return;
	new MultipleConnectionItem(ip, op);
}

void MultipleConnectionItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement proc = _doc.createElement("connection");
	proc.setAttribute("from", fromProcessor()->processor()->name());
	proc.setAttribute("to", toProcessor()->processor()->name());
	_root.appendChild(proc);
}
