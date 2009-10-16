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

#include "SubsContainer.h"
#include "MultiProcessorItem.h"

class SubProcessorItem;

class MultiDomProcessorItem: public MultiProcessorItem, public SubsContainer
{
public:
	MultiDomProcessorItem(Properties const& _pr = Properties("Latency/Throughput", 0.0), QSizeF const& _size = QSizeF());

	virtual DomProcessor*	domProcessor() const;

	virtual QDomElement	saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n = "multidomprocessoritem") const;
	static void			fromDom(QDomElement& _element, QGraphicsScene* _scene);

protected:
	virtual void		geometryChanged();
	virtual QSizeF		centreMin() const;
	virtual Properties	completeProperties() const { return SubsContainer::completeProperties(); }
	virtual void		propertiesChanged() { MultiProcessorItem::propertiesChanged(); }

	virtual QList<SubProcessorItem*> subProcessorItems() const;
	virtual BaseItem*	baseItem() { return this; }

	virtual MultiProcessorCreator* newCreator();
	virtual void		postCreate();
};
