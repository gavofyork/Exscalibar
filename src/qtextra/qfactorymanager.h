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

#ifndef _QT_FACTORYMANAGER_H
#define _QT_FACTORYMANAGER_H

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3PtrList>

#include <exscalibar.h>

#include "qfactory.h"

#define MESSAGES 0

/** @ingroup QtExtra
 * @brief Class for creating arbitrary subclasses from plugins at runtime.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * Not threadsafe.
 */
template<class Base>
class DLLEXPORT QFactoryManager
{
	Q3PtrList<QFactory<Base> > theFactories;
	QMap<QString, QFactory<Base> * > theMappings;
	QStringList theIds;

	void loadLibrary(const QString &theFile);
	void loadLibraries(const QString &thePath);

	void clear()
	{
//		for (QMapIterator<QString, QFactory<Base> *> i = theFactories.begin(); i != theFactories.end(); i++)
//			delete i.data();
		theFactories.clear();
		theIds.clear();
		theMappings.clear();
	}
public:
	void reloadAll(const QString &thePath)
	{
		clear();
		loadLibraries(thePath);
	}

	void reloadAll(const QStringList &thePaths)
	{
		clear();
		for (uint i = 0; i < (uint)thePaths.size(); i++)
			loadLibraries(thePaths[i]);
	}

	const QStringList &getAvailable() { return theIds; }
	bool isAvailable(const QString &id) { return theIds.contains(id); }

	int getVersion(const QString &id)
	{
		if (!isAvailable(id)) return -1;
		return theMappings[id]->getVersion(id);
	}
	Base *createInstance(const QString &id)
	{
		if (!isAvailable(id)) return 0;
		return theMappings[id]->createInstance(id);
	}
	Base *operator[](const QString &id) { return createInstance(id); }

	QFactoryManager() { theFactories.setAutoDelete(true); }
};

template<class Base>
void QFactoryManager<Base>::loadLibrary(const QString &theFile)
{
	if (MESSAGES) qDebug("Loading library %s...", theFile.latin1());
	QFactory<Base> *newFactory = new QFactory<Base>(theFile);
	if (!newFactory->isOpen()) { delete newFactory; return; }

	bool used = false;
	const QStringList &ids = newFactory->getAvailable();
	for (QStringList::const_iterator i = ids.begin(); i != ids.end(); i++)
	{
		if (MESSAGES) qDebug("Found processor %s...", (*i).latin1());
		if (!theIds.contains(*i))
			theIds += *i;
		else if (newFactory->getVersion(*i) <= getVersion(*i))
			continue;
		if (MESSAGES) qDebug("Using it (new version: %d)", newFactory->getVersion(*i));
		theMappings[*i] = newFactory;
		used = true;
	}
	if (used)
		theFactories.append(newFactory);
	else
		delete newFactory;
}

template<class Base>
void QFactoryManager<Base>::loadLibraries(const QString &thePath)
{
	if (MESSAGES) qDebug("Scanning path: %s...", thePath.latin1());
	QDir d(thePath);
	d.setFilter(QDir::Readable | QDir::Executable | QDir::Files | QDir::NoSymLinks);
	QStringList l = d.entryList();
	for (uint i = 0; i < (uint)l.count(); i++)
	{	if (MESSAGES) qDebug("Loading library %s...", l[i].latin1());
		loadLibrary(thePath + "/" + l[i]);
	}
}

#undef MESSAGES

#endif
