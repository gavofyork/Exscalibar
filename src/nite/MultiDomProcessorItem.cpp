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
#include "MultiDomProcessorItem.h"

MultiDomProcessorItem::MultiDomProcessorItem(Properties const& _pr, QSizeF const& _size):
	MultiProcessorItem(_pr, _size)
{
	delete m_controls;
	m_controls = 0;
	propertiesChanged();
}

void MultiDomProcessorItem::postCreate()
{
	PropertiesInfo pi = m_processor->properties();
	baseItem()->setDefaultProperties(pi.destash());
	foreach (SubProcessorItem* i, ordered())
		i->setDefaultProperties(pi.destash());
}

void MultiDomProcessorItem::paintCentre(QPainter* _p)
{
	paintFrames(_p);
}

DomProcessor* MultiDomProcessorItem::domProcessor() const
{
	return dynamic_cast<DomProcessor*>(processor());
}

QSizeF MultiDomProcessorItem::centreMin() const
{
	QSizeF s = SubsContainer::centreMin();
	return QSizeF(max(MultiProcessorItem::centreMin().width(), s.width()), max(MultiProcessorItem::centreMin().height(), s.height()));
}

QList<SubProcessorItem*> MultiDomProcessorItem::subProcessorItems() const
{
	return filter<SubProcessorItem>(childItems());
}

void MultiDomProcessorItem::geometryChanged()
{
	SubsContainer::geometryChanged();
	MultiProcessorItem::geometryChanged();
}

MultiProcessorCreator* MultiDomProcessorItem::newCreator()
{
	QString cs = composedSubs();
	if (cs.isEmpty())
		return 0;
	return new SubFactoryCreator(cs);
}

void MultiDomProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	MultiDomProcessorItem* dpi = new MultiDomProcessorItem;
	dpi->SubsContainer::importDom(_element, _scene);
	dpi->MultiProcessorItem::importDom(_element, _scene);
}

QDomElement MultiDomProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement us = MultiProcessorItem::saveYourself(_root, _doc, "multidomprocessor");
	SubsContainer::exportDom(us, _doc);
	return us;
}
