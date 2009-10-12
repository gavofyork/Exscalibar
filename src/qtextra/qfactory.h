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

#include <typeinfo>

#include <qmap.h>
#include <qstring.h>
#include <qlibrary.h>
#include <qstringlist.h>

#include <exscalibar.h>

#define MESSAGES 0

template<class Base>
class DLLEXPORT QFactory : private QLibrary
{
	QMap<QString, Base *(*)()> theCreators;
	QMap<QString, int> theVersions;
	QStringList theIds;

public:
	/** Produces one instance of the opened plugin class. returns 0 if bad. */
	Base *createInstance(const QString &name) { if (isOpen()) if (theIds.contains(name)) return theCreators[name](); return 0; }

	/** Returns a class's version. */
	int getVersion(const QString &name) { if (isOpen()) if (theIds.contains(name)) return theVersions[name]; return -1; }

	/** Returns the list of available class names. */
	const QStringList &getAvailable() { return theIds; }

	/** Returns false if failed to create factory. */
	bool isOpen() { return isLoaded(); }

	/** Opens a factory ready for production. */
	QFactory(const QString &libPath);
};

template<class Base>
QFactory<Base>::QFactory(const QString &libPath) : QLibrary(libPath)
{
	theIds.clear();
	if (!QLibrary::isLibrary(libPath))
	{
		if (MESSAGES) qDebug("%s: Not a library.", qPrintable(libPath));
		return;
	}
	load();
	if (!isLoaded())
	{
		if (MESSAGES) qDebug("%s: Could not load: %s", qPrintable(libPath), qPrintable(errorString()));
		return;
	}
	QStringList provIds;
	if (MESSAGES) qDebug("Retrieving available classes derived from %s...", typeid(Base).name());
	if (resolve("getAvailable"))
		provIds = ((const QStringList &(*)(const QString &))(resolve("getAvailable")))(typeid(Base).name());
	else
	{
		if (MESSAGES) qDebug("%s: Not a Processor plugin.", qPrintable(libPath));
		return;
	}
	if (MESSAGES) qDebug("Found %d candidates.", provIds.count());
	for (QStringList::const_iterator i = provIds.begin(); i != provIds.end(); i++)
	{	if (MESSAGES) qDebug("Loading class %s (derived from %s)...", qPrintable((*i)), typeid(Base).name());
		Base *(*creator)();
		int version = -1;
		creator = (Base *(*)())(resolve(("create" + *i).toLatin1()));
		if (resolve(("version" + *i).toLatin1()))
			version = ((int (*)())(resolve(("version" + *i).toLatin1())))();
		if (creator && version > -1)
		{	theIds += *i;
			theCreators[*i] = creator;
			theVersions[*i] = version;
			if (MESSAGES) qDebug("Loaded OK (Version: %d)", version);
		}
	}
	if (!theIds.size())
		unload();
}

#undef MESSAGES
