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

class BaseItem;

class PauseItem: public QGraphicsEllipseItem
{
public:
	PauseItem(QGraphicsItem* _parent, BaseItem* _p, float _s);

	void tick();

protected:
	virtual void paint(QPainter* _p, QStyleOptionGraphicsItem const* _o, QWidget* _w);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent*);

	enum { ItemType = UserType + 6 };
	virtual int type() const { return ItemType; }

	BaseItem* m_baseItem;
};
