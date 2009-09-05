/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_SUBPROCESSORFACTORY_H
#define _GEDDEI_SUBPROCESSORFACTORY_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "subprocessor.h"
#include "qfactorymanager.h"
#else
#include <geddei/subprocessor.h>
#include <qtextra/qfactorymanager.h>
#endif
using namespace Geddei;

namespace Geddei
{

class DomProcessor;

/** @ingroup Geddei
 * @brief Singleton class for creating SubProcessor objects from plugins.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * The SubProcessorFactory singleton exists in Geddei as the interface to the
 * plugin system. It provides a very simple mechanism for the creation of
 * SubProcessor-derived classes that are non-existant, unknown or otherwise
 * unavailable at compile time.
 *
 * The fact that it embodies a singleton is hidden behind the API. The only
 * methods available to the developer are static.
 *
 * Typical usage requires a program to first check if a given class is
 * available with available(), then perhaps to check the version provided with
 * version (). Having ascertained that the correct class is available a
 * DomProcessor object whose primary SubProcessor is an object of the given
 * type may be instantiated with createDom().
 */
class DLLEXPORT SubProcessorFactory
{
	static QFactoryManager<SubProcessor> *theOne;
	static QFactoryManager<SubProcessor> &factory();

public:
	/**
	 * Queries availablity of a SubProcessor type.
	 *
	 * @param type The type of SubProcessor (i.e. class name) to be queried.
	 * @return true if it can be create() -ed, false otherwise.
	 */
	static bool available(const QString &type);

	/**
	 * Gets (latest available) version of a SubProcessor -derived class.
	 *
	 * @param type The type of SubProcessor (i.e. class name) to be queried.
	 * @return The version of the class @a type that is available, or -1 if
	 * unavailable, given in the integer code representation.
	 */
	static int versionId(const QString &type);

	/**
	 * Gets (latest available) version of a SubProcessor -derived class.
	 *
	 * @param type The type of SubProcessor (i.e. class name) to be queried.
	 * @return The version of the class @a type that is available, or -1 if
	 * unavailable, given in a proper string representation.
	 */
	static QString version(const QString &type) { return QString::number(versionId(type) / 65536) + "." + QString::number((versionId(type) / 256) % 256) + "." + QString::number(versionId(type) % 256); }

	/**
	 * Retrieves a list of the types of SubProcessor available for creation.
	 *
	 * @return A complete list of names of SubProcessor-derived classes that
	 * will cause available to return true when each is used as the parameter.
	 */
	static const QStringList available() { return factory().getAvailable(); }

	/**
	 * Creates a SubProcessor object of type @a type.
	 *
	 * @return A newly instantiated object of class @a type , or 0 if @a type
	 * is not available.
	 */
	static SubProcessor *create(const QString &type);

	/**
	 * Creates a DomProcessor object, whose primary is a SubProcessor of type
	 * @a type.
	 *
	 * @return A newly instantiated DomProcessor object, or 0 if @a type is not
	 * available.
	 */
	static DomProcessor *createDom(const QString &type);
};

};

#endif
