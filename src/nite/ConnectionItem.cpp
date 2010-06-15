/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
 *
 * This file is part of Exscalibar.
 *
 * Exscalibar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Exscalibar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Exscalibar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "InputItem.h"
#include "OutputItem.h"
#include "ProcessorItem.h"
#include "DomProcessorItem.h"
#include "ConnectionItem.h"

ConnectionItem::ConnectionItem(InputItem* _to, OutputItem* _from):
	QGraphicsPathItem	(0),
	m_rejigging			(false),
	m_isValid			(true),
	m_from				(_from),
	m_to				(_to)
{
	_to->scene()->addItem(this);
	setPen(QPen(m_isValid ? Qt::black : Qt::red, 8));
	rejigEndPoints();
	setFlags(ItemIsFocusable | ItemIsSelectable);
	setCursor(Qt::ArrowCursor);
	setZValue(-1);
	foreach (ConnectionItem* ci, filterRelaxed<ConnectionItem>(scene()->items()))
		if (ci->from() == from() || ci->to() == to())
			ci->refreshNature();
}

ConnectionItem::~ConnectionItem()
{
	InputItem* t = m_to;
	OutputItem* f = m_from;
	m_from = 0;
	m_to = 0;
	foreach (ConnectionItem* ci, filterRelaxed<ConnectionItem>(scene()->items()))
		if (ci->from() == f || ci->to() == t)
			ci->refreshNature();
}

QList<QPointF> ConnectionItem::magnetism(BaseItem const* _b, bool _moving) const
{
	QList<QPointF> ret;
	if ((toProcessor() == dynamic_cast<ProcessorItem const*>(_b) || fromProcessor() == dynamic_cast<ProcessorItem const*>(_b)) && _moving)
		ret << ((toProcessor() == _b) ? 1 : -1) * QPointF(1e99, mapFromItem(m_from, m_from->tip()).y() - mapFromItem(m_to, m_to->tip()).y())
			<< ((toProcessor() == _b) ? 1 : -1) * QPointF(mapFromItem(m_from, m_from->tip()).x() - mapFromItem(m_to, m_to->tip()).x(), 1e99);
	return ret;
}

void ConnectionItem::rejigEndPoints()
{
	if (m_rejigging)
		return;
	m_rejigging = true;
	QPainterPath p;
	QPointF from = mapFromItem(m_from, m_from->tip());
	if (path().elementCount() == 1 && from != path().elementAt(0))
	{
		m_to->baseItem()->setPos(m_to->baseItem()->pos() + from - path().elementAt(0));
		m_to->baseItem()->notePositionChange();
	}
	QPointF to = mapFromItem(m_to, m_to->tip());
	p.moveTo(from);
	QPointF c1((to.x() * 3 + from.x()) / 4.0, from.y());
	QPointF c2((to.x() + from.x() * 3) / 4.0, to.y());
	p.cubicTo(c1, c2, to);
	setPath(p);
	m_rejigging = false;
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
	_p->setPen(QPen((m_nature == Connection) ? Qt::black : Qt::blue, 2));
	_p->drawPath(path());
}

void ConnectionItem::refreshNature()
{
	update();
	m_nature = Connection;

	DomProcessorItem* fp = dynamic_cast<DomProcessorItem*>(fromProcessor());
	DomProcessorItem* tp = dynamic_cast<DomProcessorItem*>(toProcessor());

	if (!fp || !tp ||
		filter<OutputItem>(fp->childItems()).count() != 1 ||
		filter<InputItem>(tp->childItems()).count() != 1)
		return;

	foreach (ConnectionItem* i, filter<ConnectionItem>(scene()->items()))
		if ((i->from() == from() && i != this) || (i->to() == to() && i != this))
			return;

	// Single connection between two DomProcessors each with one input/output.
	m_nature = Coupling;
}

ProcessorBasedItem* ConnectionItem::fromProcessor() const
{
	return dynamic_cast<ProcessorBasedItem*>(m_from->parentItem());
}

ProcessorBasedItem* ConnectionItem::toProcessor() const
{
	return dynamic_cast<ProcessorBasedItem*>(m_to->parentItem());
}

BaseItem* ConnectionItem::fromBase() const
{
	return dynamic_cast<BaseItem*>(m_from->parentItem());
}

BaseItem* ConnectionItem::toBase() const
{
	return dynamic_cast<BaseItem*>(m_to->parentItem());
}

void ConnectionItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	OutputItem* oi = 0;
	InputItem* ii = 0;
	foreach (ProcessorBasedItem* pi, filterRelaxed<ProcessorBasedItem>(_scene->items()))
		if (pi->name() == _element.attribute("from"))
			foreach (OutputItem* i, filter<OutputItem>(pi->childItems()))
				if (i->index() == (uint)_element.attribute("fromindex").toInt())
					oi = i;
				else {}
		else if (pi->name() == _element.attribute("to"))
			foreach (InputItem* i, filter<InputItem>(pi->childItems()))
				if (i->index() == (uint)_element.attribute("toindex").toInt())
					ii = i;
	if (!oi)
	{
		qDebug() << "Couldn't find processor" << _element.attribute("from");
		return;
	}
	if (!ii)
	{
		qDebug() << "Couldn't find processor" << _element.attribute("to");
		return;
	}
	new ConnectionItem(ii, oi);
}

void ConnectionItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement proc = _doc.createElement("connection");

	proc.setAttribute("from", fromProcessor()->name());
	proc.setAttribute("fromindex", m_from->index());
	proc.setAttribute("to", toProcessor()->name());
	proc.setAttribute("toindex", m_to->index());

	_root.appendChild(proc);
}
