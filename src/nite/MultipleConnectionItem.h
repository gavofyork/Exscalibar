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

#include "ConnectionItem.h"
#include "Magnetic.h"

class InputItem;
class OutputItem;
class MultipleInputItem;
class MultipleOutputItem;
class ProcessorItem;
class MultiProcessorItem;

class MultipleConnectionItem: public QGraphicsPathItem, public Magnetic
{
public:
	typedef ConnectionItem::Nature Nature;

	MultipleConnectionItem(MultipleInputItem* _to, MultipleOutputItem* _from);
	~MultipleConnectionItem();

	enum { ItemType = UserType + 9 };
	virtual int type() const { return ItemType; }

	MultipleInputItem* to() const { return m_to; }
	MultipleOutputItem* from() const { return m_from; }

	void setValid(bool _v) { m_isValid = _v; }

	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;

	void rejigEndPoints();

	QList<QPointF> magnetism(BaseItem const* _b, bool _moving) const;

	static void refreshNature(BaseItem* _b);
	static void refreshNature(MultipleInputItem* _i, MultipleOutputItem* _o, QGraphicsScene* _s);
	void refreshNature();
	Nature nature() const { return m_nature; }

private:
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

	bool m_isValid;

	MultipleOutputItem* m_from;
	MultipleInputItem* m_to;

	QPointF m_centre;

	Nature m_nature;
};
