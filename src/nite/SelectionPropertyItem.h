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

#include "PropertyItem.h"

class SelectionPropertyItem: public BasePropertyItem
{
public:
	SelectionPropertyItem(PropertyItem* _p, QRectF const& _r);

	enum { ItemType = UserType + 19 };
	virtual int			type() const { return ItemType; }

	virtual float		minWidth() const;
	virtual void		mousePressEvent(QGraphicsSceneMouseEvent* _e);
	virtual void		paintItem(QPainter* _p, const QStyleOptionGraphicsItem* _o, QWidget* _w);

private:
	QStringList			m_syms;
};
