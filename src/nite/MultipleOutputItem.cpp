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

#include "ProcessorsScene.h"
#include "ProcessorItem.h"
#include "MultipleOutputItem.h"

MultipleOutputItem::MultipleOutputItem(ProcessorItem* _p, QSizeF const& _size):
	QGraphicsItem	(_p),
	m_multiplicity	(UINT_MAX),
	m_size			(_size),
	m_index			(0),
	m_hover			(false)
{
	setCursor(Qt::CrossCursor);
	setAcceptHoverEvents(true);
}

MultipleOutputItem::MultipleOutputItem(int _i, MultiProcessorItem* _p, QSizeF const& _size):
	QGraphicsItem	(_p),
	m_multiplicity	(UINT_MAX),
	m_size			(_size),
	m_index			(_i),
	m_hover			(false)
{
	setCursor(Qt::CrossCursor);
	setAcceptHoverEvents(true);
}

void MultipleOutputItem::setMultiplicity(uint _m)
{
	if (m_multiplicity != _m)
	{
		prepareGeometryChange();
		m_multiplicity = _m;
		update();
	}
}

bool MultipleOutputItem::isConnected() const
{
	if (!scene())
		return false;
	foreach (MultipleConnectionItem* i, filter<MultipleConnectionItem>(scene()->items()))
		if (i->from() == this)
			return true;
	return false;
}

MultiSource* MultipleOutputItem::source() const
{
	return processorItem() ? static_cast<MultiSource*>(processorItem()->processor()) : static_cast<MultiSource*>(multiProcessorItem()->multiProcessor());
}

BaseItem* MultipleOutputItem::baseItem() const
{
	return dynamic_cast<BaseItem*>(parentItem());
}

ProcessorItem* MultipleOutputItem::processorItem() const
{
	return dynamic_cast<ProcessorItem*>(parentItem());
}

MultiProcessorItem* MultipleOutputItem::multiProcessorItem() const
{
	return dynamic_cast<MultiProcessorItem*>(parentItem());
}

QRectF MultipleOutputItem::boundingRect() const
{
	return QRectF(0, -m_size.height() / 2, m_size.width(), m_size.height());
}

QPointF MultipleOutputItem::tip() const
{
	return QPointF(m_size.width(), 0);
}

void MultipleOutputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	double psot = m_size.height() / 2;
	double cs = m_size.width();

	_p->save();
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
	_p->restore();
	QString s = m_multiplicity != Undefined ? QString::number(m_multiplicity) : "?";
	QFont f;
	f.setBold(true);
	QRectF br = boundingRect().adjusted(2, 2, -2 - m_size.height() / 2, -2);
	f.setPixelSize(br.height());
	_p->setFont(f);
	_p->setPen(QColor(0, 0, 0, 96));
	_p->drawText(br.translated(0, 1), Qt::AlignCenter, s);
	_p->setPen(Qt::white);
	_p->drawText(br, Qt::AlignCenter, s);
}

void MultipleOutputItem::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	m_hover = true;
	update();
}

void MultipleOutputItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	m_hover = false;
	update();
}

void MultipleOutputItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
	qobject_cast<ProcessorsScene*>(scene())->beginMultipleConnect(this);
}
