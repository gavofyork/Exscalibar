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

#include "ProcessorsView.h"
#include "ProcessorItem.h"
#include "OutputItem.h"

OutputItem::OutputItem(int _i, ProcessorItem* _p, QSizeF const& _size):
	QGraphicsItem	(_p),
	m_size			(_size),
	m_index			(_i),
	m_hover			(false),
	m_inputItem		(0)
{
	setCursor(Qt::CrossCursor);
	setAcceptHoverEvents(true);
}

bool OutputItem::isConnected() const
{
	if (!scene())
		return false;
	foreach (ConnectionItem* i, filter<ConnectionItem>(scene()->items()))
		if (i->from() == this)
			return true;
	return false;
}

BaseItem* OutputItem::baseItem() const
{
	return dynamic_cast<BaseItem*>(parentItem());
}

QPolygonF OutputItem::polygon() const
{
	QPolygonF p;
	double psot = m_size.height() / 2;
	double cs = m_size.width();
	p.append(QPointF(0, -psot));
	p.append(QPointF(0, psot));
	p.append(QPointF(cs - psot, psot));
	p.append(QPointF(cs, 0));
	p.append(QPointF(cs - psot, -psot));
	return p;
}

QPointF OutputItem::tip() const
{
	return QPointF(m_size.width(), 0);
}

void OutputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	double psot = m_size.height() / 2;
	double cs = m_size.width();

	_p->translate(0, -pos().y());
	{
		QPolygonF p;
		p.append(QPointF(1, psot));
		p.append(QPointF(cs - psot, psot));
		p.append(QPointF(cs, 0));
		p.append(QPointF(cs - psot, -psot));
		p.append(QPointF(1, -psot));
		_p->setPen(baseItem()->outerPen());
		_p->setBrush(Qt::NoBrush);
		_p->drawPolyline(p.translated(0, pos().y()));
	}
	{
		QPolygonF p;
		p.append(QPointF(-.5f, psot - 1));
		p.append(QPointF(cs - psot - 1, psot - 1));
		p.append(QPointF(cs - 1, 0));
		p.append(QPointF(cs - psot - 1, 1-psot));
		p.append(QPointF(-.5f, 1-psot));
		_p->setPen(Qt::NoPen);
		_p->setBrush(baseItem()->fillBrush());
		_p->drawPolygon(p.translated(0, pos().y()));
	}
	{
		QPolygonF p;
		p.append(QPointF(0, psot - 1));
		p.append(QPointF(cs - psot - 1, psot - 1));
		p.append(QPointF(cs - 1, 0));
		p.append(QPointF(cs - psot - 1, 1-psot));
		p.append(QPointF(0, 1-psot));
		_p->setPen(baseItem()->innerPen());
		_p->setBrush(Qt::NoBrush);
		_p->drawPolyline(p.translated(0, pos().y()));
	}
}

void OutputItem::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	prepareGeometryChange();
	m_hover = true;
	update();
}

void OutputItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	prepareGeometryChange();
	m_hover = false;
	update();
}

void OutputItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
	qobject_cast<ProcessorsScene*>(scene())->beginConnect(this);
}

ProcessorItem* OutputItem::processorItem() const
{
	return dynamic_cast<ProcessorItem*>(parentItem());
}
