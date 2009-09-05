/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@kde.org                                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _QT_FACTORY_H
#define _QT_FACTORY_H

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
	load();
	QStringList provIds;
	theIds.clear();
	if (MESSAGES) qDebug("Retrieving available classes derived from %s...", typeid(Base).name());
	if (resolve("getAvailable"))
		provIds = ((const QStringList &(*)(const QString &))(resolve("getAvailable")))(typeid(Base).name());
	if (MESSAGES) qDebug("Found %d candidates.", provIds.count());
	for (QStringList::const_iterator i = provIds.begin(); i != provIds.end(); i++)
	{	if (MESSAGES) qDebug("Loading class %s (derived from %s)...", (*i).latin1(), typeid(Base).name());
		Base *(*creator)();
		int version = -1;
		creator = (Base *(*)())(resolve("create" + *i));
		if (resolve("version" + *i))
			version = ((int (*)())(resolve("version" + *i)))();
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

#endif
