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
 * @author Gav Wood <gav@kde.org>
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

}
