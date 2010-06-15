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
#include <QtXml>

#include <Geddei>
using namespace Geddei;

#include "InputItem.h"

class ProcessorBasedItem;
class MultiProcessorItem;

class MultipleInputItem: public InputItem
{
public:
	MultipleInputItem(ProcessorBasedItem* _p, QSizeF const& _size);
	MultipleInputItem(int _i, MultiProcessorItem* _p, QSizeF const& _size);

	void setMultiplicity(uint _m);

	// This or processorItem() will return non-zero.
	MultiProcessorItem* multiProcessorItem() const;

	bool isConnected() const;

	virtual void interPaint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

	virtual void typesConfirmed();

	enum { ItemType = UserType + 12 };
	virtual int type() const { return ItemType; }

private:
	uint		m_multiplicity;
};
