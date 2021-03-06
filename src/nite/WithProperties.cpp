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

#include "GeddeiNite.h"
#include "WithProperties.h"

void WithProperties::setPropertiesInfo(PropertiesInfo const& _i)
{
	m_properties.defaultFrom(_i);
	m_propertiesInfo = _i;
	m_dynamicKeys.clear();
	foreach (QString k, _i.keys())
		if (_i.datum(k).allowed.size())
			m_dynamicKeys << k;
}

void WithProperties::setProperty(QString const& _key, QVariant const& _value)
{
	m_properties[_key] = _value;
}
