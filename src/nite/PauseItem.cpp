#if 0
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

#include <QtGui>

#include "BaseItem.h"
#include "PauseItem.h"

PauseItem::PauseItem(QGraphicsItem* _parent, BaseItem* _p, float _s):
	QGraphicsEllipseItem	(_parent),
	m_baseItem				(_p)
{
	setPen(Qt::NoPen);
	setBrush(Qt::NoBrush);
	setRect(QRectF(0, 0, _s, _s));
}

void PauseItem::paint(QPainter* _p, QStyleOptionGraphicsItem const* _o, QWidget* _w)
{
	_p->save();
	QGraphicsEllipseItem::paint(_p, _o, _w);
	_p->restore();
	if (m_baseItem->isRunning())
	{
		_p->setPen(QPen(QColor(0, 0, 0, 32), 2));
		_p->translate(QPointF(rect().width() / 2, rect().height() / 2));
		_p->rotate(m_baseItem->cyclePoint());
		_p->drawPoint(QPointF(rect().height() / 2, 0));
		_p->setPen(QPen(QColor(0, 0, 0, 64), 2));
		_p->rotate(360.0 / 12);
		_p->drawPoint(QPointF(rect().height() / 2, 0));
	}
}

void PauseItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
	m_baseItem->togglePause();
	update();
}

void PauseItem::tick()
{
	update();
}
#endif
