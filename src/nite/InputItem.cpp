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
#include "InputItem.h"

InputItem::InputItem(int _i, BaseItem* _p, QSizeF const& _size): QGraphicsItem(_p), m_index(_i), m_baseSize(_size)
{
	m_size = m_baseSize;
}

ProcessorItem* InputItem::processorItem() const
{
	return dynamic_cast<ProcessorItem*>(parentItem());
}

BaseItem* InputItem::baseItem() const
{
	return dynamic_cast<BaseItem*>(parentItem());
}

bool InputItem::isConnected() const
{
	return filter<ConnectionItem>(childItems()).size();
}

QRectF InputItem::boundingRect() const
{
	return QRectF(-m_size.width(), -m_size.height() / 2, m_size.width(), m_size.height());
}

QPointF InputItem::tip() const
{
	return QPointF(m_size.height() / 2 - m_size.width(), 0);
}

void InputItem::typesConfirmed()
{
	prepareGeometryChange();
	m_size = m_baseSize;
	m_typeInfo = "<div><b>Single Connection</b></div>" + processorItem()->m_processor->input(m_index).type().info();
	update();
	foreach (QGraphicsItem* i, childItems())
		if (ConnectionItem* ci = dynamic_cast<ConnectionItem*>(i))
			ci->rejigEndPoints();
}

void InputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->save();
	_p->setPen(baseItem()->outerPen());
	_p->translate(0, -pos().y());
	{
		QPolygonF p;
		p.append(QPointF(-1, -m_size.height() / 2));
		p.append(QPointF(-m_size.width(), -m_size.height() / 2));
		p.append(QPointF(m_size.height() / 2 - m_size.width(), 0));
		p.append(QPointF(-m_size.width(), m_size.height() / 2));
		p.append(QPointF(-1, m_size.height() / 2));
		_p->drawPolyline(p.translated(0, pos().y()));
	}
	{
		QPolygonF p;
		p.append(QPointF(0.5, -m_size.height() / 2 + 1));
		p.append(QPointF(-m_size.width() + 2, -m_size.height() / 2 + 1));
		p.append(QPointF(m_size.height() / 2 - m_size.width() + 2, 0));
		p.append(QPointF(-m_size.width() + 2, m_size.height() / 2 - 1));
		p.append(QPointF(0.5, m_size.height() / 2 - 1));
		_p->setPen(Qt::NoPen);
		_p->setBrush(baseItem()->fillBrush());
		_p->drawPolygon(p.translated(0, pos().y()));
	}
	{
		QPolygonF p;
		p.append(QPointF(0, -m_size.height() / 2 + 1));
		p.append(QPointF(-m_size.width() + 2, -m_size.height() / 2 + 1));
		p.append(QPointF(m_size.height() / 2 - m_size.width() + 2, 0));
		p.append(QPointF(-m_size.width() + 2, m_size.height() / 2 - 1));
		p.append(QPointF(0, m_size.height() / 2 - 1));
		_p->setPen(baseItem()->innerPen());
		_p->setBrush(Qt::NoBrush);
		_p->drawPolyline(p.translated(0, pos().y()));
	}
	_p->restore();
}
