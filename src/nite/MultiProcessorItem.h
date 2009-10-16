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

#include "BaseItem.h"

class MultiProcessorItem: public BaseItem
{
public:
	MultiProcessorItem(QString const& _type, Properties const& _pr = Properties(), QString const& _name = QString::null, QSizeF const& _size = QSizeF(0, 0));
	~MultiProcessorItem();

	enum { Type = BaseItem::Type + 2 };
	virtual int			type() const { return Type; }

	MultiProcessor*		multiProcessor() const { return m_multiProcessor; }
	Processor*			processor() const { return m_processor; }

	virtual void		prepYourself(ProcessorGroup&);
	virtual bool		connectYourself();
	virtual void		disconnectYourself();

	static void			fromDom(QDomElement& _element, QGraphicsScene* _scene);
	virtual QDomElement saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n = "multiprocessor") const;

	virtual QSizeF		centreMin() const;

protected:
	MultiProcessorItem(Properties const& _pr = Properties(), QSizeF const& _size = QSizeF(0, 0));

	virtual void		propertiesChanged(QString const& _newName = QString::null);
	virtual void		geometryChanged();
	virtual void		positionChanged();

	virtual MultiProcessorCreator* newCreator() { return new FactoryCreator(m_type); }
	virtual void		postCreate() {}

private:
	QString				m_type;
	MultiProcessor*		m_multiProcessor;
	Processor*			m_processor;
};
