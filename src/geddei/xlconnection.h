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

#include "connection.h"
#include "bufferdata.h"
#include "bufferreader.h"
#include "signaltyperef.h"
#else
#include <geddei/connection.h>
#include <geddei/bufferdata.h>
#include <geddei/bufferreader.h>
#include <geddei/signaltyperef.h>
#endif
using namespace Geddei;

namespace Geddei
{

class Sink;

/** @ingroup Geddei
 * @brief Refinement of Connection to a local Sink.
 * @author Gav Wood <gav@kde.org>
 *
 * This class functions essentially as an abstract interface that contains
 * some utility methods. It is the base class for connection classes that
 * are used by the sink side of the connection.
 *
 * With proper use of Processor class's Processor::specifyInputSpace() method
 * you should only ever need to use one method; readSamples().
 *
 * Other methods are included for convenience, such as readSample() and
 * readSecond(). The "peeking" methods are included for more esoteric coding,
 * and advanced coders may find the waitForSamples() and samplesReady() methods
 * quite useful.
 */
class xLConnection: virtual public Connection
{
	friend class Processor;
	friend class HeavyProcessor;
	friend class CoProcessor;

protected:
	Sink *theSink;  ///< Sink object that anchors thit end of the connection.
	uint theSinkIndex;  ///< Index into the Sink object, if any.

	/** @internal
	 * Resets the connection object.
	 *
	 * Should be called between processing sessions (when all processors are
	 * stopped). Only needs to be called if the Connection object is persistant
	 * between sessions.
	 */
	virtual void reset() = 0;

	/** @internal
	 * Notifies the object that the sink processor will be stopped.
	 *
	 * Allows blocking calls to exit, if they belong to the sink processor.
	 *
	 * This constitutes the initial part of a shutdown.
	 *
	 * @sa sinkStopped()
	 */
	virtual void sinkStopping() = 0;

	/** @internal
	 * Notifies the object that the sink processor has been stopped.
	 *
	 * Restores normal blocking calls behaviour from sinkStopping().
	 *
	 * This constitutes the final part of a shutdown.
	 *
	 * @sa sinkStopping()
	 */
	virtual void sinkStopped() = 0;

	/** @internal
	 * Blocks until either:
	 *
	 * a) It discards up to @a samples from the input stream and it then reads
	 * a plunger. In this case, it returns false.
	 *
	 * b) It discards nothing, but guarantees that at least @a samples can be
	 * read from the input stream immediately and without interruption. In this
	 * case, it returns true.
	 *
	 * On exit it guarantees either a specific number of samples can be read
	 * (like waitUntilReady()), or that it has gotten rid of at least one
	 * plunger. If neither plunger nor enough data are forthcoming in the input
	 * stream, it will block indefinately.
	 */
	virtual bool plungeSync(uint samples) const = 0;

	/** @internal
	 * Will return true if @a elements can be read from the input stream without
	 * blocking. Any plungers that would block this from being true are skipped.
	 * If a plunger is skipped then the sink is notified. Will return false if
	 * @a elements could be read, but also if sometime in the future @a prefer
	 * could be made available.
	 *
	 * NON-BLOCKING.
	 */
	virtual bool require(uint samples, uint preferSamples = Undefined) = 0;

	/** @internal
	 * Returns the number of elements currently available to be read without
	 * blocking.
	 *
	 * @return The number of elements currently ready.
	 * This is guaranteed to be correct at some point between the start of the
	 * call and the return of the call, but no where in particular. This is
	 * essentially a technical issue. As such the value returned may be an
	 * underestimate.
	 */
	virtual uint elementsReady() const = 0;

	/** @internal
	 * Waits until a read @a elements elements can be guaranteed without
	 * blocking.
	 *
	 * @param elements Numnber of elements to wait for.
	 */
	virtual void waitForElements(uint elements) const = 0;

	/** @internal
	 * Reads a number of samples or seconds (or elements, but this is
	 * discouraged).
	 *
	 * If @a elements is nonzero, this may block if there isn't enough data to
	 * be read.
	 *
	 * The Buffer is only guaranteed to be purged of the elements after all
	 * BufferData objects referencing the data at the one returned are
	 * destroyed.
	 *
	 * @param elements Number of elements to be read. If zero, then as much as
	 * possible data is read, without blocking. In the event that no data is
	 * available, the BufferData will be of size 0.
	 * @return BufferData of size @a elements which contains the next
	 * @a elements elements from the source.
	 *
	 * @sa peekElements()
	 */
	virtual const BufferData readElements(uint elements) = 0;

