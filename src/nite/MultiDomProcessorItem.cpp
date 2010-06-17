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

#include "ConnectionItem.h"
#include "MultipleConnectionItem.h"
#include "MultiDomProcessorItem.h"

MultiDomProcessorItem::MultiDomProcessorItem(QString const& _type, Properties const& _pr, QString const& _name, QSizeF const& _size):
	MultiProcessorItem(_pr, _size)
{
	m_type = _type;
	propertiesChanged(_name);
}

Properties MultiDomProcessorItem::subsProperties()
{
	Properties ret;
	QList<MultiDomProcessorItem*> dpis = all();
	for (int i = (uint)dpis.count() - 1; i >= 0; i--)
		ret = ret.stashed() + dpis[i]->properties();
	return ret;
}

QString MultiDomProcessorItem::composedSubs()
{
	QString ret;
	foreach (MultiDomProcessorItem const* i, all())
		ret += "&" + i->m_type;
	return ret.mid(1);
}

void MultiDomProcessorItem::propertiesChanged(QString const& _newName)
{
	if (executive())
		executive()->update(subsProperties().stashed() + baseProperties());

	if (DomProcessor* n = new DomProcessor(m_type))
	{
		n->init(_newName.isEmpty() ? prototypal() ? prototypal()->name() : QString::number((long unsigned)this) : _newName, properties().stashed() + baseProperties());
		if (!prototypal())
		{
			PropertiesInfo p(n->properties());
			p.destash();
			// TODO: Handle basePropertiesInfo();
			setDefaultProperties(p);
		}
		delete m_processor;
		m_processor = n;
	}
	BaseItem::propertiesChanged(_newName);
}

void MultiDomProcessorItem::paintCentre(QPainter* _p)
{
	QRectF ca = QRectF(QPointF(0, 0), clientRect().size());
	_p->save();
	_p->translate(round((clientRect().size().width() - subPrototypal()->width()) / 2), 0);
	_p->setClipRect(ca);
	subPrototypal()->draw(*_p);
	_p->setClipping(false);
	_p->restore();
}

void MultiDomProcessorItem::prepYourself(ProcessorGroup& _g)
{
	if (!allAfter().isEmpty())
		return;

	m_multiProcessor = new MultiProcessor(new SubFactoryCreator(composedSubs()));
	m_multiProcessor->init(name(), properties().stashed() + baseProperties());
	foreach (MultiDomProcessorItem* i, allBefore())
		i->m_multiProcessor = m_multiProcessor;

	assert(m_multiProcessor);

	MultiProcessorItem::prepYourself(_g);
}

void MultiDomProcessorItem::disconnectYourself()
{
	if (m_multiProcessor)
		MultiProcessorItem::disconnectYourself();
}

void MultiDomProcessorItem::unprepYourself()
{
	MultiProcessorItem::unprepYourself();
	if (allAfter().isEmpty())
		delete m_multiProcessor;
	m_multiProcessor = 0;
}

QList<MultiDomProcessorItem*> MultiDomProcessorItem::allBefore()
{
	if (filter<InputItem>(childItems()).count() == 1)
	{
		InputItem* ii = filter<InputItem>(childItems())[0];
		foreach (MultipleConnectionItem* i, filter<MultipleConnectionItem>(scene()->items()))
			if (i->to() == ii && i->nature() == ConnectionItem::Coupling)
				return QList<MultiDomProcessorItem*>(dynamic_cast<MultiDomProcessorItem*>(i->from()->baseItem())->allBefore()) << dynamic_cast<MultiDomProcessorItem*>(i->from()->baseItem());
	}
	return QList<MultiDomProcessorItem*>();
}

QList<MultiDomProcessorItem*> MultiDomProcessorItem::allAfter()
{
	if (filter<OutputItem>(childItems()).count() == 1)
	{
		OutputItem* ii = filter<OutputItem>(childItems())[0];
		foreach (MultipleConnectionItem* i, filter<MultipleConnectionItem>(scene()->items()))
			if (i->from() == ii && i->nature() == ConnectionItem::Coupling)
				return QList<MultiDomProcessorItem*>() << dynamic_cast<MultiDomProcessorItem*>(i->to()->baseItem()) << dynamic_cast<MultiDomProcessorItem*>(i->to()->baseItem())->allAfter();
	}
	return QList<MultiDomProcessorItem*>();
}

void MultiDomProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	MultiDomProcessorItem* dpi = new MultiDomProcessorItem(_element.attribute("type"));
	dpi->importDom(_element, _scene);
}

QDomElement MultiDomProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	return MultiProcessorItem::saveYourself(_root, _doc, "multidomprocessor");
}
