/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include "properties.h"

namespace Geddei
{

Properties::Properties(const PropertiesInfo &info)
{
	for (QMap<QString, QVariant>::ConstIterator i = info.theData.begin(); i != info.theData.end(); i++)
		theData[i.key()] = i.data();
}

void Properties::set(const Properties &pairs)
{
	for (QMap<QString, QVariant>::ConstIterator i = pairs.theData.begin(); i != pairs.theData.end(); i++)
		theData[i.key()] = i.data();
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

const QString PropertiesInfo::description(const QString &key) const
{
	if (!theInfo.contains(key))
		return "";
	return theInfo[key].description;
}

}
