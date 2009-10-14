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

#include "MultipleConnectionItem.h"

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
	return dynamic_cast<ProcessorItem*>(m_from);
}

ProcessorItem* MultipleConnectionItem::toProcessor() const
{
	return dynamic_cast<ProcessorItem*>(parentItem());
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
	QDomElement proc = _doc.createElement("multipleconnection");
	proc.setAttribute("from", fromProcessor()->processor()->name());
	proc.setAttribute("to", toProcessor()->processor()->name());
	_root.appendChild(proc);
}
