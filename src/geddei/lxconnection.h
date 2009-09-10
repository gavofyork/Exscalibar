/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_LXCONNECTION_H
#define _GEDDEI_LXCONNECTION_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "connection.h"
#include "bufferdata.h"
#include "signaltyperef.h"
#else
#include <geddei/connection.h>
#include <geddei/bufferdata.h>
#include <geddei/signaltyperef.h>
#endif
using namespace Geddei;

namespace Geddei
{

class Source;

/** @ingroup Geddei
 * @brief Refinement of Connection from a local Source.
 * @author Gav Wood <gav@kde.org>
 *
 * This class functions essentially as an abstract interface that contains
 * some utility methods. It is the base class for connection classes that
 * are used by the source side of the connection.
 *
 * With proper use of Processor class's Processor::specifyOutputSpace() method
 * you should only ever need to use two methods; makeScratchSamples() and
 * push(). maximumScratchSamples() may also be useful for those concerned with
 * efficiency.
 *
 * Other methods are included for completeness to allow currently barely
 * imagined uses.
 */
class LxConnection: virtual public Connection, public ScratchOwner
{
	friend class Processor;
	friend class DomProcessor;
	friend class Splitter;

	float *theScratch;
	uint theScratchSize;
	BufferID lastScratch;

	/**
	 * Reimplementation from ScratchOwner.
	 *
	 * Almost same as push(); used for polymorphism needed for auto cleanup.
	 *
	 * @param data The relevant data for the auxilliary operation.
	 */
	virtual void pushScratch(const BufferData &data);

	/**
	 * Reimplementation from ScratchOwner.
	 *
	 * Will forget all about last scratch; used for polymorphism needed for
	 * auto cleanup.
	 *
	 * @param data The relevant data for the auxilliary operation.
	 */
	virtual void forgetScratch(const BufferData &data);

	/**
	 * @name Methods for use by Processor.
	 */
	//@{

	/**
	 * Inserts a plunger into the connection's data stream. You should not
	 * generally use this method directly unless you know what you are doing.
	 * Instead use Processor::plunge(), which will insert a plunger into all
	 * output connections.
	 *
	 * Notifies any receiving Sink objects that anther plunger will be ready to
	 * be received in the future.
	 *
	 * @note To avoid any confusion regarding order, do not call this when
	 * there are any active (unpushed) scratch BufferData objects around.
	 *
	 * @sa Processor::plunge()
	 */
	virtual void pushPlunger() = 0;

	/**
	 * Blocks until the Sink is ready to receive data (i.e. all types are
	 * initialised ok).
	 *
	 * @return false if something went wrong.
	 */
	virtual bool waitUntilReady() = 0;
	virtual Tristate isReadyYet() = 0;

	/**
	 * Conducts the setting of the type if an object wants to do it without
	 * being asked first.
	 *
	 * Syncs it with sink connection if the two are different objects.
	 *
	 * @param type The SignalType object this connection's type should be set
	 * to.
	 */
	virtual void setType(const SignalType *type) = 0;

	/**
	 * Resets the type so that a further setType call will be needed.
	 * Syncs it with sink connection if the two are different objects.
	 */
	virtual void resetType() = 0;

	/**
	 * Call to allow trapdoor-like returning from blocking methods.
	 * Used for controlled exiting (the thereIsInputForProcessing() after such methods will enforce exiting from main loop by throwing an exception).
	 */
	virtual void sourceStopping() = 0;

	/**
	 * Called to signify trapdoor-like returning is no-longer needed for this thread.
	 */
	virtual void sourceStopped() = 0;

	/**
	 * Resets anything that needs to be done between processor being stopped and started.
	 */
	virtual void reset() = 0;

	/**
	 * This will let the opposite end of the connection know that from now on
	 * we may receive a new set of plunger notifications.
	 */
	virtual void startPlungers() = 0;

	/**
	 * This will let the opposite end of the connection know that the source is
	 * done, so they know that no more data will follow.
	 */
	virtual void noMorePlungers() = 0;

	/**
	 * This will let the opposite end of the connection know that the source
	 * is about to push a plunger.
	 */
	virtual void plungerSent() = 0;

	//@}

protected:
	Source *theSource;
	uint theSourceIndex;

	/**
	 * Simple constructor.
	 */
	LxConnection(Source *source, uint sourceIndex);

	/**
	 * Get the maximum amount of scratch elements we can make that won't cause
	 * a deadlock if we try to create a scratch of that size. If no elements
	 * are free, will block until elements are free, rather than return zero.
	 *
	 * @param minimum The minimum number of elements it should return. This
	 * must be less than or equal to maximumScratchElementsEver().
	 * @return Undefined (= (uint)-1) in the case of unlimited.
	 */
	virtual uint maximumScratchElements(uint minimum = 1) = 0;

	/**
	 * Returns the maximum amount of scratch elements we could ever make that
	 * won't (definately) cause a deadlock.
	 *
	 * Typically this value should be divided by two to be sure that the
	 * scratch size asked for will definately be possible.
	 *
	 * @return Undefined (= (uint)-1) in the case of unlimited.
	 */
	virtual uint maximumScratchElementsEver() = 0;

	/** @deprecated
	 * TODO: Kill this method
	 * Create a new scratch pad with which data may be sent efficiently.
	 *
	 * Use push() to send the data, or set autoPush to true for it to happen
	 * automatically.
	 *
	 * This method may block if no resource is available.
	 *
	 * For internal note:
	 * This implementation makes an (inefficient) unassociated scratch object.
	 * Reimplement to provide a mroe efficient solution.
	 *
	 * @param elements The size of the data chunk required in elements.
	 * @param autoPush If true, then on destruction of the returned BufferData
	 * object, it will get pushed automatically. This would happen, for
	 * instance, when it goes out of semantic scope.
	 * @return The BufferData object into which data can be written.
	 */
	virtual BufferData makeScratchElements(uint elements, bool autoPush = false);

public:
	/**
	 * Retrieves the type of signal this connection transfers.
	 *
	 * @return A SignalTypeRef of this conection's SignalType.
	 */
	virtual const SignalTypeRef type() { return SignalTypeRef(theType); }

