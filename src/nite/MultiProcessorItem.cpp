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

#include "MultiProcessorItem.h"

MultiProcessorItem::MultiProcessorItem(QString const& _type, Properties const& _pr, QString const& _name, QSizeF const& _size):
	BaseItem			(_pr, _size),
	m_creator			(new FactoryCreator(_type)),
	m_multiProcessor	(0),
	m_processor			(0)
{
	propertiesChanged(_name);
}

MultiProcessorItem::~MultiProcessorItem()
{
	delete m_multiProcessor;
	delete m_processor;
	delete m_creator;
}

void MultiProcessorItem::propertiesChanged(QString const& _newName)
{
	QString name = _newName;
	if (name.isEmpty())
		name = QString::number((long uint)(this));

	delete m_multiProcessor;
	delete m_processor;
	m_multiProcessor = new MultiProcessor(m_creator);
	m_processor = m_creator->newProcessor();
	m_multiProcessor->init(name, completeProperties());
	m_processor->init(name, completeProperties());
}
