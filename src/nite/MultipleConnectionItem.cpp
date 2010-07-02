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

#include "MultiDomProcessorItem.h"
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

MultipleConnectionItem::~MultipleConnectionItem()
{
//	MultipleInputItem* t = to();
//	MultipleOutputItem* f = from();
	m_from = 0;
	m_to = 0;
}

void MultipleConnectionItem::refreshNature(BaseItem*)
{
	/*if (_b->scene())
		foreach (MultipleConnectionItem* ci, filterRelaxed<MultipleConnectionItem>(_b->scene()->items()))
			if (ci->from()->baseItem() == _b || ci->to()->baseItem() == _b)
				ci->refreshNature();*/
}

void MultipleConnectionItem::refreshNature(MultipleInputItem*, MultipleOutputItem*, QGraphicsScene*)
{
/*	if (_s)
		foreach (MultipleConnectionItem* ci, filterRelaxed<MultipleConnectionItem>(_s->items()))
			if (ci->from() == _o || ci->to() == _i)
				ci->refreshNature();*/
}

void MultipleConnectionItem::refreshNature()
{
	update();
	m_nature = ConnectionItem::Connection;

	MultiDomProcessorItem* fp = dynamic_cast<MultiDomProcessorItem*>(from()->baseItem());
	MultiDomProcessorItem* tp = dynamic_cast<MultiDomProcessorItem*>(to()->baseItem());

	if (!fp || !tp ||
		filter<MultipleOutputItem>(fp->childItems()).count() != 1 ||
		filter<MultipleInputItem>(tp->childItems()).count() != 1)
		return;

	foreach (MultipleConnectionItem* i, filter<MultipleConnectionItem>(scene()->items()))
		if ((i->from() == from() && i != this) || (i->to() == to() && i != this))
			return;

	// Single connection between two DomProcessors each with one input/output.
	m_nature = ConnectionItem::Coupling;
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
	m_centre = QPointF((from.x() + to.x()) / 2, (from.y() + to.y()) / 2);
	p.addEllipse(m_centre, 5, 5);
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
	_p->setPen(QPen((m_nature == ConnectionItem::Coupling) ? Qt::blue : Qt::black, 2));
	_p->drawPath(path());
	_p->setPen(Qt::NoPen);
	_p->setBrush(Qt::white);
	_p->drawEllipse(m_centre, 5, 5);
	_p->setPen(Qt::black);
	static QFont f("Helvetica", 7, QFont::Bold);
	_p->setFont(f);
	_p->drawText(QRectF(m_centre, QSizeF(0, 0)).adjusted(-5, -5, 5, 5), Qt::AlignCenter, "?");
}

QList<QPointF> MultipleConnectionItem::magnetism(BaseItem const* _b, bool _moving) const
{
	QList<QPointF> ret;
	if ((to()->baseItem() == _b || from()->baseItem() == _b) && _moving)
		ret << ((to()->baseItem() == _b) ? 1 : -1) * QPointF(1e99, mapFromItem(m_from, m_from->tip()).y() - mapFromItem(m_to, m_to->tip()).y())
			<< ((to()->baseItem() == _b) ? 1 : -1) * QPointF(mapFromItem(m_from, m_from->tip()).x() - mapFromItem(m_to, m_to->tip()).x(), 1e99);
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
