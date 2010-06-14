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

#include "PropertyItem.h"

class RangePropertyItem: public BasePropertyItem
{
public:
	RangePropertyItem(PropertyItem* _p, QRectF const& _r, AllowedValue _a):
		BasePropertyItem(_p, _r),
		m_a			(_a)
	{}

	virtual void			mousePressEvent(QGraphicsSceneMouseEvent* _e);
	virtual void			mouseMoveEvent(QGraphicsSceneMouseEvent* _e);
	virtual void			paintItem(QPainter* _p, const QStyleOptionGraphicsItem* _o, QWidget* _w);
	virtual float			minWidth() const;

	enum { ItemType = UserType + 18 };
	virtual int				type() const { return ItemType; }

private:
	AllowedValue m_a;
};
