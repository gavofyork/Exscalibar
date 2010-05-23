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

#include "MultipleInputItem.h"
#include "MultipleOutputItem.h"
#include "MultipleConnectionItem.h"

MultipleConnectionItem::MultipleConnectionItem(MultipleInputItem* _to, MultipleOutputItem* _from):
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

void MultipleConnectionItem::rejigEndPoints()
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

QList<QPointF> MultipleConnectionItem::magnetism(BaseItem const* _b, bool _moving) const
{
	QList<QPointF> ret;
	if ((to()->multiProcessorItem() == dynamic_cast<MultiProcessorItem const*>(_b) || from()->multiProcessorItem() == dynamic_cast<MultiProcessorItem const*>(_b)) && _moving)
		ret << (to()->multiProcessorItem() == _b ? 1 : -1) * QPointF(1e99, mapFromItem(m_from, m_from->tip()).y() - mapFromItem(m_to, m_to->tip()).y());
	return ret;
}

void MultipleConnectionItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	MultipleOutputItem* moi = 0;
	MultipleInputItem* mii = 0;
	foreach (BaseItem* bi, filterRelaxed<BaseItem>(_scene->items()))
		if (bi->name() == _element.attribute("from"))
			foreach (MultipleOutputItem* i, filter<MultipleOutputItem>(bi->childItems()))
				if (i->index() == _element.attribute("fromindex").toUInt())
					moi = i; else {}
		else if (bi->name() == _element.attribute("to"))
			foreach (MultipleInputItem* i, filter<MultipleInputItem>(bi->childItems()))
				if (i->index() == _element.attribute("toindex").toUInt())
					mii = i; else {}
	if (!moi || !mii)
		return;
	if (mii->isConnected())
		return;
	new MultipleConnectionItem(mii, moi);
}

void MultipleConnectionItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement proc = _doc.createElement("multipleconnection");
	proc.setAttribute("from", from()->baseItem()->name());
	proc.setAttribute("fromindex", from()->index());
	proc.setAttribute("to", to()->baseItem()->name());
	proc.setAttribute("toindex", to()->index());
	_root.appendChild(proc);
}
