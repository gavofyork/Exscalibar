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
#include "ProcessorsView.h"
#include "ProcessorItem.h"
#include "MultipleInputItem.h"
#include "MultipleOutputItem.h"

// TODO: MultipleOutputPort
// qobject_cast<ProcessorsScene*>(scene())->beginMultipleConnect(this);

ProcessorItem::ProcessorItem(QString const& _type, Properties const& _pr, QSizeF const& _size):
	ProcessorBasedItem		(_pr, _size),
	m_nominal				(0),
	m_type					(_type),
	m_propertiesDirty		(false)
{
	if (!m_type.isEmpty())
		propertiesChanged();
}

void ProcessorItem::propertiesChanged(QString const& _newName)
{
	if (m_nominal && m_nominal->isRunning())
	{
		m_nominal->update(completeProperties());
		m_propertiesDirty = true;
		return;
	}

	m_propertiesDirty = false;
	if (Processor* n = ProcessorFactory::create(m_type))
	{
		n->init(_newName.isEmpty() ? m_nominal ? m_nominal->name() : QString::number((long unsigned)this) : _newName, completeProperties());
		if (!m_nominal)
			setDefaultProperties(n->properties());
		delete m_nominal;
		m_nominal = n;
	}
	BaseItem::propertiesChanged(_newName);
}

void ProcessorItem::unprepYourself()
{
	ProcessorBasedItem::unprepYourself();
	if (m_propertiesDirty)
		propertiesChanged();
}

void ProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	ProcessorItem* pi = new ProcessorItem(_element.attribute("type"));
	pi->importDom(_element, _scene);
}

QDomElement ProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const
{
	QDomElement proc = _doc.createElement(_n);
	proc.setAttribute("type", m_nominal->type());
	BaseItem::exportDom(proc, _doc);
	_root.appendChild(proc);
	return proc;
}
