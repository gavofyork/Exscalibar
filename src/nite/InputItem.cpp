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
#include "ProcessorBasedItem.h"
#include "InputItem.h"

InputItem::InputItem(int _i, BaseItem* _p, QSizeF const& _size): QGraphicsItem(_p), m_index(_i), m_baseSize(_size)
{
	m_size = m_baseSize;
}

ProcessorBasedItem* InputItem::processorItem() const
{
	return dynamic_cast<ProcessorBasedItem*>(parentItem());
}

BaseItem* InputItem::baseItem() const
{
	return dynamic_cast<BaseItem*>(parentItem());
}

bool InputItem::isConnected() const
{
	if (!scene())
		return false;
	foreach (ConnectionItem* ci, filter<ConnectionItem>(scene()->items()))
		if (ci->to() == this)
			return true;
	return false;
}

QRectF InputItem::boundingRect() const
{
	return QRectF(-m_size.width(), -m_size.height() / 2, m_size.width(), m_size.height()).adjusted(-2, -2, 2, 2);
}

QPointF InputItem::tip() const
{
	return QPointF(m_size.height() / 2 - m_size.width(), 0);
}

void InputItem::typesConfirmed()
{
	prepareGeometryChange();
	m_size = m_baseSize;
	m_typeInfo = "<div><b>Single Connection</b></div>" + processorItem()->executive()->input(m_index).type().info();
	update();
	foreach (ConnectionItem* ci, filter<ConnectionItem>(scene()->items()))
		if (ci->to() == this)
			ci->rejigEndPoints();
}

void InputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem* _o, QWidget* _w)
{
	float sho2 = m_size.height() / 2;
	float sw = m_size.width();
	float de = .42f;

	QPolygonF po;
	po.append(QPointF(-1, -sho2));
	po.append(QPointF(-sw, -sho2));
	po.append(QPointF(sho2 - sw, 0));
	po.append(QPointF(-sw, sho2));
	po.append(QPointF(-1, sho2));
	QPolygonF pi;
	pi.append(QPointF(0, -sho2 + 1));
	pi.append(QPointF(-sw + 2 + de, -sho2 + 1));
	pi.append(QPointF(-sw + 1 + de + sho2, 0));
	pi.append(QPointF(-sw + 2 + de, sho2 - 1));
	pi.append(QPointF(0, sho2 - 1));
	QPolygonF pf;
	pf.append(QPointF(0.5, -sho2 + 1));
	pf.append(QPointF(-sw + 2 + de, -sho2 + 1));
	pf.append(QPointF(-sw + 1 + de + sho2, 0));
	pf.append(QPointF(-sw + 2 + de, sho2 - 1));
	pf.append(QPointF(0.5, sho2 - 1));

	_p->save();
	_p->translate(0, -pos().y());
	_p->setPen(baseItem()->outerPen());
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
