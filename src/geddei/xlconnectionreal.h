/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_XLCONNECTIONREAL_H
#define _GEDDEI_XLCONNECTIONREAL_H

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
 * @author Gav Wood <gav@cs.york.ac.uk>
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
	virtual const SignalTypeRef type();

	// Reimplementations from xLConnection
	friend class RLConnection;
private:
	virtual void reset() { theBuffer.clear(); }
	virtual void sinkStopping();
	virtual void sinkStopped();
	virtual const uint elementsReady() const;
	virtual void waitForElements(const uint elements) const;
	virtual const BufferData readElements(const uint elements);
	virtual const BufferData peekElements(const uint elements);
	virtual void enforceMinimum(const uint elements);
	virtual BufferReader *newReader() { return new BufferReader(&theBuffer); }
	virtual void killReader();
	virtual void resurectReader();
	virtual const uint capacity() const { return theBuffer.size() / theType->scope(); }
	virtual const float filled() const { return 1.0 - float(theBuffer.elementsFree()) / float(theBuffer.size()); }
	virtual const bool plungeSync(const uint samples) const;
	
protected:
	friend class BobPort;
	Buffer theBuffer;
	BufferReader *theReader;

	/**
	 * Extracts the type from the source end of the connection.
	 */
	virtual const bool pullType() = 0;

	/**
	 * Simple constructor.
	 *
	 * @param newSink The Sink-derived object to which this Connection shall be
	 * tied.
	 * @param newSinkIndex The input index of the Sink-derived object to which
	 * this Connection shall be tied.
	 * @param bufferSize The size of the buffer for this Connection.
	 */
	xLConnectionReal(Sink *newSink, const uint newSinkIndex, const uint bufferSize);

	/**
	 * Simple destructor.
	 */
	~xLConnectionReal();
};

}

#endif
