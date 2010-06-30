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
#include "DomProcessorItem.h"

DomProcessorItem::DomProcessorItem(QString const& _type, Properties const& _pr, QSizeF const& _size):
	ProcessorBasedItem	(_pr, _size),
	m_combined			(0),
	m_prototypal		(0),
	m_type				(_type)
{
	if (!m_type.isEmpty())
		propertiesChanged();
}

Properties DomProcessorItem::subsProperties()
{
	Properties ret;
	QList<DomProcessorItem*> dpis = all();
	for (int i = (uint)dpis.count() - 1; i >= 0; i--)
		ret = ret.stashed() + dpis[i]->properties();
	return ret;
}

QString DomProcessorItem::composedSubs()
{
	QString ret;
	foreach (DomProcessorItem const* i, all())
		ret += "&" + i->m_type;
	return ret.mid(1);
}

void DomProcessorItem::propertiesChanged(QString const& _newName)
{
	if (executive())
		executive()->update(subsProperties().stashed() + baseProperties());

	if (DomProcessor* n = new DomProcessor(m_type))
	{
		n->init(_newName.isEmpty() ? m_prototypal ? m_prototypal->name() : QString::number((long unsigned)this) : _newName, properties().stashed() + baseProperties());
		if (!m_prototypal)
		{
			PropertiesInfo p(n->properties());
			p.destash();
			setDefaultProperties(p);
		}
		delete m_prototypal;
		m_prototypal = n;
	}
	ProcessorBasedItem::propertiesChanged(_newName);
}

void DomProcessorItem::paintCentre(QPainter* _p)
{
	QRectF ca = QRectF(QPointF(0, 0), clientRect().size());
	_p->save();
	_p->translate(round((clientRect().size().width() - subPrototypal()->width()) / 2), 0);
	_p->setClipRect(ca);
	subPrototypal()->draw(*_p);
	_p->setClipping(false);
	_p->restore();
}

QList<DomProcessorItem*> DomProcessorItem::allBefore()
{
	DomProcessorItem* dpi = 0;
	if (inputs().count() == 1 && inputs().first()->connections().count() == 1 && (dpi = dynamic_cast<DomProcessorItem*>(inputs().first()->connections().first()->fromProcessor())) && dpi->outputs().count() == 1 && dpi->outputs().first()->connections().count() == 1)
		return QList<DomProcessorItem*>(dpi->allBefore()) << dpi;
	return QList<DomProcessorItem*>();
}

QList<DomProcessorItem*> DomProcessorItem::allAfter()
{
	DomProcessorItem* dpi = 0;
	if (outputs().count() == 1 && outputs().first()->connections().count() == 1 && (dpi = dynamic_cast<DomProcessorItem*>(outputs().first()->connections().first()->toProcessor())) && dpi->inputs().count() == 1 && dpi->inputs().first()->connections().count() == 1)
		return QList<DomProcessorItem*>() << dpi << dpi->allAfter();
	return QList<DomProcessorItem*>();
}

void DomProcessorItem::prepYourself(ProcessorGroup& _g)
{
	if (allAfter().size() != 0)
		return;

	m_combined = new DomProcessor(composedSubs());
	m_combined->init(name(), subsProperties().stashed() + baseProperties());

	foreach (DomProcessorItem* d, allBefore())
		d->m_combined = m_combined;

	ProcessorBasedItem::prepYourself(_g);

	//if (filter<OutputItem>(childItems()).count() && filter<OutputItem>(childItems()).first()->isConnected() && filter<OutputItem>(childItems()).first()->connections().first()->nature() == ConnectionItem::Reaper)
	executive()->setNoGroup();
}

void DomProcessorItem::unprepYourself()
{
	if (allAfter().size() == 0)
		delete m_combined;
	m_combined = 0;
	ProcessorBasedItem::unprepYourself();
}

void DomProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	DomProcessorItem* pi = new DomProcessorItem(_element.attribute("type"));
	pi->importDom(_element, _scene);
}

QDomElement DomProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement proc = _doc.createElement("domprocessor");
	proc.setAttribute("type", m_type);
	exportDom(proc, _doc);
	_root.appendChild(proc);
	return proc;
}
