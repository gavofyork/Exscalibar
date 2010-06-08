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
	InputItem(int _i, BaseItem* _p, QSizeF const& _size);

	ProcessorItem* processorItem() const;
	BaseItem* baseItem() const;

	bool isConnected() const;

	inline uint index() const { return m_index; }
	inline QSizeF size() const { return m_size; }

	virtual QRectF boundingRect() const;
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual void interPaint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}
	QPointF tip() const;

	void typesConfirmed();

	QString const& typeInfo() const { return m_typeInfo; }

	enum { ItemType = UserType + 2 };
	virtual int type() const { return ItemType; }

protected:
	uint		m_index;
	QSizeF		m_size;
	QSizeF		m_baseSize;
	QString		m_typeInfo;
};
