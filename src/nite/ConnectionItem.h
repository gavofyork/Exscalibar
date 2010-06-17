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

#include "Magnetic.h"

class InputItem;
class OutputItem;
class ProcessorBasedItem;

class ConnectionItem: public QGraphicsPathItem, public Magnetic
{
public:
	enum Nature { Connection, Coupling };

	ConnectionItem(InputItem* _to, OutputItem* _from);
	virtual ~ConnectionItem();

	enum { ItemType = UserType + 4 };
	virtual int type() const { return ItemType; }

	InputItem* to() const { return m_to; }
	OutputItem* from() const { return m_from; }

	void setValid(bool _v) { m_isValid = _v; }

	QList<QPointF> magnetism(BaseItem const* _b, bool _moving) const;

	QPointF wouldAdjust() const;

	ProcessorBasedItem* toProcessor() const;
	ProcessorBasedItem* fromProcessor() const;
	BaseItem* toBase() const;
	BaseItem* fromBase() const;

	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;

	void rejigEndPoints();

	void refreshNature();
	Nature nature() const { return m_nature; }

private:
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

	bool m_rejigging;
	bool m_isValid;
	OutputItem* m_from;
	InputItem* m_to;

	Nature m_nature;
};
