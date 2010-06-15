/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

MultipleOutputItem::MultipleOutputItem(ProcessorBasedItem* _p, QSizeF const& _size):
	OutputItem		(0, _p, _size),
	m_multiplicity	(Undefined)
{
}

MultipleOutputItem::MultipleOutputItem(int _i, MultiProcessorItem* _p, QSizeF const& _size):
	OutputItem		(_i, _p, _size),
	m_multiplicity	(Undefined)
{
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
	return processorItem() ? static_cast<MultiSource*>(processorItem()->executive()) : static_cast<MultiSource*>(multiProcessorItem()->multiProcessor());
}

MultiProcessorItem* MultipleOutputItem::multiProcessorItem() const
{
	return dynamic_cast<MultiProcessorItem*>(baseItem());
}

void MultipleOutputItem::interPaint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	QString s = m_multiplicity != Undefined ? QString::number(m_multiplicity) : "?";
	QFont f;
	f.setBold(true);
	QRectF br = boundingRect().adjusted(2, 4, -2 - m_size.height() / 2 - 1, -4);
	f.setPixelSize(br.height());
	_p->setFont(f);
	_p->setPen(QColor(0, 0, 0, 96));
	_p->drawText(br.translated(0, 1), Qt::AlignCenter, s);
	_p->setPen(Qt::white);
	_p->drawText(br, Qt::AlignCenter, s);
}

void MultipleOutputItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
	qobject_cast<ProcessorsScene*>(scene())->beginMultipleConnect(this);
}
