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

#include <Geddei>

#include "ConnectionItem.h"
#include "ProcessorItem.h"
#include "MultiProcessorItem.h"
#include "MultipleInputItem.h"

MultipleInputItem::MultipleInputItem(ProcessorItem* _p, QSizeF const& _size):
	QGraphicsItem	(_p),
	m_multiplicity	(UINT_MAX),
	m_index			(0),
	m_size			(_size),
	m_baseSize		(_size)
{
}

MultipleInputItem::MultipleInputItem(int _i, MultiProcessorItem* _p, QSizeF const& _size):
	QGraphicsItem	(_p),
	m_multiplicity	(UINT_MAX),
	m_index			(_i),
	m_size			(_size),
	m_baseSize		(_size)
{
}

MultiSink* MultipleInputItem::sink() const
{
	return processorItem() ? static_cast<MultiSink*>(processorItem()->processor()) : static_cast<MultiSink*>(multiProcessorItem()->multiProcessor());
}

void MultipleInputItem::setMultiplicity(uint _m)
{
	if (m_multiplicity != _m)
	{
		prepareGeometryChange();
		m_multiplicity = _m;
		update();
	}
}

bool MultipleInputItem::isConnected() const
{
	return filter<MultipleConnectionItem>(childItems()).size();
}

BaseItem* MultipleInputItem::baseItem() const
{
	return dynamic_cast<BaseItem*>(parentItem());
}

ProcessorItem* MultipleInputItem::processorItem() const
{
	return dynamic_cast<ProcessorItem*>(parentItem());
}

MultiProcessorItem* MultipleInputItem::multiProcessorItem() const
{
	return dynamic_cast<MultiProcessorItem*>(parentItem());
}

QRectF MultipleInputItem::boundingRect() const
{
	return QRectF(-m_size.width(), -m_size.height() * (0.5 + 0.25 * ((m_multiplicity == Undefined) ? 6 : m_multiplicity)), m_size.width(), m_size.height() * (1 + 0.25 * ((m_multiplicity == Undefined) ? 6 : m_multiplicity)));
}

QPointF MultipleInputItem::tip() const
{
	return QPointF(m_size.height() / 2 - m_size.width(), 0);
}

void MultipleInputItem::typesConfirmed()
{
}

void MultipleInputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->setPen(QPen(Qt::black, 0));
	_p->setBrush(QColor::fromHsv(40, 128, 220));
	float w = m_size.width();
	float h = m_size.height();
	for (int i = 0; (uint)i < ((m_multiplicity == Undefined) ? 4 : (m_multiplicity - 1)); i++)
	{
		if (m_multiplicity == Undefined)
			_p->setPen(QPen(QColor::fromHsv(0, 0, 0, (4 - i) * 255 / 4), 0));
		_p->drawLine(0, -h * (i + 3) / 4, -w, -h * (i + 3) / 4);
		if (i < 2)
			_p->drawLine(-w, -h * (i - 1) / 4, h / 2 - w, -h * (i + 1) / 4);
	}
	_p->setPen(QPen(Qt::black, 0));
	QPolygonF p;
	p.append(QPointF(m_size.height() / 2 - m_size.width(), 0));
	p.append(QPointF(-m_size.width(), m_size.height() / 2));
	p.append(QPointF(0, m_size.height() / 2));
	p.append(QPointF(0, -m_size.height() / 2));
	p.append(QPointF(-m_size.width(), -m_size.height() / 2));
	_p->drawPolygon(p);
}
