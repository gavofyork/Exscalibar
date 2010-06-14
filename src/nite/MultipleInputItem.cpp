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

#include "MultipleConnectionItem.h"
#include "ConnectionItem.h"
#include "ProcessorBasedItem.h"
#include "MultiProcessorItem.h"
#include "MultipleInputItem.h"

MultipleInputItem::MultipleInputItem(ProcessorBasedItem* _p, QSizeF const& _size):
	InputItem		(0, _p, _size),
	m_multiplicity	(Undefined)
{
}

MultipleInputItem::MultipleInputItem(int _i, MultiProcessorItem* _p, QSizeF const& _size):
	InputItem		(_i, _p, _size),
	m_multiplicity	(Undefined)
{
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
	if (!scene())
		return false;
	foreach (MultipleConnectionItem* mci, filter<MultipleConnectionItem>(scene()->items()))
		if (mci->to() == this)
			return true;
	return false;
}

MultiProcessorItem* MultipleInputItem::multiProcessorItem() const
{
	return dynamic_cast<MultiProcessorItem*>(parentItem());
}

void MultipleInputItem::typesConfirmed()
{
	if (processorItem() && processorItem()->executive()->numInputs())
		m_typeInfo = processorItem()->executive()->input(0).type().info();
	else if (multiProcessorItem() && (int)multiProcessorItem()->processor()->numInputs() > (int)m_index)
		m_typeInfo = multiProcessorItem()->processor()->input(m_index).type().info();
	m_typeInfo = QString("<div><b>Multiple Connection</b></div><div>Arity: %1</div>").arg(m_multiplicity) + m_typeInfo;
}

void MultipleInputItem::interPaint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	QString s = m_multiplicity != Undefined ? QString::number(m_multiplicity) : "?";
	QFont f;
	f.setBold(true);
	QRectF br = boundingRect().adjusted(m_size.height() / 2 + 4, 4, -4, -4);
	f.setPixelSize(br.height());
	_p->setFont(f);
	_p->setPen(QColor(0, 0, 0, 96));
	_p->drawText(br.translated(0, 1), Qt::AlignCenter, s);
	_p->setPen(Qt::white);
	_p->drawText(br, Qt::AlignCenter, s);
}
