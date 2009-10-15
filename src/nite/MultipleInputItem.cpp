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
#include "MultipleInputItem.h"

MultipleInputItem::MultipleInputItem(ProcessorItem* _p, QSizeF const& _size):
	QGraphicsItem	(_p),
	m_index			(UINT_MAX),
	m_size			(_size),
	m_baseSize		(_size)
{
}

MultipleInputItem::MultipleInputItem(int _i, MultiProcessorItem* _p, QSizeF const& _size):
	QGraphicsItem	(_p),
	m_index			(_i),
	m_size			(_size),
	m_baseSize		(_size)
{
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
	return QRectF(-m_size.width(), -m_size.height(), m_size.width(), m_size.height() * 3 / 2);
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
	_p->drawLine(0, -m_size.height() * 3 / 4, -m_size.width(), -m_size.height() * 3 / 4);
	_p->drawLine(0, -m_size.height(), -m_size.width(), -m_size.height());
	_p->drawLine(-m_size.width(), 0, m_size.height() / 2 - m_size.width(), -m_size.height() / 2);
	_p->drawLine(-m_size.width(), m_size.height() / 4, m_size.height() / 2 - m_size.width(), -m_size.height() / 4);

	QPolygonF p;
	p.append(QPointF(m_size.height() / 2 - m_size.width(), 0));
	p.append(QPointF(-m_size.width(), m_size.height() / 2));
	p.append(QPointF(0, m_size.height() / 2));
	p.append(QPointF(0, -m_size.height() / 2));
	p.append(QPointF(-m_size.width(), -m_size.height() / 2));
	_p->drawPolygon(p);
}
