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
	m_size			(_size),
	m_index			(UINT_MAX),
	m_hover			(false)
{
	setCursor(Qt::CrossCursor);
	setAcceptHoverEvents(true);
}

MultipleOutputItem::MultipleOutputItem(int _i, MultiProcessorItem* _p, QSizeF const& _size):
	QGraphicsItem	(_p),
	m_size			(_size),
	m_index			(_i),
	m_hover			(false)
{
	setCursor(Qt::CrossCursor);
	setAcceptHoverEvents(true);
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
	return QRectF(0, -m_size.height(), m_size.width(), m_size.height() * 3 / 2);
}

QPointF MultipleOutputItem::tip() const
{
	return QPointF(m_size.width(), 0);
}

void MultipleOutputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->setPen(QPen(Qt::black, 0));
	_p->setBrush(QColor::fromHsv(40, 128, m_hover ? 255 : 200));
	double w = m_size.width();
	double h = m_size.height();
	_p->drawLine(0, -h * 3 / 4, w - h / 2, -h * 3 / 4);
	_p->drawLine(0, -h, w - h / 2, -h);
	_p->drawLine(w - h / 2, -h * 3 / 4, w, -h / 4);
	_p->drawLine(w - h / 2, -h, w, -h / 2);
	QPolygonF p;
	p.append(QPointF(0, -h / 2));
	p.append(QPointF(0, h / 2));
	p.append(QPointF(w - h / 2, h / 2));
	p.append(QPointF(w, 0));
	p.append(QPointF(w - h / 2, -h / 2));
	_p->drawPolygon(p);
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
