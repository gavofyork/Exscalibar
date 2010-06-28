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

#pragma once

#include <QtGui>

class ProcessorBasedItem;
class ConnectionItem;

class OutputItem: public QGraphicsItem
{
public:
	OutputItem(int _i, BaseItem* _p, QSizeF const& _size);

	ProcessorBasedItem* processorItem() const;
	BaseItem* baseItem() const;
	inline uint index() const { return m_index; }
	inline QSizeF size() const { return m_size; }

	bool isConnected() const;
	QList<ConnectionItem*> connections() const;

	QPointF tip() const;

	enum { ItemType = UserType + 3 };
	virtual int type() const { return ItemType; }

	virtual QRectF boundingRect() const;

protected:
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual void interPaint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent*);

	QSizeF			m_size;
	uint			m_index;
	bool			m_hover;
};
