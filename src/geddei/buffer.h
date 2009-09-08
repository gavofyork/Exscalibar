#ifndef _SUCS_BUFFER_H
#define _SUCS_BUFFER_H

#include <iostream>
#include <cassert>
using namespace std;

#include <QThread>
#include <QMutex>
#include <QVector>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "qfastwaitcondition.h"
#include "bufferdata.h"
#include "signaltype.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <geddei/bufferdata.h>
#include <geddei/signaltype.h>
#endif
using namespace Geddei;

namespace Geddei
{

class BufferReader;
class Processor;

/** @internal @ingroup Geddei
 * @brief Class to encompass an efficient threadsafe databank.
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class Buffer: public ScratchOwner
{
	friend class BufferReader;

	mutable QFastWaitCondition theDataIn, theDataOut;
	mutable QMutex theDataFlux;

	float *theData;
	uint theLogSize, theSize, theMask, theUsed;
	uint readPos, writePos;
	BufferInfo *lastScratch;
	SignalType *theType;
	QVector<const Processor *> theTrapdoors;
	QList<uint> thePlungers;
	QList<BufferReader*> theReaders;

	friend ostream &operator<<(ostream &out, Buffer &me);

	/** theDataFlux ***MUST*** be locked before calling any methods who end in UNSAFE. */

	/**
	 * Updates the position of the global readPos from the registered readers.
	 * This should be called after any operation on a reader's position.
	 * Will discard any plungers that are *before* the new readPos.
	 */
	void updateUNSAFE();

	bool trapdoorUNSAFE() const;
	uint waitForUNSAFE(uint elements) const;
	uint waitForUNSAFE(uint elements, const BufferReader *reader) const;
	uint waitForIgnorePlungersUNSAFE(uint elements, const BufferReader *reader) const;
	void waitForFreeUNSAFE(uint elements) const;

	/**
	 * Discards the next plunger, if there is one.
	 * Thread-safe.
	 */
	void discardNextPlungerUNSAFE();

	/**
	 * Simply finds the next plunger from readPos and returns its position in
	 * the buffer.
	 *
	 * @return -1 if there isn't one.
	 */
	int nextPlungerUNSAFE() const;

	/**
	 * Finds the next plunger from @a pos and returns its position relative to
	 * @a pos. Upto @arg ignore plungers are ignored at position @a pos.
	 *
	 * @return -1 if there isn't such a plunger.
	 */
	int nextPlungerUNSAFE(uint pos, uint ignore) const;

public:
	// USE THIS FROM processor on stop() - dont forget to reinitialise
	void openTrapdoor(const Processor *processor);
	void closeTrapdoor(const Processor *processor);

	/**
	 * Discards the next plunger, if there is one.
	 *
	 * Thread-safe.
	 */
	void discardNextPlunger();

	/**
	 * Checks for the space availability of elements in the buffer.
	 *
	 * Thread-safe.
	 */
	uint elementsFree() const;

	/**
	 * Waits until the buffer is at least empty enough to allow a number of
	 * elements to be pushed into it or a native scratch to be made.
	 *
	 * @note The buffer size should be at least twice as big as any size waited
	 * upon to prevent WSD.
	 *
	 * Thread-safe. Blocking.
	 */
	void waitForFreeElements(uint elements) const;

	/**
	 * Creates a scratch of a given length in either samples or seconds (or elements, but this is discouraged).
	 *
	 * Thread-safe.
	 */
	BufferData makeScratchElements(uint elements, bool autoPush = false);

	/**
	 * Invalidates the last scratch. Should only be used if scratch was *not* auto-pushing.
	 * Can be called automatically by non-auto-pushing native scratches at end of life.
	 *
	 * Thread-safe.
	 */
	void forgetScratch(const BufferData &data);

	/**
	 * Pushes native data into Buffer. Will not cause a block. Invalidates the scratch passed as the argument.
	 * Can be called automatically at end of life of a BufferData.
	 *
	 * Thread-safe.
	 */
	void pushScratch(const BufferData &data);

	/**
	 * Pushes non-native data into Buffer. May block if Buffer is full. Invalidates the last scratch silently.
	 *
	 * Thread-safe. Blocking.
	 */
	void pushData(const BufferData &data);

	/**
	 * Puts all of data into buffer, blocking until complete. Invalidates the last scratch silently (unless == data, in which case not silently).
	 * May block if data is non-native.
	 *
	 * Thread-safe. Blocking (potentially).
	 */
	void push(const BufferData &data);

	/**
	 * Appends a plunger to the current data.
	 *
	 * Thread-safe.
	 */
	void appendPlunger();

	/**
	 * Clears buffer.
	 * Shouldn't be called with native BufferData objects floating.
	 */
	void clear();

	/**
	 * Resizes buffer.
	 * Shouldn't be called with native BufferData objects floating.
	 */
	void resize(uint size);

	/**
	 * Sets the buffer's SignalType. Causes a clear() (see caveats for clear()).
	 */
	void setType(const SignalType *type);

	/**
	 * Returns the size of the buffer.
	 */
	uint size() const { return theSize; }

	/**
	 * Returns the type of the buffer.
	 */
	const SignalType *type() const { return theType; }

	/**
	 * Some syntactic sugar, if you're into that sort of thing.
	 */
	Buffer &operator<<(const BufferData &data) { push(data); return *this; }

	void debug();

	Buffer(uint size, const SignalType *type = 0);
	virtual ~Buffer();
};

}

#endif