	/** @internal plunge
	 * Reads a number of samples or seconds (or elements, but this is
	 * discouraged).
	 *
	 * If @a elements is nonzero, this may block if there isn't enough data to
	 * be read.
	 *
	 * Elements are not removed from buffer, so any subsequent calls will
	 * return same data.
	 *
	 * @param elements Number of elements to be read. If zero, then as much as
	 * possible data is read, without blocking. In the event that no data is
	 * available, the BufferData will be of size 0.
	 * @return BufferData of size @a elements which contains the next
	 * @a elements elements from the source.
	 *
	 * @sa readElements
	 */
	virtual const BufferData peekElements(uint elements) = 0;

	/** @internal
	 * Deletes the reader. Any reads must now be done by creating your own
	 * BufferReader with newReader().
	 *
	 * @note Do not call the standard reading methods. They are now unsafe.
	 *
	 * @sa resurectReader() @sa newReader()
	 */
	virtual void killReader() = 0;

	/** @internal
	 * Reinstates the reader. This undoes the killReader() operation, and
	 * therefore should only be called after a killReader().
	 *
	 * The standard reading methods will work properly again after this call.
	 *
	 * @sa killReader() @sa newReader()
	 */
	virtual void resurectReader() = 0;

	/** @internal
	 * Creates and returns a new reader for the source buffer.
	 *
	 * @return Pointer to a new BufferReader object, that is associated with
	 * the source Buffer object.
	 */
	virtual BufferReader *newReader() = 0;

	/** @internal
	 * Simple constructor.
	 */
	xLConnection(Sink *newSink, uint newSinkIndex);

public:
	/**
	 * Retrieves the type of signal this connection transfers.
	 *
	 * @return A SignalTypeRef of this conection's SignalType.
	 */
	virtual const SignalTypeRef type() { return SignalTypeRef(theType); }

	/**
	 * Blocks until at least @a samples are ready to be read (or peeked) on the
	 * connection.
	 *
	 * Generally you will not need to use this method. It's only here for those
	 * with a good knowledge of how the code works internally or those willing
	 * to experiment to try to optimise their code into oblivion.
	 *
	 * @param samples The number of samples this call should no longer block
	 * at.
	 */
	void waitForSamples(uint samples = 1) const { waitForElements(theType->elementsFromSamples(samples)); }

	/**
	 * Checks how many samples are currently ready to be read. This is correct
	 * at some point between calling and returning, though Geddei does not (and
	 * cannot) guarantee when exactly.
	 *
	 * Generally you will not need to use this method, and be warned, the
	 * semantics can be slightly tricky, essentially due to the lack of
	 * synchronisation facilities. It's only here for those with a good
	 * knowledge of how the code works internally or those willing to
	 * experiment to try to optimise their code into oblivion.
	 *
	 * @return The number of samples ready. Due to the timing semantics, this
	 * may be an underestimate.
	 */
	uint samplesReady() const { return theType->samples(elementsReady()); }

	/**
	 * Read a single sample from the connection. This will block until a sample
	 * is ready to be read.
	 *
	 * From the API user's point of view the sample is immediately disguarded
	 * from the stream. Technically, if you're interested, it may only actually
	 * be disguarded once the BufferData object (and all shared copies of it)
	 * is destroyed. This happens automatically thanks to the cleverness of
	 * BufferData.
	 *
	 * @return A BufferData object containing the sample read.
	 */
	const BufferData readSample() { return readElements(theType->elementsFromSamples(1)); }

	/**
	 * Read a number of samples from the connection. If a non-zero @a samples
	 * is provided, it will block until @a samples samples are ready on the
	 * connection, thus you will only ever have the exact amount you require
	 * returned.
	 *
	 * Under default conditions, specifying @a samples of zero (the default)
	 * will return a BufferDatas containing all the samples currently
	 * available, once some samples are available. To make it return
	 * immediately, even if no samples are available, set @a allowZero to true.
	 * This will, of course, make valid the case of returning a BufferData
	 * object of size 0.
	 *
	 * From the API user's point of view the data is immediately disguarded
	 * from the stream. Technically, if you're interested, it may only actually
	 * be disguarded once the BufferData object (and all shared copies of it)
	 * is destroyed. This happens automatically thanks to the cleverness of
	 * BufferData.
	 *
	 * @param samples The number of samples to read. If zero, an arbitrary
	 * number of samples will be read, depending upon the current contents of
	 * the buffer.
	 * @param allowZero If true, a BufferData of size zero is allowed to be
	 * returned, and the call will never block.
	 * @return A BufferData object containing the samples read. If samples > 0
	 * then this is guaranteed to contain exactly @a samples samples.
	 */
	const BufferData readSamples(uint samples = 0, bool allowZero = false) { if (!allowZero && !samples) while (!samplesReady()) plungeSync(1); return readElements(theType->elementsFromSamples(samples ? samples : samplesReady())); }

