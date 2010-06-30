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
#include "connection.h"
#include "bufferdata.h"
#include "type.h"
#else
#include <geddei/connection.h>
#include <geddei/bufferdata.h>
#include <geddei/type.h>
#endif
using namespace Geddei;

namespace Geddei
{

class Source;
class SubProcessor;

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
class DLLEXPORT LxConnection: virtual public Connection, public ScratchOwner
{
	friend class Processor;
	friend class HeavyProcessor;
	friend class CoProcessor;
	friend class DomProcessor;
	friend class Splitter;

public:
	/**
	 * Simple destructor.
	 */
	~LxConnection();

	Type const& writeType() const { return m_sub ? m_midType : type(); }

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
	 * instance, when it goes out of semanticscope.
	 * @return The BufferData object into which data can be written.
	 */
	BufferData makeScratchSamples(uint samples, bool autoPush = false);

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
	 * instance, when it goes out of semanticscope.
	 * @return The BufferData object into which data can be written.
	 */
	BufferData makeScratchSample(bool autoPush = false) { return makeScratchSamples(1, autoPush); }

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
	void push(BufferData const& _data);

	void setSub(SubProcessor* _s);

	/**
	 * Get the maximum amount of scratch samples we can make that won't cause
	 * a deadlock if we try to create a scratch of that size. If no samples
	 * are free, will block until samples are free, rather than return zero.
	 *
	 * @param minimum The minimum number of samples it should return. This
	 * must be less than or equal to maximumScratchSamplesEver().
	 * @return Undefined (= (uint)-1) in the case of unlimited.
	 */
	uint maximumScratchSamples(uint minimum = 1);

	/**
	 * Returns the maximum amount of scratch samples we could ever make that
	 * won't (definately) cause a deadlock.
	 *
	 * Typically this value should be divided by two to be sure that the
	 * scratch size asked for will definitely be possible.
	 *
	 * @return Undefined (= (uint)-1) in the case of unlimited.
	 */
	uint maximumScratchSamplesEver();

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

	virtual void setMinimumRead(uint _s);
	virtual void setMinimumWrite(uint _s);

protected:
	/**
	 * Simple constructor.
	 */
	LxConnection(Source *source, uint sourceIndex);

	/**
	 * Retrieves the type of signal this connection transfers.
	 *
	 * @return A Type of this conection's TransmissionType.
	 */
	virtual Type const& type() const { return theType; }

	virtual void pushBE(const BufferData &data) = 0;

	/**
	 * Get the maximum amount of scratch elements we can make that won't cause
	 * a deadlock if we try to create a scratch of that size. If no elements
	 * are free, will block until elements are free, rather than return zero.
	 *
	 * @param minimum The minimum number of elements it should return. This
	 * must be less than or equal to freeInDestinationBufferEver().
	 * @return Undefined (= (uint)-1) in the case of unlimited.
	 */
	virtual uint freeInDestinationBuffer(uint minimum = 1) = 0;

	/**
	 * Returns the maximum amount of scratch elements we could ever make that
	 * won't (definitely) cause a deadlock.
	 *
	 * Typically this value should be divided by two to be sure that the
	 * scratch size asked for will definately be possible.
	 *
	 * @return Undefined (= (uint)-1) in the case of unlimited.
	 */
	virtual uint freeInDestinationBufferEver() = 0;

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
	 * instance, when it goes out of semanticscope.
	 * @return The BufferData object into which data can be written.
	 */
	virtual BufferData makeScratchElements(uint elements, bool autoPush = false);

	/**
	 * Conducts the setting of the type if an object wants to do it without
	 * being asked first.
	 *
	 * Syncs it with sink connection if the two are different objects.
	 *
	 * @param type The TransmissionType object this connection's type should be set
	 * to.
	 */
	virtual void setType(Type const& _type);

	Source *theSource;
	uint theSourceIndex;

private:
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

	float *theScratch;
	uint theScratchSize;
	BufferID lastScratch;

	SubProcessor* m_sub;
	Type m_midType;
	QVector<float> m_midScratch;
	uint m_leftOver;
};

}
