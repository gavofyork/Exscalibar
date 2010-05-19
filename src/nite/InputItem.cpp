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

bool InputItem::isConnected() const
{
	return filter<ConnectionItem>(childItems()).size();
}

QRectF InputItem::boundingRect() const
{
	return QRectF(-m_size.width(), -m_size.height() / 2, m_size.width(), m_size.height() / 2);
}

QPointF InputItem::tip() const
{
	return QPointF(m_size.height() / 2 - m_size.width(), 0);
}

void InputItem::typesConfirmed()
{
	prepareGeometryChange();
	double secs = processorItem()->m_processor->input(m_index).capacity() / processorItem()->m_processor->input(m_index).type().frequency();
	m_size = QSizeF(log(1.0 + secs) / log(2) * 16 + m_baseSize.width(), m_baseSize.height());
	m_typeInfo = "<div><b>Single Connection</b></div>" + processorItem()->m_processor->input(m_index).type().info();
	update();
	foreach (QGraphicsItem* i, childItems())
		if (ConnectionItem* ci = dynamic_cast<ConnectionItem*>(i))
			ci->rejigEndPoints();
}

void InputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->setPen(QPen(Qt::black, 0));
	_p->setBrush(QColor::fromHsv(40, 128, 220));
	QPolygonF p;
	p.append(QPointF(m_size.height() / 2 - m_size.width(), 0));
	p.append(QPointF(-m_size.width(), m_size.height() / 2));
	p.append(QPointF(0, m_size.height() / 2));
	p.append(QPointF(0, -m_size.height() / 2));
	p.append(QPointF(-m_size.width(), -m_size.height() / 2));
	_p->drawPolygon(p);

	if (processorItem()->m_processor->isRunning())	//  a bit unsafe, since the processor could stop & get reset between these two.
	{
		double fill = processorItem()->m_processor->input(m_index).filled();
		_p->fillRect(QRectF(0, -m_size.height() / 4, -(m_size.width() - m_baseSize.width()) * fill, m_size.height() / 2) , QColor(Qt::darkRed));
	}
}
