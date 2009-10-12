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

OutputItem::OutputItem(int _i, ProcessorItem* _p): QGraphicsItem(_p), m_index(_i), m_hover(false), m_inputItem(0)
{
	setCursor(Qt::CrossCursor);
	setAcceptHoverEvents(true);
}

QPolygonF OutputItem::polygon() const
{
	QPolygonF p;
	double psot = portSize / 2 * (m_hover ? 2 : 1);
	double cs = cornerSize * (m_hover ? 2 : 1);
	p.append(QPointF(0, -psot));
	p.append(QPointF(0, psot));
	p.append(QPointF(cs, psot));
	p.append(QPointF(cs + psot, 0));
	p.append(QPointF(cs, -psot));
	return p;
}

QPointF OutputItem::tip() const
{
	double psot = portSize / 2;
	double cs = cornerSize;
	return QPointF(cs + psot, 0);
}

void OutputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->setPen(QPen(Qt::black, 0));
	_p->setBrush(QColor::fromHsv(40, 128, 220, 255));
	_p->drawPolygon(polygon());
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
	return qgraphicsitem_cast<ProcessorItem*>(parentItem());
}