	/**
	 * Read a second's worth of signal data from the connection. This will block
	 * until a second's worth is ready to be read.
	 *
	 * From the API user's point of view the data is immediately disguarded
	 * from the stream. Technically, if you're interested, it may only actually
	 * be disguarded once the BufferData object (and all shared copies of it)
	 * is destroyed. This happens automatically thanks to the cleverness of
	 * BufferData.
	 *
	 * @return A BufferData object containing the second of data read.
	 */
	const BufferData readSecond() { return readElements(theType->elementsFromSeconds(1.)); }

	/**
	 * Read a number of seconds' worth of signal data from the connection. This
	 * will block until enough data is ready to be read.
	 *
	 * From the API user's point of view the data is immediately disguarded
	 * from the stream. Technically, if you're interested, it may only actually
	 * be disguarded once the BufferData object (and all shared copies of it)
	 * is destroyed. This happens automatically thanks to the cleverness of
	 * BufferData.
	 *
	 * @return A BufferData object containing the data read. This is guarenteed
	 * to be exactly @a seconds seconds of signal data.
	 */
	const BufferData readSeconds(float seconds) { return readElements(theType->elementsFromSeconds(seconds)); }

	/**
	 * Read a single sample from the connection. This will block until a sample
	 * is ready to be read. The data will not be removed from the connection's
	 * stream and further calls to read data from this connection will reread
	 * the data.
	 *
	 * @return A BufferData object containing the sample read.
	 */
	const BufferData peekSample() { return peekElements(theType->elementsFromSamples(1)); }

	/**
	 * Read a number of samples from the connection. If a non-zero @a samples
	 * is provided, it will block until @a samples samples are ready on the
	 * connection, thus you will only ever have the exact amount you require
	 * returned. The data will not be removed from the connection's stream and
	 * further calls to read data from this connection will reread the data.
	 *
	 * Under default conditions, specifying @a samples of zero (the default)
	 * will return a BufferDatas containing all the samples currently
	 * available, once some samples are available. To make it return
	 * immediately, even if no samples are available, set @a allowZero to true.
	 * This will, of course, make valid the case of returning a BufferData
	 * object of size 0.
	 *
	 * @param samples The number of samples to read. If zero, an arbitrary
	 * number of samples will be read, depending upon the current contents of
	 * the buffer.
	 * @param allowZero If true, a BufferData of size zero is allowed to be
	 * returned, and the call will never block.
	 * @return A BufferData object containing the samples read. If samples > 0
	 * then this is guaranteed to contain exactly @a samples samples.
	 */
	const BufferData peekSamples(uint samples = 0, bool allowZero = false) { if (!allowZero && !samples) while (samplesReady() < 1) plungeSync(1); return peekElements(theType->elementsFromSamples(samples ? samples : samplesReady())); }

	/**
	 * Read a second's worth of signal data from the connection. This will
	 * block until a second's worth is ready to be read. The data will not
	 * be removed from the connection's stream and further calls to read
	 * data from this connection will reread the data.
	 *
	 * @return A BufferData object containing the second of data read.
	 */
	const BufferData peekSecond() { return peekElements(theType->elementsFromSeconds(1.)); }

	/**
	 * Read a number of seconds' worth of signal data from the connection. This
	 * will block until enough data is ready to be read. The data will not be
	 * removed from the connection's stream and further calls to read data from
	 * this connection will reread the data.
	 *
	 * @return A BufferData object containing the data read. This is guarenteed
	 * to be exactly @a seconds seconds of signal data.
	 */
	const BufferData peekSeconds(float seconds) { return peekElements(theType->elementsFromSeconds(seconds)); }

	/**
	 * Get the capacity of the buffer. Any reads above this amount will block
	 * indefinately.
	 *
	 * @return The number of samples that this buffer can hold at once.
	 */
	virtual uint capacity() const = 0;

	/**
	 * Get the current filled-ness of the buffer.
	 *
	 * @return 0 if there is no buffer on this side of the connection.
	 * Otherwise return the filledness relative to the size (range 0. to 1.).
	 */
	virtual float filled() const { return 0.; }

	/**
	 * Simple destructor.
	 */
	virtual ~xLConnection();

};

}
