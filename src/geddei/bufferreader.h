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
#include "bufferdata.h"
#include "transmissiontype.h"
#include "buffer.h"
#else
#include <geddei/bufferdata.h>
#include <geddei/transmissiontype.h>
#include <geddei/buffer.h>
#endif

namespace Geddei
{

class Processor;

/** @internal @ingroup Geddei
 * @brief Conspirator class for conducting read operations on Buffer objects.
 * @author Gav Wood <gav@kde.org>
 */
class BufferReader : public ScreenOwner
{
	friend class Buffer;
	Buffer *theBuffer;
	BufferInfo *lastRead;
	uint readPos, lastReadSize, theUsed, theToBeSkipped, theAlreadyPlungedHere;

	/**
	 * The guts of skipElements. Do not use directly.
	 */
	void skipElementsUNSAFE(uint elements);

	/**
	 * Just used for reseting the stuff.
	 */
	void clearUNSAFE();

public:
	void openTrapdoor(const Processor *processor);
	void closeTrapdoor(const Processor *processor);

	/**
	 * Discards the next plunger iff it is immediate to this reader.
	 *
	 * This actually simply increments theAlreadyPlungedHere, so the plunger
	 * is ignored. No effort whatsoever is made to actually delete the plunger.
	 * It is assumed that the Buffer object will do that when all Readers have
	 * moved past it.
	 *
	 * NOT TRUE:
	 * This is a dodgy proxy method. It actually acts upon the Buffer, not
	 * the reader; its effects are realised for any readers of the buffer.
	 * This should only be called once for any particular buffer.
	 *
	 * Thread-safe.
	 */
	void skipPlunger();

	/**
	 * Returns the number of elements before the next plunger is reached, or
	 * -1 if no plunger exists in the input stream.
	 *
	 * This takes into account theAlreadyPlungedHere counter.
	 */
	int nextPlunger() const;

	/**
	 * Returns the number of elements that would definately cause an immediate
	 * and complete return from readElements.
	 *
	 * Essentially, it is min(elementsUsed, nextPlunger), where nextPlunger ==
	 * elementsUsed if there is no plunger.
	 *
	 * This takes into account theAlreadyPlungedHere counter.
	 *
	 * Thread-safe.
	 */
	uint elementsReady() const;

	/**
	 * Waits until the buffer is at least full enough to allow a read of a
	 * number of elements.
	 *
	 * @note The buffer size should be at least twice as big as any size waited
	 * upon.
	 *
	 * This takes into account theAlreadyPlungedHere counter.
	 *
	 * Thread-safe. Blocking.
	 */
	void waitForElements(uint elements) const;

	/**
	 * Reads a number of elements from the buffer.
	 *
	 * May cause a block if the buffer hasn't enough data ready to be read.
	 *
	 * If autoFree is true (default) the elements are removed from the buffer.
	 * If autoFree is false, they are not removed so any subsequent reads will
	 * return them again.
	 *
	 * This takes into account theAlreadyPlungedHere counter.
	 *
	 * Thread-safe. Blocking.
	 */
	const BufferData readElements(uint elements, bool autoFree = true);

	/**
	 * Reads a number of elements from the buffer, immediately discards them.
	 * If a read is currently taking place, the elements are not immediately
	 * discarded but will be once the read has finished. If the read finishes
	 * as a forgetRead(), the action is undefined.
	 *
	 * This will block until enough elements are ready to be read. If there
	 * happens to be one or many plungers before @arg elements elements could
	 * be read under normal circumstances, then they will be skipped and
	 * silently discarded. In this circumstance, the operation may block until
	 * all pending reads are completed.
	 *
	 * Any number of skipElements -based BufferReader objects may be called on
	 * a single Buffer object, but they wont work properly until a real reader
	 * is called. i.e. all elements must be read one way or another. Skipping
	 * elements can only be used in conjunction with reading, not exclusively.
	 *
	 * This takes into account theAlreadyPlungedHere counter, and may or reset
	 * it.
	 *
	 * Thread-safe.
	 */
	void skipElements(uint elements);

	//* Reimplementation from ScreenOwner.

	/**
	 * To be called when data in the last read is finished with.
	 *
	 * Called implicitly at end of life of an autoFree = true (default) read
	 * BufferData.
	 *
	 * This may increment or reset theAlreadyPlungedHere counter.
	 *
	 * Thread-safe.
	 */
	virtual void haveRead(const BufferData &data);

	/**
	 * Invalidates the last read operation's BufferData.
	 *
	 * The buffer will act as though it was never read and a subsequent read
	 * operation will return the same data.
	 *
	 * Thread-safe.
	 */
	virtual void forgetRead(const BufferData &data);

	/**
	 * Returns the type of the buffer.
	 */
	Type const& type() const { return theBuffer->type(); }

	/**
	 * Returns the size of the buffer.
	 */
	uint size() const { return theBuffer->size(); }

	void debug();

	BufferReader(Buffer *buffer);
	virtual ~BufferReader();
};

}
