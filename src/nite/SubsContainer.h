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

#include "ProcessorItem.h"

class SubProcessorItem;

class SubsContainer
{
	friend class SubProcessorItem;

public:
	SubsContainer();

	virtual DomProcessor* domProcessor() const = 0;
	virtual BaseItem*	baseItem() = 0;
	BaseItem const*		baseItem() const { return const_cast<SubsContainer*>(this)->baseItem(); }
	virtual QList<SubProcessorItem*>	subProcessorItems() const = 0;

	void				reorder();
	virtual void		propertiesChanged() = 0;

protected:
	void				geometryChanged();
	QSizeF				centreMin() const;
	Properties			completeProperties() const;
	virtual float		margin() const { return 5.f; }
	virtual float		padding() const { return 2.f; }
	void				paintFrames(QPainter*) const;

	QString				composedSubs() const;
	QList<SubProcessorItem*> ordered() const;

	void				importDom(QDomElement& _item, QGraphicsScene* _scene);
	void				exportDom(QDomElement& _item, QDomDocument& _doc) const;

	virtual void		subPropertiesChanged() { propertiesChanged(); }
};
