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

class ProcessorItem;

class InputItem: public QGraphicsItem
{
public:
	InputItem(int _i, ProcessorItem* _p);

	ProcessorItem* processorItem() const;
	inline uint index() const { return m_index; }
	inline QSizeF size() const { return m_size; }

	virtual QRectF boundingRect() const;
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	QPointF tip() const;

	static void fromDom(QDomElement _element, ProcessorItem* _parent)
	{
		new InputItem(_element.attribute("index").toInt(), _parent);
	}

	void saveYourself(QDomElement& _parent, QDomDocument& _doc) const
	{
		QDomElement out = _doc.createElement("input");
		out.setAttribute("index", m_index);
		_parent.appendChild(out);
	}

	void typesConfirmed();

	enum { Type = UserType + 2 };
	virtual int type() const { return Type; }

private:
	uint		m_index;
	QSizeF		m_size;
};
