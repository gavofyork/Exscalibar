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

InputItem::InputItem(int _i, ProcessorItem* _p): QGraphicsItem(_p), m_index(_i)
{
	m_size = QSizeF(10 + cornerSize, portSize);
	setPos(cornerSize - m_size.width() - 1, cornerSize + cornerSize / 2 + portLateralMargin + (portLateralMargin + portSize) * m_index);
}

ProcessorItem* InputItem::processorItem() const
{
	return static_cast<ProcessorItem*>(parentItem());
}

QRectF InputItem::boundingRect() const
{
	return QRectF(QPointF(0.0, 0.0), m_size);
}

QPointF InputItem::tip() const
{
	return QPointF(portSize / 2, portSize / 2);
}

void InputItem::typesConfirmed()
{
	prepareGeometryChange();
	double secs = processorItem()->m_processor->input(m_index).capacity() / processorItem()->m_processor->input(m_index).type().frequency();
	m_size = QSizeF(log(1.0 + secs) / log(2) * 16 + 10 + cornerSize, portSize);
	update();
	foreach (QGraphicsItem* i, childItems())
		if (ConnectionItem* ci = qgraphicsitem_cast<ConnectionItem*>(i))
			ci->rejigEndPoints();
}

void InputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->setPen(QPen(Qt::black, 0));
	_p->setBrush(QColor::fromHsv(40, 128, 220, 255));
	QPolygonF p;
	p.append(QPointF(0, 0));
	p.append(QPointF(portSize / 2, portSize / 2));
	p.append(QPointF(0, portSize));
	p.append(QPointF(m_size.width(), portSize));
	p.append(QPointF(m_size.width(), 0));
	_p->drawPolygon(p);

	if (processorItem()->m_processor->isRunning())	//  a bit unsafe, since the processor could stop & get reset between these two.
	{
		double fill = processorItem()->m_processor->input(m_index).filled();
		_p->fillRect(QRectF(cornerSize + (m_size.width() - cornerSize) * (1.f - fill), portSize / 4, (m_size.width() - cornerSize) * fill, portSize / 2), QColor(Qt::darkRed));
	}
}
