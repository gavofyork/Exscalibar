/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

#include <QString>
#include <QMap>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "groupable.h"
#else
#include <geddei/groupable.h>
#endif

namespace Geddei
{

class Processor;
class DomProcessor;
class MultiProcessor;

/** @ingroup Geddei
 * @brief A class to segment a number of Processor objects into a set.
 * @author Gav Wood <gav@kde.org>
 *
 * This class allows several Processor objects to be controlled simultaneously.
 * They may be easily started, stopped and deleted. This helps enormously with
 * managing your Processors.
 *
 * You may also treat it like an array using the array subscript operator ([])
 * to retrieve a particular Processor object reference through its name.
 */
class DLLEXPORT ProcessorGroup
{
	mutable Groupable* m_errorProc;
	bool theAdopt;
	QMap<QString, Groupable*> theMembers;

	friend class Groupable;
	void add(Groupable *o);
	void remove(Groupable *o);

public:
	/**
	 * Confirm the signal types of all connections of all Processor objects in
	 * this set.
	 *
	 * @return true iff all types confirm correctly.
	 */
	bool confirmTypes() const;
	Groupable* errorProc() const { return m_errorProc; }

	/**
	 * Start all Processor objects in the group. Note this returns once all have
	 * been "primed". If you want to wait until they are actually going (or not
	 * due to an error), use waitUntilGoing() as well, or specify true to
	 * @a waitUntilGoing.
	 *
	 * @param waitUntilGoing If true, blocks until all Processor objects are
	 * started correctly or have otherwise failed.
	 * @return true if all Processor objects could be initially started
	 * correctly.
	 *
	 * @sa waitUntilGoing()
	 */
	bool go(bool waitUntilGoing = true) const;

	/**
	 * Waits for all Processor objects to be actaully running correctly. Use this
	 * only after a go() call that has returned true.
	 *
	 * @param errorProc If supplied (and non-zero), populate with a pointer to the
	 * first Processor object that returned an error.
	 * @param errorData If supplied (and non-zero), populate with any numeric data
	 * the error arose with.
	 * @return The Error it failed with or Processor::NoError if all went correctly.
	 */
	Groupable::ErrorType waitUntilGoing(Groupable **errorProc = 0, int *errorData = 0) const;

	/**
	 * Returns a QString describing each error that occured. Should only be called
	 * after either go() or waitUntilGoing();
	 *
	 * @return Error in pretty form.
	 */
	const QString error() const;

	/**
	 * Stops all Processor objects in the group.
	 *
	 * @param resetToo If true, reset() all the Processor objects after stop()ing
	 * them. If unspecified, defaults to true.
	 *
	 * @sa reset()
	 */
	void stop(bool resetToo = true) const;

	/**
	 * Resets all Processor objects in the group. They must already be stop()ed.
	 *
	 * @sa stop()
	 */
	void reset() const;

	/**
	 * Deletes all Processor objects in the group.
	 */
	void deleteAll();

	/**
	 * Disconnects all connections that are outgoing from any Processor objects
	 * in the group.
	 *
	 * @note This means that if all existing connections are between Processor
	 * objects in this group, then all existing connections will be destroyed.
	 */
	void disconnectAll();

	/**
	 * Checks for the existance of a named Processor in the group.
	 *
	 * @param name The name of the Processor object to be found.
	 * @return true if a Processor object of name @a name is contained in the group.
	 */
	bool exists(const QString &name);

	/**
	 * Get a reference to a named Processor object. It must exist in the group.
	 *
	 * @param name The name of the Processor object to be found.
	 * @return A reference to the Processor object of name @a name.
	 */
	Processor &get(const QString &name);

	/**
	 * Get a reference to a named DomProcessor object. It must exist in the
	 * group.
	 *
	 * @param name The name of the DomProcessor object to be found.
	 * @return A reference to the DomProcessor object of name @a name.
	 */
	DomProcessor &dom(const QString &name);

	/**
	 * Get a reference to a named MultiProcessor object. It must exist in the
	 * group.
	 *
	 * @param name The name of the MultiProcessor object to be found.
	 * @return A reference to the MultiProcessor object of name @a name.
	 */
	MultiProcessor &multi(const QString &name);

	/**
	 * Get a reference to a named Groupable object. It must exist in the group.
	 * This is a convenience method that does the same as get().
	 *
	 * @param name The name of the Groupable object to be found.
	 * @return A reference to the Groupable object of name @a name.
	 */
	Groupable &operator[](const QString &name) { return *theMembers[name]; }

	/**
	 * Get the number of Processor objects in group.
	 *
	 * @return The number of Processor objects in the group.
	 */
	uint count() const { return theMembers.count(); }

	/**
	 * Default constructor.
	 *
	 * @param adopt If true, this will adopt all the Processor objects and thus
	 * delete any left over on destruction.
	 */
	ProcessorGroup(bool adopt = false);

	/**
	 * Default destructor.
	 */
	~ProcessorGroup();
};

}
