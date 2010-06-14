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
#include "ProcessorBasedItem.h"
#include "OutputItem.h"

OutputItem::OutputItem(int _i, BaseItem* _p, QSizeF const& _size):
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

QRectF OutputItem::boundingRect() const
{
	return QRectF(0, -m_size.height() / 2, m_size.width(), m_size.height()).adjusted(-2, -2, 2, 2);
}

QPointF OutputItem::tip() const
{
	return QPointF(m_size.width(), 0);
}

void OutputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem* _o, QWidget* _w)
{
	double psot = m_size.height() / 2;
	double cs = m_size.width();
	float de = .42f;

	QPolygonF po;
	po.append(QPointF(1, psot));
	po.append(QPointF(cs - psot, psot));
	po.append(QPointF(cs, 0));
	po.append(QPointF(cs - psot, -psot));
	po.append(QPointF(1, -psot));
	QPolygonF pf;
	pf.append(QPointF(-.5f, psot - 1));
	pf.append(QPointF(cs - psot - de, psot - 1));
	pf.append(QPointF(cs - 1 - de, 0));
	pf.append(QPointF(cs - psot - de, 1-psot));
	pf.append(QPointF(-.5f, 1-psot));
	QPolygonF pi;
	pi.append(QPointF(0, psot - 1));
	pi.append(QPointF(cs - psot - de, psot - 1));
	pi.append(QPointF(cs - 1 - de, 0));
	pi.append(QPointF(cs - psot - de, 1-psot));
	pi.append(QPointF(0, 1-psot));

	_p->save();
	_p->translate(0, -pos().y());
	_p->setPen(baseItem()->outerPen());
	_p->setBrush(Qt::NoBrush);
	_p->drawPolyline(po.translated(0, pos().y()));
	_p->setPen(Qt::NoPen);
	_p->setBrush(baseItem()->fillBrush());
	_p->drawPolygon(pf.translated(0, pos().y()));
	_p->setPen(baseItem()->innerPen());
	_p->setBrush(Qt::NoBrush);
	_p->drawPolyline(pi.translated(0, pos().y()));
	_p->restore();

	interPaint(_p, _o, _w);

	if (parentItem()->isSelected())
	{
		_p->setBrush(Qt::NoBrush);
		for (int i = 1; i < 4; i++)
		{
			_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), i));
			_p->drawPolyline(po);
		}
	}
}

void OutputItem::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	m_hover = true;
	update();
}

void OutputItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	m_hover = false;
	update();
}

void OutputItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
	qobject_cast<ProcessorsScene*>(scene())->beginConnect(this);
}

ProcessorBasedItem* OutputItem::processorItem() const
{
	return dynamic_cast<ProcessorBasedItem*>(parentItem());
}
