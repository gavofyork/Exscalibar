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

#include "properties.h"

namespace Geddei
{

Properties::Properties(const PropertiesInfo &info)
{
	for (QMap<QString, QVariant>::ConstIterator i = info.theData.begin(); i != info.theData.end(); i++)
		theData[i.key()] = i.value();
}

void Properties::defaultFrom(PropertiesInfo const& _defaults)
{
	foreach (QString s, _defaults.keys())
		if (!keys().contains(s))
			theData[s] = _defaults.defaultValue(s);
}

void Properties::set(const Properties &pairs)
{
	for (QMap<QString, QVariant>::ConstIterator i = pairs.theData.begin(); i != pairs.theData.end(); i++)
		theData[i.key()] = i.value();
}

void Properties::toBuffer(QBuffer &data) const
{
	data.open(QIODevice::WriteOnly);
	QDataStream out(&data);
	out << theData;
	data.close();
}

void Properties::fromBuffer(QBuffer &data)
{
	data.open(QIODevice::ReadOnly);
	QDataStream in(&data);
	in >> theData;
	data.close();
}

QByteArray Properties::serialise() const
{
	QBuffer buffer;
	toBuffer(buffer);
	return buffer.buffer();
}

void Properties::deserialise(QByteArray &data)
{
	QBuffer buffer(&data);
	fromBuffer(buffer);
}

QString PropertiesInfo::description(const QString &key) const
{
	if (!theInfo.contains(key))
		return "";
	return theInfo[key].description;
}

bool PropertiesInfo::isDynamic(const QString &key) const
{
	if (!theInfo.contains(key))
		return false;
	return theInfo[key].isDynamic;
}

QString PropertiesInfo::symbolOf(const QString &key) const
{
	if (!theInfo.contains(key))
		return "?";
	return theInfo[key].symbol.isNull() ? key.left(1) : theInfo[key].symbol;
}

}
