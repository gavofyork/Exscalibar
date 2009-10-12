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

#include "multiplicative.h"
#include "processorport.h"
#else
#include <geddei/multiplicative.h>
#include <geddei/processorport.h>
#endif
using namespace Geddei;

namespace Geddei
{

class MultiSink;

/** @internal @ingroup Geddei
 * @brief Essentially abstract class to describe a multiplicative source object.
 * @author Gav Wood <gav@kde.org>
 *
 * Any class derived from this should provide a number of outputs that are the same
 * signal type (this is relaxed slightly in Processor, where it only adheres to this
 * in certain operating modes). Because of this assumption it can abstract away the
 * difference between Processors with multiple outputs of the same type and sets of
 * Processors each that take one input of a certain type (MultiProcessors).
 */
class DLLEXPORT MultiSource: virtual public Multiplicative
{
	//* Reimplementations from Multiplicative
	virtual void setSourceMultiplicity(uint multiplicity);

	//@{
	/** Deferred connect data. */
	bool theDeferredConnect, theConnected;
	uint theDeferredBufferSize;
	MultiSink *theDeferredSink;
	//@}

protected:
	/**
	 * Override to provide the routine for providing a source port.
	 *
	 * @param i The source port to be returned.
	 * @return A ProcessorPort reference to the given source (output) port.
	 */
	virtual ProcessorPort sourcePort(uint i) = 0;

	/**
	 * Override to provide a check to make sure a connection is possible.
	 * For the Processor class to check against its flags.
	 */
	virtual void connectCheck() const {}

public:
	/**
	 * Test to see if a connect() would defer.
	 *
	 * @param sink Pointer to the sink to which it is to be connected.
	 * @param bufferSize A minimum size of the buffer to be employed.
	 * @return true if the connect should be abandoned. false if both
	 * multiplicative source and sink are ready.
	 */
	bool deferConnect(MultiSink *sink, uint bufferSize);

	/**
	 * Connect this to a multiplicative sink.
	 *
	 * @param sink Pointer to the sink to which it is to be connected.
	 * @param bufferSize A minimum size of the buffer to be employed.
	 */
	void connect(MultiSink *sink, uint bufferSize = 1);

	/**
	 * Disconnect all outputs from the multiplicitive sink. This must already
	 * be connected.
	 */
	void disconnect();

	/**
	 * Connect this to a multiplicative sink. This is a convenience method
	 * for connect().
	 *
	 * @param sink Pointer to the sink to which it is to be connected.
	 *
	 * @sa connect()
	 */
	void operator>>=(MultiSink &sink) { connect(&sink); }

	/**
	 * Disconnect all outputs from the multiplicitive sink. This must already
	 * be connected. This is a convenience method for disconnect().
	 *
	 * @sa disconnect()
	 */
	void operator--() { disconnect(); }

	/**
	 * Basic constructor.
	 */
	MultiSource() : theDeferredConnect(false), theConnected(false) {}
};

}
