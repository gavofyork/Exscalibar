/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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
#include "ConnectionItem.h"

ConnectionItem::ConnectionItem(InputItem* _to, OutputItem* _from):
	QGraphicsPathItem	(0),
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
}

QList<QPointF> ConnectionItem::magnetism(BaseItem const* _b, bool _moving) const
{
	QList<QPointF> ret;
	if ((toProcessor() == dynamic_cast<ProcessorItem const*>(_b) || fromProcessor() == dynamic_cast<ProcessorItem const*>(_b)) && _moving)
		ret << (toProcessor() == _b ? 1 : -1) * QPointF(1e99, mapFromItem(m_from, m_from->tip()).y() - mapFromItem(m_to, m_to->tip()).y());
	return ret;
}

void ConnectionItem::rejigEndPoints()
{
	QPainterPath p;
	QPointF to = mapFromItem(m_to, m_to->tip());
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
	return dynamic_cast<ProcessorItem*>(m_from->parentItem());
}

ProcessorItem* ConnectionItem::toProcessor() const
{
	return dynamic_cast<ProcessorItem*>(m_to->parentItem());
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

	proc.setAttribute("from", fromProcessor()->processor()->name());
	proc.setAttribute("fromindex", m_from->index());
	proc.setAttribute("to", toProcessor()->processor()->name());
	proc.setAttribute("toindex", m_to->index());

	_root.appendChild(proc);
}
