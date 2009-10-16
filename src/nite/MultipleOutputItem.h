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

#pragma once

#include <QtGui>

#include <Geddei>
using namespace Geddei;

class ProcessorItem;
class MultiProcessorItem;

class MultipleOutputItem: public QGraphicsItem
{
public:
	MultipleOutputItem(ProcessorItem* _p, QSizeF const& _size);
	MultipleOutputItem(int _i, MultiProcessorItem* _p, QSizeF const& _size);

	void setMultiplicity(uint _m);

	BaseItem* baseItem() const;

	// One of these two will return non-zero.
	ProcessorItem* processorItem() const;
	MultiProcessorItem* multiProcessorItem() const;

	MultiSource* source() const;

	bool isConnected() const;

	inline uint index() const { return m_index; }
	inline QSizeF size() const { return m_size; }

	QPointF tip() const;

	enum { Type = UserType + 13 };
	virtual int type() const { return Type; }

private:
	virtual QRectF boundingRect() const;
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent*);

	uint			m_multiplicity;
	QSizeF			m_size;
	uint			m_index;
	bool			m_hover;
};
