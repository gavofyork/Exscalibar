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

#include "MultiProcessorItem.h"

class ControlsItem: public QGraphicsRectItem
{
public:
	ControlsItem(MultiProcessorItem* _p, float _size = 10);

	MultiProcessorItem* mpi() const { return dynamic_cast<MultiProcessorItem*>(parentItem()); }
	void init();

protected:
	uint whichRect(QPointF _pos) const;
	QRectF rectOf(uint _i) const;
	virtual void paint(QPainter* _p, QStyleOptionGraphicsItem const*, QWidget*);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);

	uint m_clickedRect;
	float m_size;
};

