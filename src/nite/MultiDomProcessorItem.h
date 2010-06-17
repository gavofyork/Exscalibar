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

#include <QtXml>
#include <QtGui>

#include <Geddei>
using namespace Geddei;

#include "MultiProcessorItem.h"

class MultiDomProcessorItem: public MultiProcessorItem
{
public:
	MultiDomProcessorItem(QString const& _type, Properties const& _pr = Properties("Latency/Throughput", 0.0), QString const& _name = QString::null, QSizeF const& _size = QSizeF());

	Properties			baseProperties() const { return Properties("Latency/Throughput", 0.0); }
	PropertiesInfo		basePropertiesInfo() const { return PropertiesInfo("Processors Multiplicity", 0, "The number of processors that make this object. (>0 to set, 0 for automatic)"); }

	enum { ItemType = UserType + 15 };
	virtual int			type() const { return ItemType; }

	virtual QDomElement	saveYourself(QDomElement& _root, QDomDocument& _doc) const;
	static void			fromDom(QDomElement& _element, QGraphicsScene* _scene);

	virtual QString		typeName() const { return m_type; }

	QList<MultiDomProcessorItem*> allBefore();
	QList<MultiDomProcessorItem*> allAfter();
	inline QList<MultiDomProcessorItem*> all() { return QList<MultiDomProcessorItem*>(allBefore()) << this << allAfter(); }

	DomProcessor*		domPrototypal() const { return dynamic_cast<DomProcessor*>(m_processor); }
	SubProcessor*		subPrototypal() const { return domPrototypal()->primary(); }

	Properties			subsProperties();
	QString				composedSubs();

protected:
	virtual void		propertiesChanged(QString const& _name);

	virtual void		prepYourself(ProcessorGroup&);
	virtual void		disconnectYourself();
	virtual void		unprepYourself();

	virtual QColor		outlineColour() const { return BaseItem::outlineColour(); }
	virtual uint		redrawPeriod() const { return 0; }
	virtual bool		isResizable() const { return false; }

	virtual void		paintCentre(QPainter* _p);
};
