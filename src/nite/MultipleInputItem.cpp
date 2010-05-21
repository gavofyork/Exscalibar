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
	return QRectF(-m_size.width(), -m_size.height() / 2, m_size.width(), m_size.height());
}

QPointF MultipleInputItem::tip() const
{
	return QPointF(m_size.height() / 2 - m_size.width(), 0);
}

void MultipleInputItem::typesConfirmed()
{
	if (processorItem() && processorItem()->processor()->numInputs())
		m_typeInfo = processorItem()->processor()->input(0).type().info();
	else if (multiProcessorItem() && (int)multiProcessorItem()->processor()->numInputs() > m_index)
		m_typeInfo = multiProcessorItem()->processor()->input(m_index).type().info();
	m_typeInfo = QString("<div><b>Multiple Connection</b></div><div>Arity: %1</div>").arg(m_multiplicity) + m_typeInfo;
}

void MultipleInputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->save();
	_p->translate(0, -pos().y());
	{
		QPolygonF p;
		p.append(QPointF(-1, -m_size.height() / 2));
		p.append(QPointF(-m_size.width(), -m_size.height() / 2));
		p.append(QPointF(m_size.height() / 2 - m_size.width(), 0));
		p.append(QPointF(-m_size.width(), m_size.height() / 2));
		p.append(QPointF(-1, m_size.height() / 2));
		_p->setPen(baseItem()->outerPen());
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
	QString s = m_multiplicity != Undefined ? QString::number(m_multiplicity) : "?";
	QFont f;
	f.setBold(true);
	QRectF br = boundingRect().adjusted(m_size.height() / 2 + 2, 2, -2, -2);
	f.setPixelSize(br.height());
	_p->setFont(f);
	_p->setPen(QColor(0, 0, 0, 96));
	_p->drawText(br.translated(0, 1), Qt::AlignCenter, s);
	_p->setPen(Qt::white);
	_p->drawText(br, Qt::AlignCenter, s);
}
