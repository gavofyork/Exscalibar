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

#include "OutputItem.h"

class ProcessorItem;
class MultiProcessorItem;

class MultipleOutputItem: public OutputItem
{
public:
	MultipleOutputItem(ProcessorItem* _p, QSizeF const& _size);
	MultipleOutputItem(int _i, MultiProcessorItem* _p, QSizeF const& _size);

	void setMultiplicity(uint _m);

	// This or processorItem() will return non-zero.
	MultiProcessorItem* multiProcessorItem() const;
	MultiSource* source() const;
	bool isConnected() const;

	enum { ItemType = UserType + 13 };
	virtual int type() const { return ItemType; }

private:
	virtual void interPaint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent*);

	uint			m_multiplicity;
};
