/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_MLCONNECTION_H
#define _GEDDEI_MLCONNECTION_H

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
	virtual const SignalTypeRef type();

	//* Reimplementations from xLConnection
	virtual void reset();
	virtual void sinkStopping();
	virtual void sinkStopped();
	virtual uint elementsReady() const;
	virtual void waitForElements(uint elements) const;
	virtual const BufferData readElements(uint elements);
	virtual const BufferData peekElements(uint elements);
	virtual void enforceMinimum(uint elements);
	virtual BufferReader *newReader();
	virtual void killReader();
	virtual void resurectReader();
	virtual bool plungeSync(uint samples) const;
	virtual uint capacity() const;

protected:
	LMConnection *theConnection;
	BufferReader *theReader;

public:
	/**
	 * Blocks until sink is happy that all inputs are confirmed.
	 */
	bool waitUntilReady();

	/**
	 * Resets the (essentially cached) type of the connection.
	 * Should be propogated between start & stop called
	 */
	void resetType();

	/**
	 * Sets the (wouldbe cached) type of the connection.
	 */
	void setType(const SignalType *type);
	
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
	
	MLConnection(Sink *sink, uint sinkIndex, LMConnection *connection);
	virtual ~MLConnection();
};

};

#endif
