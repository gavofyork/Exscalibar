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

class InputItem;
class OutputItem;
class ProcessorItem;
class MultiProcessorItem;

class MultipleConnectionItem: public QGraphicsPathItem
{
public:
	MultipleConnectionItem(ProcessorItem* _to, ProcessorItem* _from);

	enum { Type = UserType + 9 };
	virtual int type() const { return Type; }

	ProcessorItem* from() const { return m_from; }

	void setValid(bool _v) { m_isValid = _v; }

	ProcessorItem* toProcessor() const;
	ProcessorItem* fromProcessor() const;
	MultiProcessorItem* toMultiProcessor() const;
	MultiProcessorItem* fromMultiProcessor() const;
	ProcessorItem* toIndex() const;
	ProcessorItem* fromIndex() const;

	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;

	void rejigEndPoints();

private:
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

	bool m_isValid;
	ProcessorItem* m_from;
};
