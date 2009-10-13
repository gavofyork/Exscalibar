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

class MultiProcessorItem: public QGraphicsItem
{
public:
	MultiProcessorItem(QString const& _type, Properties const& _pr = Properties(), QString const& _name = QString::null, QSizeF const& _size = QSizeF(0, 0));
	~MultiProcessorItem();

	enum { Type = UserType + 10 };
	virtual int			type() const { return Type; }

	MultiProcessor*		multiProcessor() const { return m_multiProcessor; }
	QRectF				clientArea() const;

	static void			fromDom(QDomElement& _element, QGraphicsScene* _scene);
	virtual QDomElement saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n = "multiprocessor") const;

	virtual bool		connectYourself(ProcessorGroup& _group);
	void				typesConfirmed();
	virtual void		disconnectYourself();

	Properties const&	properties() const { return m_properties; }
	void				setProperty(QString const& _key, QVariant const& _value);
	virtual void		propertiesChanged(QString const& _newName = QString::null);

	void				setPos(qreal _x, qreal _y) { QGraphicsItem::setPos(round(_x) - .5f, round(_y) - .5f); }
	void				setPos(QPointF _p) { QGraphicsItem::setPos(round(_p.x()) - .5f, round(_p.y()) - .5f); }

protected:
	virtual QRectF		boundingRect() const;
	virtual void		paint(QPainter*, QStyleOptionGraphicsItem const*, QWidget*);

	FactoryCreator*		m_creator;
	MultiProcessor*		m_multiProcessor;
	Properties			m_properties;
	Processor*			m_processor;
};
