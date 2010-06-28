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
#include "bufferreader.h"
#include "xlconnection.h"
#include "contiguous.h"
#else
#include <geddei/buffer.h>
#include <geddei/bufferreader.h>
#include <geddei/xlconnection.h>
#include <geddei/contiguous.h>
#endif
using namespace Geddei;

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
	// Reimplementations from xLConnection
	friend class RLConnection;

protected:
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

	virtual const Type& type() const;

	/**
	 * Extracts the type from the source end of the connection.
	 */
	virtual bool pullType() = 0;

	friend class BobPort;
	Buffer theBuffer;
	BufferReader* theReader;
	mutable uint64_t m_samplesRead;
	mutable uint64_t m_latestPeeked;
	mutable double m_latestTime;
	int m_minRead;
	int m_minWrite;

private:
	virtual void reset() { m_samplesRead = 0; m_latestPeeked = 0; m_latestTime = 0.0; theBuffer.clear(); m_minRead = m_minWrite = 0; }
	virtual void sinkStopping();
	virtual void sinkStopped();
	virtual uint elementsReady() const;
	virtual void waitForElements(uint elements) const;
	virtual const BufferData readElements(uint elements);
	virtual const BufferData peekElements(uint elements);
	virtual void enforceMinimum(uint _elements);
	virtual void enforceMinimumRead(uint _elements);
	virtual void enforceMinimumWrite(uint _elements);
	virtual BufferReader *newReader() { return new BufferReader(&theBuffer); }
	virtual void killReader();
	virtual void resurectReader();
	virtual uint capacity() const { return theBuffer.size() / theType->size(); }
	virtual float filled() const { return 1.0 - float(theBuffer.elementsFree()) / float(theBuffer.size()); }
	virtual bool plungeSync(uint samples) const;
	virtual bool require(uint samples, uint preferSamples = Undefined);
	virtual double secondsPassed() const { return type().isA<Contiguous>() ? m_latestPeeked / (double)(type().asA<Contiguous>().frequency()) : m_latestTime; }
	virtual double secondsPassed(float _s) const { return type().isA<Contiguous>() ? (m_latestPeeked - theReader->lastReadSize() + _s) / (double)(type().asA<Contiguous>().frequency()) : m_latestTime; }
};

}
