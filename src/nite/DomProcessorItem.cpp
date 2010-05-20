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

#include "SubProcessorItem.h"
#include "DomProcessorItem.h"

DomProcessorItem::DomProcessorItem(Properties const& _pr, QSizeF const& _size):
	ProcessorItem(QString::null, _pr, _size)
{
}

DomProcessor* DomProcessorItem::domProcessor() const
{
	return dynamic_cast<DomProcessor*>(processor());
}

QSizeF DomProcessorItem::centreMin() const
{
	QSizeF s = SubsContainer::centreMin();
	return QSizeF(max(ProcessorItem::centreMin().width(), s.width()), max(ProcessorItem::centreMin().height(), s.height()));
}

QList<SubProcessorItem*> DomProcessorItem::subProcessorItems() const
{
	return filter<SubProcessorItem>(childItems());
}

void DomProcessorItem::geometryChanged()
{
	SubsContainer::geometryChanged();
	ProcessorItem::geometryChanged();
}

void DomProcessorItem::paintCentre(QPainter* _p)
{
	paintFrames(_p);
}

Processor* DomProcessorItem::reconstructProcessor()
{
	QString cs = composedSubs();
	if (cs.isEmpty())
		return 0;
	Processor* p = new DomProcessor(cs);
	PropertiesInfo pi = p->properties();

	baseItem()->setDefaultProperties(pi.destash());
	foreach (SubProcessorItem* i, ordered())
		i->setDefaultProperties(pi.destash());

	return p;
}

void DomProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	DomProcessorItem* dpi = new DomProcessorItem;
	dpi->ProcessorItem::importDom(_element, _scene);
	dpi->SubsContainer::importDom(_element, _scene);
	dpi->setName(_element.attribute("name"));
}

QDomElement DomProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement us = ProcessorItem::saveYourself(_root, _doc, "domprocessor");
	SubsContainer::exportDom(us, _doc);
	return us;
}
