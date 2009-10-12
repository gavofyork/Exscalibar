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

#include <QMap>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDebug>

#include <exscalibar.h>

#include "qfactory.h"

#define MESSAGES 0

/** @ingroup QtExtra
 * @brief Class for creating arbitrary subclasses from plugins at runtime.
 * @author Gav Wood <gav@kde.org>
 *
 * Not threadsafe.
 */
template<class Base>
class DLLEXPORT QFactoryManager
{
	QList<QFactory<Base>*> theFactories;
	QMap<QString, QFactory<Base> * > theMappings;
	QStringList theIds;

	void loadLibrary(const QString &theFile);
	void loadLibraries(const QString &thePath);

	void clear()
	{
//		for (QMapIterator<QString, QFactory<Base> *> i = theFactories.begin(); i != theFactories.end(); i++)
//			delete i.data();
		while (theFactories.size())
			delete theFactories.takeLast();
		theIds.clear();
		theMappings.clear();
	}
public:
	void reloadAll(const QString &thePath)
	{
		clear();
		loadLibraries(thePath);
	}

	void reloadAll(QStringList const& thePaths)
	{
		clear();
		foreach (QString i, thePaths)
			loadLibraries(i);
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

	~QFactoryManager() { clear(); }
};

template<class Base>
void QFactoryManager<Base>::loadLibrary(const QString &theFile)
{
	if (MESSAGES) qDebug("Loading library %s...", qPrintable(theFile));
	QFactory<Base> *newFactory = new QFactory<Base>(theFile);
	if (!newFactory->isOpen()) { delete newFactory; return; }

	bool used = false;
	const QStringList &ids = newFactory->getAvailable();
	for (QStringList::const_iterator i = ids.begin(); i != ids.end(); i++)
	{
		if (MESSAGES) qDebug("Found processor %s...", qPrintable((*i)));
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
	if (MESSAGES) qDebug("Scanning path: %s...", qPrintable(thePath));
	QDir d(thePath);
	d.setFilter(QDir::Readable | QDir::Executable | QDir::Files | QDir::NoSymLinks);
	QStringList l = d.entryList();
	for (uint i = 0; i < (uint)l.count(); i++)
	{	if (MESSAGES) qDebug("Loading library %s...", qPrintable(l[i]));
		loadLibrary(thePath + "/" + l[i]);
	}
}

#undef MESSAGES
