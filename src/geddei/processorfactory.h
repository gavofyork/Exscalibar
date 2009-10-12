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

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "qfactorymanager.h"
#include "processor.h"
#else
#include <qtextra/qfactorymanager.h>
#include <geddei/processor.h>
#endif
using namespace Geddei;

namespace Geddei
{

/** @ingroup Geddei
 * @brief Singleton class for creating Processor objects from plugins.
 * @author Gav Wood <gav@kde.org>
 *
 * The ProcessorFactory singleton exists in Geddei as the interface to the
 * plugin system. It provides a very simple mechanism for the creation of
 * Processor-derived classes that are non-existant, unknown or otherwise
 * unavailable at compile time.
 *
 * The fact that it embodies a singleton is hidden behind the API. The only
 * methods available to the developer are static.
 *
 * Typical usage requires a program to first check if a given class is
 * available with available(), then perhaps to check the version provided with
 * version (). Having ascertained that the correct class is available an
 * object may be instantiated with create().
 */
class DLLEXPORT ProcessorFactory
{
	static QFactoryManager<Processor> *theOne;
	static QFactoryManager<Processor> &factory();

public:
	/**
	 * Queries availablity of a Processor type.
	 *
	 * @param type The type of Processor (i.e. class name) to be queried.
	 * @return true if it can be create() -ed, false otherwise.
	 */
	static bool available(const QString &type) { return factory().isAvailable(type); }

	/**
	 * Gets (latest available) version of a Processor -derived class.
	 *
	 * @param type The type of Processor (i.e. class name) to be queried.
	 * @return The version of the class @a type that is available, or -1 if
	 * unavailable, in an integer code.
	 */
	static int versionId(const QString &type) { return factory().getVersion(type); }

	/**
	 * Gets (latest available) version of a Processor -derived class.
	 *
	 * @param type The type of Processor (i.e. class name) to be queried.
	 * @return The version of the class @a type that is available, or -1 if
	 * unavailable, in a proper string representation.
	 */
	static QString version(const QString &type) { return QString::number(versionId(type) / 65536) + "." + QString::number((versionId(type) / 256) % 256) + "." + QString::number(versionId(type) % 256); }

	/**
	 * Retrieves a list of the types of Processor available for creation.
	 *
	 * @return A complete list of names of Processor-derived classes that will
	 * cause available to return true when each is used as the parameter.
	 */
	static const QStringList available() { return factory().getAvailable(); }

	/**
	 * Creates a Processor object of type @a type.
	 *
	 * @return A newly instantiated object of class @a type , or 0 if @a type
	 * is not available.
	 */
	static Processor *create(const QString &type);
};

}
