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

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "connection.h"
#else
#include <geddei/connection.h>
#endif
using namespace Geddei;

namespace Geddei
{

class xLConnection;
class Processor;

/** @internal @ingroup Geddei
 * @brief Abstract class defining operations on an object that consumes data from a Connection.
 * @author Gav Wood <gav@kde.org>
 *
 */
class DLLEXPORT Sink
{
public:
	/**
	 * Make a record that @a me is now inhabiting @a port of this Source.
	 * If the source produces only one ubiquitous output stream, use the
	 * default port of 0.
	 */
	virtual void doRegisterIn(xLConnection *me, uint port) = 0;

	/**
	 * Asserts that @a me has been previously registered at @a port on
	 * this source and deregisters it.
	 */
	virtual void undoRegisterIn(xLConnection *me, uint port) = 0;

	/**
	 * Checks that the input port @a sinkIndex is valid for a new connection.
	 * This is just used to check before a doRegisterIn call.
	 */
	virtual bool readyRegisterIn(uint sinkIndex) const = 0;

	/**
	 * Called by an incoming signal to suggest that the sink should initialise
	 * ready for plunger notification.
	 *
	 * This is necessary since this Sink may be started after an input
	 * Processor has already attempted notification of a plunger.
	 */
	virtual void startPlungers() = 0;

	/**
	 * Called by an incoming signal to affirm that no more plungers will
	 * follow in an input stream is received.
	 */
	virtual void noMorePlungers() = 0;

	/**
	 * Called when an incoming signal to affirm that another plunger will
	 * be available sometime in the future.
	 */
	virtual void plungerSent(uint index) = 0;

	/**
	 * Callback for when a plunger is in the input.
	 */
	virtual void plunged(uint index) = 0;

	/**
	 * Reimplement to allow exit status checking around i/o (and also in the main
	 * processing loop.
	 */
	virtual void checkExit() = 0;

	/**
	 * This should return only when the Sink has started processing or bailed
	 * due to error.
	 */
	virtual bool waitUntilReady() = 0;
	virtual Connection::Tristate isGoingYet() = 0;

	/**
	 * This should return only when the Sink has finished all processing.
	 */
	virtual void waitToStop() = 0;

	/**
	 * Checks the types (recursively if neccessary) and reports if all is ok. Sets
	 * up Connections types on its way.
	 * Note this is done automatically if neccessary at go(), but it's a good practice
	 * to call it beforehand anyway.
	 */
	virtual bool confirmTypes() = 0;

	/**
	 * May be called upon between stop() and start() to reset any cached data stored
	 * on connections.
	 */
	virtual void resetTypes() = 0;

	/**
	 * Virtual destructor.
	 */
	virtual ~Sink() {}
};

}
