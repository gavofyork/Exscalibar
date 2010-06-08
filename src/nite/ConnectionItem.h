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
#include <QtXml>

#include "Magnetic.h"

class InputItem;
class OutputItem;
class ProcessorItem;

class ConnectionItem: public QGraphicsPathItem, public Magnetic
{
public:
	ConnectionItem(InputItem* _to, OutputItem* _from);

	enum { ItemType = UserType + 4 };
	virtual int type() const { return ItemType; }

	InputItem* to() const { return m_to; }
	OutputItem* from() const { return m_from; }

	void setValid(bool _v) { m_isValid = _v; }

	QList<QPointF> magnetism(BaseItem const* _b, bool _moving) const;

	QPointF wouldAdjust() const;

	ProcessorItem* toProcessor() const;
	ProcessorItem* fromProcessor() const;

	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;

	void rejigEndPoints();

private:
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

	bool m_isValid;
	OutputItem* m_from;
	InputItem* m_to;
};
