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

#include "buffer.h"
#include "bufferreader.h"
#include "xlconnection.h"
#else
#include <geddei/buffer.h>
#include <geddei/bufferreader.h>
#include <geddei/xlconnection.h>
#endif
using namespace Geddei;

class BobPort;

namespace Geddei
{

class Processor;

/** @internal @ingroup Geddei
 * @brief Refinement of a Connection to a local Sink object, that transfers data.
 * @author Gav Wood <gav@kde.org>
 *
 * Data delivery mechanism implemented, though pullType() left as a hook to the
 * type delivery system and theBuffer left open for allowing actual data to be
 * injected.
 */
class xLConnectionReal: public xLConnection
{
	// Reimplementations from Connection
	friend class Processor;
public:
	virtual const SignalTypeRef type() const;

	// Reimplementations from xLConnection
	friend class RLConnection;
private:
	virtual void reset() { m_samplesRead = 0; m_latestPeeked = 0; theBuffer.clear(); }
	virtual void sinkStopping();
	virtual void sinkStopped();
	virtual uint elementsReady() const;
	virtual void waitForElements(uint elements) const;
	virtual const BufferData readElements(uint elements);
	virtual const BufferData peekElements(uint elements);
	virtual void enforceMinimum(uint elements);
	virtual BufferReader *newReader() { return new BufferReader(&theBuffer); }
	virtual void killReader();
	virtual void resurectReader();
	virtual uint capacity() const { return theBuffer.size() / theType->size(); }
	virtual float filled() const { return 1.0 - float(theBuffer.elementsFree()) / float(theBuffer.size()); }
	virtual bool plungeSync(uint samples) const;
	virtual bool require(uint samples, uint preferSamples = Undefined);
	virtual double secondsPassed() const { return type().isA<Signal>() ? m_latestPeeked / (double)(type().asA<Signal>().frequency()) : 0.0; }

protected:
	friend class BobPort;
	Buffer theBuffer;
	BufferReader *theReader;
	mutable uint64_t m_samplesRead;
	mutable uint64_t m_latestPeeked;


	/**
	 * Extracts the type from the source end of the connection.
	 */
	virtual bool pullType() = 0;

	/**
	 * Simple constructor.
	 *
	 * @param newSink The Sink-derived object to which this Connection shall be
	 * tied.
	 * @param newSinkIndex The input index of the Sink-derived object to which
	 * this Connection shall be tied.
	 * @param bufferSize The size of the buffer for this Connection.
	 */
	xLConnectionReal(Sink *newSink, uint newSinkIndex, uint bufferSize);

	/**
	 * Simple destructor.
	 */
	~xLConnectionReal();
};

}
