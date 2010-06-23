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

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "buffer.h"
#include "xlconnection.h"
#else
#include <geddei/buffer.h>
#include <geddei/xlconnection.h>
#endif
using namespace Geddei;

namespace Geddei
{

class LMConnection;
class BufferReader;
class Processor;

/** @internal @ingroup Geddei
 * @brief Embodiment of Connection that delivers data from an LMConnection to a local Sink.
 * @author Gav Wood <gav@kde.org>
 *
 * A refinement of a flow control class for outbound communication.
 * This class implements a single-in, multiple-out connection paradigm.
 */
class DLLEXPORT MLConnection: public xLConnection
{
	//* Reimplementations from Connection
	virtual Type const& type() const;

	//* Reimplementations from xLConnection
	virtual void reset();
	virtual void sinkStopping();
	virtual void sinkStopped();
	virtual uint elementsReady() const;
	virtual void waitForElements(uint elements) const;
	virtual const BufferData readElements(uint elements);
	virtual const BufferData peekElements(uint elements);
	virtual void enforceMinimumRead(uint elements);
	virtual void enforceMinimumWrite(uint elements);
	virtual BufferReader *newReader();
	virtual void killReader();
	virtual void resurectReader();
	virtual bool plungeSync(uint samples) const;
	virtual uint capacity() const;
	virtual bool require(uint samples, uint preferSamples = Undefined);

protected:
	LMConnection *theConnection;
	BufferReader *theReader;
	mutable uint64_t m_samplesRead;
	mutable uint64_t m_latestPeeked;
	mutable double m_latestTime;

public:
	/**
	 * Blocks until sink is happy that all inputs are confirmed.
	 */
	bool waitUntilReady();
	Tristate isReadyYet();

	/**
	 * Resets the (essentially cached) type of the connection.
	 * Should be propogated between start & stop called
	 */
	void resetType();

	/**
	 * Sets the (wouldbe cached) type of the connection.
	 */
	void setType(Type const& _type);

	/**
	 * Notifies the Sink that a fresh stream of plungers may be coming from the
	 * source.
	 */
	void startPlungers();

	/**
	 * Half the dispatchPlunger() operation. This one doesn't actually append
	 * the plunger to the Buffer, it simply notifies the Sink that a plunger
	 * has been appended.
	 */
	void plungerSent();

	/**
	 * Notifies the Sink that no more plungers will be on their way from this
	 * source.
	 */
	void noMorePlungers();

	virtual double secondsPassed() const { return type().isA<Contiguous>() ? m_latestPeeked / (double)(type().asA<Contiguous>().frequency()) : m_latestTime; }
	virtual double secondsPassed(float _s) const { return type().isA<Contiguous>() ? (m_latestPeeked - theReader->lastReadSize() + _s) / (double)(type().asA<Contiguous>().frequency()) : m_latestTime; }

	MLConnection(Sink *sink, uint sinkIndex, LMConnection *connection);
	virtual ~MLConnection();
};

}
