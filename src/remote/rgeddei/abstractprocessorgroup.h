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

#include <qstring.h>
#include <qmap.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "processor.h"
#include "abstractprocessor.h"
#else
#include <geddei/processor.h>
#include <geddei/abstractprocessor.h>
#endif
using namespace Geddei;

namespace rGeddei
{

class AbstractDomProcessor;

/** @ingroup rGeddei
 * @brief A class to segment a number of AbstractProcessor objects into a set.
 * @author Gav Wood <gav@kde.org>
 *
 * Similar in essence to Geddei's ProcessorGroup class, this class allows
 * several Processor objects to be controlled simultaneously. They may be
 * easily started, stopped and deleted. This helps enormously with managing
 * your AbstractProcessors.
 *
 * You may also treat it like an array using the array subscript operator ([])
 * to retrieve a particular AbstractProcessor object reference through its
 * name.
 */
class DLLEXPORT AbstractProcessorGroup
{
	QMap<QString, AbstractProcessor *> theProcessors;

	friend class AbstractProcessor;
	void add(AbstractProcessor *o);
	void remove(AbstractProcessor *o);

public:
	/**
	 * Call AbstractProcessor::init() on all AbstractProcessor objects in the
	 * group with only the default arguments.
	 *
	 * All properties will be default, and all names unique but unknown.
	 *
	 * @note This is rarely used, since the usual workflow involves setting an
	 * AbstractProcessor object's group with the call to
	 * AbstractProcessor::init(). However in some situations this call may still
	 * be useful.
	 */
	void init() const;

	/**
	 * Start all represented Processor objects processing. See
	 * Geddei::Processor::go() and AbstractProcessor::go() for more
	 * information.
	 *
	 * @param waitUntilGoing If true, this call should block until all involved
	 * Processor objects are either running or have failed to start.
	 * @return true if the operation succeeded.
	 */
	bool go(bool waitUntilGoing = true) const;

	/**
	 * Block until all represented Processor objects are started or have failed
	 * to start.
	 *
	 * @param errorProc If non-zero, specifies a location at which a pointer to
	 * the offending AbstractProcessor object, if existant, will be stored.
	 * @param errorData If non-zero, specifies a location at which an integer
	 * coded description of the error, if existant, will be stored.
	 * @return The type of error that occured (if existant), coded as a
	 * Geddei::Processor::ErrorType enumeration.
	 */
	Processor::ErrorType waitUntilGoing(AbstractProcessor **errorProc = 0, int *errorData = 0) const;

	/**
	 * Stop all represented Processor objects immediately. See
	 * Geddei::Processor::stop() and AbstractProcessor::stop() for more
	 * information.
	 *
	 * @param resetToo Conduct a reset of all processors as well. Leave as true
	 * unless you know what you're doing.
	 */
	void stop(bool resetToo = true) const;

	/**
	 * Resets all Processor objects in the group. They must already be
	 * stop()ed. See Geddei::Processor::reset() and AbstractProcessor::reset()
	 * for more information.
	 *
	 * @sa stop()
	 */
	void reset() const;

	/**
	 * Disconnect all represented Processor objects immediately. See
	 * Geddei::Processor::disconnectAll() for more information.
	 */
	void disconnectAll();

	/**
	 * Delete all AbstractProcessor objects in this group.
	 */
	void deleteAll();

	/**
	 * Check whether an AbstractProcessor object of name @a name exists in this
	 * group.
	 *
	 * @param name The name to be checked for.
	 * @return true iff the object of name @a name exists in this group.
	 */
	bool exists(const QString &name);

	/**
	 * Retreive the AbstractProcessor object of name @a name.
	 *
	 * @note Do not call this method if no AbstractProcessor of name @a name
	 * exists in this group.
	 *
	 * @param name The name of the AbstractProcessor to be retrieved.
	 * @return Reference to the AbstractProcessor of name @a name.
	 *
	 * @sa operator[]()
	 */
	AbstractProcessor &get(const QString &name);

	/**
	 * Retreive the AbstractDomProcessor object of name @a name.
	 *
	 * @note Do not call this method if no AbstractDomProcessor of name
	 * @a name exists in this group.
	 *
	 * @param name The name of the AbstractDomProcessor to be retrieved.
	 * @return Reference to the AbstractDomProcessor of name @a name.
	 *
	 * @sa operator[]() @sa get()
	 */
	AbstractDomProcessor &dom(const QString &name);

	/**
	 * Array subscript operator. Semantically equivalent to get().
	 *
	 * @param name The name of the AbstractProcessor to be retrieved.
	 * @return Reference to the AbstractProcessor of name @a name.
	 *
	 * @sa get()
	 */
	AbstractProcessor &operator[](const QString &name) { return get(name); }

	/**
	 * Basic constructor.
	 */
	AbstractProcessorGroup();

	/**
	 * Default destructor.
	 */
	~AbstractProcessorGroup();
};

}