	/**
	 * Returns the amount of free ELEMENTS in the destination buffer (trivial on LL but
	 * harder for LR). Useful for gauging amount to push.
	 */
	virtual uint bufferElementsFree() = 0;

	/**
	 * Create a new scratch pad with which data may be sent efficiently.
	 *
	 * Use push() to send the data, or set @a autoPush to true for it to happen
	 * automatically.
	 *
	 * This method may block if no resource is available.
	 *
	 * @param samples The size of the data chunk required in samples.
	 * @param autoPush If true, then on destruction of the returned BufferData
	 * object, it will get pushed automatically. This would happen, for
	 * instance, when it goes out of semantic scope.
	 * @return The BufferData object into which data can be written.
	 */
	virtual BufferData makeScratchSamples(uint samples, bool autoPush = false);

	/**
	 * Create a new scratch pad with which data may be sent efficiently. The
	 * scratch pad is one sample long.
	 *
	 * Use push() to send the data, or set @a autoPush to true for it to happen
	 * automatically.
	 *
	 * This method may block if no resource is available.
	 *
	 * @param autoPush If true, then on destruction of the returned BufferData
	 * object, it will get pushed automatically. This would happen, for
	 * instance, when it goes out of semantic scope.
	 * @return The BufferData object into which data can be written.
	 */
	virtual BufferData makeScratchSample(bool autoPush = false) { return makeScratchSamples(1, autoPush); }

	/**
	 * Create a new scratch pad with which data may be sent efficiently.
	 *
	 * Use push() to send the data, or set @a autoPush to true for it to happen
	 * automatically.
	 *
	 * This method may block if no resource is available.
	 *
	 * @param seconds The size of the data chunk required in seconds.
	 * @param autoPush If true, then on destruction of the returned BufferData
	 * object, it will get pushed automatically. This would happen, for
	 * instance, when it goes out of semantic scope.
	 * @return The BufferData object into which data can be written.
	 */
	virtual BufferData makeScratchSeconds(float seconds, bool autoPush = false);

	/**
	 * Create a new scratch pad with which data may be sent efficiently. The
	 * scratch is exactly one second long.
	 *
	 * Use push() to send the data, or set @a autoPush to true for it to happen
	 * automatically.
	 *
	 * This method may block if no resource is available.
	 *
	 * @param seconds The size of the data chunk required in seconds.
	 * @param autoPush If true, then on destruction of the returned BufferData
	 * object, it will get pushed automatically. This would happen, for
	 * instance, when it goes out of semantic scope.
	 * @return The BufferData object into which data can be written.
	 */
	virtual BufferData makeScratchSecond(bool autoPush = false) { return makeScratchSeconds(1., autoPush); }

	/**
	 * Transfers @a data down the connection to its sink.
	 *
	 * If @a data is a native scratch, it becomes invalid, and any automatic
	 * cleanups it has will be removed.
	 *
	 * This method will block until the data is sent (though not neccessarily
	 * received).
	 *
	 * @param data The data to be transferred down this connection.
	 */
	virtual void push(const BufferData &data) = 0;

	/**
	 * Get the maximum amount of scratch samples we can make that won't cause
	 * a deadlock if we try to create a scratch of that size. If no samples
	 * are free, will block until samples are free, rather than return zero.
	 *
	 * @param minimum The minimum number of samples it should return. This
	 * must be less than or equal to maximumScratchSamplesEver().
	 * @return Undefined (= (uint)-1) in the case of unlimited.
	 */
	uint maximumScratchSamples(uint minimum = 1) { return maximumScratchElements(theType->elementsFromSamples(minimum)) == Undefined ? Undefined : theType->samples(maximumScratchElements(theType->elementsFromSamples(minimum))); }

	/**
	 * Returns the maximum amount of scratch samples we could ever make that
	 * won't (definately) cause a deadlock.
	 *
	 * Typically this value should be divided by two to be sure that the
	 * scratch size asked for will definately be possible.
	 *
	 * @return Undefined (= (uint)-1) in the case of unlimited.
	 */
	uint maximumScratchSamplesEver() { return maximumScratchElementsEver() == Undefined ? Undefined : theType->samples(maximumScratchElementsEver()); }

	/**
	 * Some syntactic sugar, if you're into that sort of thing. Equivalent to
	 * the push() method.
	 *
	 * That is, the following two statements are sematically identical:
	 *
	 * @code
	 * connection.push(d);
	 * connection << d;
	 * @endcode
	 *
	 * @param data The data to be transferred down this connection.
	 * @return A reference to this object.
	 */
	LxConnection &operator<<(const BufferData &data) { push(data); return *this; }

	/**
	 * Some syntactic sugar, if you're into that sort of thing. Equivalent to
	 * the makeScratchSamples() method.
	 *
	 * That is, the following two statements are sematically identical:
	 *
	 * @code
	 * BufferData d = connection.makeScratchSamples(5, false);
	 * BufferData d = connection + 5;
	 * @endcode
	 *
	 * @param samples The size of the data chunk required in samples.
	 * @return The BufferData object into which data can be written.
	 */
	BufferData operator+(uint samples) { return makeScratchSamples(samples); }

	/**
	 * Simple destructor.
	 */
	~LxConnection();
};

}

#endif
