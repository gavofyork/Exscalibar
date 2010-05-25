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

#include <QtGui>

#include <Geddei>
using namespace Geddei;

class WithProperties: public QGraphicsItem
{
public:
	WithProperties(QGraphicsItem* _p, Properties const& _pr = Properties()): QGraphicsItem(_p), m_properties(_pr) {}

	enum { Type = UserType + 16 };
	virtual int			type() const { return Type; }

	inline QVariant					property(QString const& _key) const { return properties()[_key]; }
	inline PropertiesDatum			propertiesInfo(QString const& _key) const { return propertiesInfo().datum(_key); }
	virtual Properties const&		properties() const { return m_properties; }
	virtual PropertiesInfo const&	propertiesInfo() const { return m_propertiesInfo; }
	virtual void					setProperty(QString const& _key, QVariant const& _value) { m_properties[_key] = _value; }
	virtual void					setPropertiesInfo(PropertiesInfo const& _i);

protected:
	Properties			m_properties;
	PropertiesInfo		m_propertiesInfo;
	QStringList			m_dynamicKeys;
};
