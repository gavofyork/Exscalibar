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

#include <QtXml>
#include <QtGui>

#include <Geddei>
using namespace Geddei;

class SubsContainer;

class SubProcessorItem: public QGraphicsItem
{
	friend class SubsContainer;// so it can have its properties populated.

public:
	SubProcessorItem(SubsContainer* _dpi, QString const& _type, int _index = 0, Properties const& _pr = Properties());

	QSizeF size() const { return QSizeF(subProcessor()->width(), subProcessor()->height()); }

	uint index() const { return m_index; }
	SubsContainer* subsContainer() const;
	DomProcessor* domProcessor() const;
	SubProcessor* subProcessor() const;

	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;
	static void fromDom(QDomElement const& _element, SubsContainer* _dpi);

	QString spType() const { return m_type; }

	enum { Type = UserType + 8 };
	virtual int type() const { return Type; }

	Properties const& properties() const { return m_properties; }
	void setProperty(QString const& _key, QVariant const& _value);
	void setDefaultProperties(PropertiesInfo const& _p) { m_properties.defaultFrom(_p); }

private:
	virtual QRectF boundingRect() const
	{
		return QRectF(QPointF(-2, -2), size() + QSizeF(4, 4));
	}
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* _e);

	Properties			m_properties;
	QString				m_type;
	uint				m_index;
};
