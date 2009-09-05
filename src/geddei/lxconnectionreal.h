/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_LXCONNECTIONREAL_H
#define _GEDDEI_LXCONNECTIONREAL_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "lxconnection.h"
#else
#include <geddei/lxconnection.h>
#endif
using namespace Geddei;

namespace Geddei
{

class Source;

/** @internal @ingroup Geddei
 * @brief Refinement of Connection that transfers data from a local Source.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * A refinement of a flow control class for outbound communication.
 * This class implements "half" of a LxConnection class. Where the data goes is
 * unspecified; several virtual methods are provided to hook onto the data transfer
 * mechanism.
 */
class LxConnectionReal: public LxConnection
{
	uint theLastElementsFree;

	//* Reimplementation from LxConnection
	virtual void push(const BufferData &data);

protected:
	/**
	 * The utility methods - these should be transparent regardless of where the sink
	 * processor is located (i.e. whether it's an LL or an LR connection).
	 * These are used internally by Processor, hence the friend.
	 */
	friend class Processor;

	/**
	 * Blocks until destination buffer has free space.
	 */
	virtual void bufferWaitForFree() = 0;

	/**
	 * Returns the amount of free ELEMENTS in the destination buffer (trivial on LL but
	 * harder for LR). Useful for gauging amount to push.
	 */
	virtual uint bufferElementsFree() = 0;

	/**
	 * Actually moves the data to the receiving "place" (either a connection or an object).
	 */
	virtual void transport(const BufferData &data) = 0;

	/**
	 * Simple constructor.
	 */
	LxConnectionReal(Source *source, uint sourceIndex) : LxConnection(source, sourceIndex), theLastElementsFree(0) {}

	/**
	 * Simple virtual destructor.
	 */
//	~LxConnectionReal() {}
};

};

#endif
