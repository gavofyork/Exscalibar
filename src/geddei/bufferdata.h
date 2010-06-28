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

#include <iostream>
#include <cassert>
using namespace std;

#include <QMap>
#include <QMutex>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "bufferinfo.h"
#include "transmissiontype.h"
#include "globals.h"
#else
#include <geddei/bufferinfo.h>
#include <geddei/transmissiontype.h>
#include <geddei/globals.h>
#endif
using namespace Geddei;

namespace Geddei
{

class Buffer;
class LRConnection;
class Splitter;
class BufferData;
class Auxilliary;

typedef BufferInfo *BufferID;

/** @internal @ingroup Geddei
 * @brief Abstract class to represent classes that handle BufferData objects.
 * @author Gav Wood <gav@kde.org>
 *
 * BufferData objects may be "owned" by other Geddei objects. In this case it
 * may be useful to have default actions upon destruction of the BufferData,
 * such as writing it out to its owner or ignoring the read operation it
 * represents.
 *
 * An Auxilliary-derived object is such an object that is able to own and
 * provide these default "happen-on-death" operations.
 */
class Auxilliary
{
public:
	/**
	 * Resolve the data with its owner in a way such that no overall change
	 * happens in the system. Essentially a rollback style operation.
	 *
	 * @param data The data whose deletion is initiating the operation.
	 */
	virtual void forget(const BufferData &) = 0;

	/**
	 * Resolve the data with its owner in a way such that the assumed operation
	 * happens in the system. Essentially a commit style operation.
	 *
	 * @param data The data whose deletion is initiating the operation.
	 */
	virtual void activate(const BufferData &) = 0;

	/**
	 * Virtual destructor.
	 */
	virtual ~Auxilliary() {}
};

/** @internal @ingroup Geddei
 * @brief An Auxilliary-derived class to represent a scratch owner.
 * @author Gav Wood <gav@kde.org>
 *
 * A scratch, Geddei speak for a writable BufferData object may be "owned" by
 * some objects. If these object are derived form ScratchOwner, auxilliary
 * methods may be given for allowing the scratch in question to be
 * automatically dealt with on destruction.
 *
 * In the case of scratches, this is either pushing out to the connection or
 * forgetting all about it.
 */
class ScratchOwner: public Auxilliary
{
public:
	/**
	 * Transfer @a data to connection.
	 *
	 * @param data The data whose deletion is initiating the operation.
	 */
	virtual void pushScratch(const BufferData &) = 0;

	/**
	 * Return @a data back whence it came, in this case probably just ignore it
	 * ever existed.
	 *
	 * @param data The data whose deletion is initiating the operation.
	 */
	virtual void forgetScratch(const BufferData &) = 0;

	void forget(const BufferData &i) { forgetScratch(i); }
	void activate(const BufferData &i) { pushScratch(i); }

	/**
	 * Virtual destructor.
	 */
	virtual ~ScratchOwner() {}
};

/** @internal @ingroup Geddei
 * @brief An Auxilliary-derived class to represent a screen owner.
 * @author Gav Wood <gav@kde.org>
 *
 * A screen, Geddei speak for a readable BufferData object may be "owned" by
 * some objects. If these object are derived form ScreenOwner, auxilliary
 * methods may be given for allowing the screen in question to be
 * automatically dealt with on destruction.
 *
 * In the case of screens, this is either dismissing and rolling back the read
 * operation or allowing the read to come to its logical end and freeing any
 * resources held by the read's data.
 */
class ScreenOwner: public Auxilliary
{
public:
	/**
	 * Assert the reading came to its logical end and the data may be duely
	 * disguarded.
	 *
	 * @param data The data whose deletion is initiating the operation.
	 */
	virtual void haveRead(const BufferData &) = 0;

	/**
	 * Return the data back whence it came, essentially undoing the read
	 * operation.
	 *
	 * @param data The data whose deletion is initiating the operation.
	 */
	virtual void forgetRead(const BufferData &) = 0;

	void forget(const BufferData &i) { forgetRead(i); }
	void activate(const BufferData &i) { haveRead(i); }

	/**
	 * Virtual destructor.
	 */
	virtual ~ScreenOwner() {}
};

/** @ingroup Geddei
 * @brief Class for interfacing with signal data in Geddei.
 * @author Gav Wood <gav@kde.org>
 *
 * BufferData objects are among the most important and well used objects in
 * Geddei. The capture the essence of a data container. They present a simple
 * and intuitive interface to the developer - that of a C-style array. They
 * provide invisible data sharing capabilities. They provide methods to slice
 * up data, to recreate data as C arrays, to introduce new data from C arrays,
 * and to create new data chunks either with a new databank (i.e. not shared)
 * or sharing data from a given array.
 *
 * Elements (individual values) of data may be accessed either by their
 * position or in a tabular manner by accessing an element of a sample.
 *
 * There are such things as null BufferData objects which may or may not be
 * valid. Validity and nullness may be ascertained by the isValid() and
 * isNull() methods. A null BufferData object stores no data. If it is still
 * valid, undert most cases you need not care that it is null, since it is
 * essentially masquerading as a real data storage object. If invalid, then
 * it must not be written to or accessed in any other way since it represents
 * (essentially) a null and discarded pointer.
 *
 * @note This class is not explicitly thread-safe; you should only access a
 * BufferData object or any derivatives (made my copying it, using mid(),
 * samples() and sample()) by one thread at once.
 */
class DLLEXPORT BufferData
{
	static BufferData *theFake;

	// The attributes in here belong to the data we reference.
	// Changing them will change how all other references act upon the data, and
	// when the reference gets destroyed how the Buffer will react.
	// As such it should remain generally unchanged.
	BufferInfo *theInfo;

	// These are attributes specific to how we view the data.
	// Changing them will have no discourse beyond how it is viewed in this class.
	// In particular, if the data is written to a buffer, the original aperture
	// of the data will be used.
	uint theVisibleSize, theOffset;

	float *theWritePointer;

	friend class Buffer;
	friend class BufferReader;
	friend class RLConnection;
	friend class LxConnection;
	friend class LLConnection;
	friend class LLsConnection;
	friend class LRConnection;
	friend class DRCoupling;
	friend class RSCoupling;
	friend ostream &operator<<(ostream &out, const BufferData &me);

	friend class Splitter;
	void adopt(ScratchOwner *aux) { theInfo->theAux = aux; }

	/**
	 * Check to see if the rollover functionality of the buffer data chunk is used.
	 * If so, two seperate sends of sizes firstPart() and secondPart() would be needed.
	 * Otherwise, just firstPart() is needed.
	 */
	bool rollsOver() const { return theOffset + theVisibleSize - 1 > theInfo->theMask; }
	uint sizeOnlyPart() const { return theVisibleSize; }
	uint sizeFirstPart() const { return (theInfo->theMask + 1) - theOffset; }
	uint sizeSecondPart() const { return theOffset + theVisibleSize - (theInfo->theMask + 1); }
	const float *firstPart() const { return theInfo->theData + theOffset; }
	float *firstPart() { return theInfo->theData + theOffset; }
	const float *secondPart() const { return theInfo->theData; }
	float *secondPart() { return theInfo->theData; }
	/**
	 * Makes sure that isArray works ok.
	 * You shouldn't need to touch this.
	 */
	BufferData &dontRollOver(bool makeCopy = false);
	const BufferData &dontRollOver(bool makeCopy = true) const;

	/**
	 * Invalidates data, making it unreadable/unwritable.
	 *
	 * Used by buffers and connections to make sure that once the data pointed
	 * to becomes invalid, no access may happen on it.
	 *
	 * The data in theInfo must be destroyed or otherwise freed by the caller.
	 * This object will no longer attempt to free it automatically.
	 */
	void invalidate() const { theInfo->theValid = false; }

	/**
	 * Causes the preassigned end type to become None, meaning that on
	 * destruction of the last reference nothing will happen to the data.
	 *
	 * Used by buffers/connections if the data is explicity used, to make sure
	 * that it doesn't happen twice.
	 */
	void ignoreDeath() const { theInfo->theEndType = BufferInfo::Ignore; }

	BufferData(uint size, uint sampleSize, float *data, ScratchOwner *scratch = 0, BufferInfo::Legacy endType = BufferInfo::Ignore, uint offset = 0, uint mask = 0xffffffff);
	BufferData(uint size, uint sampleSize, float *data, ScreenOwner *screen = 0, BufferInfo::Legacy endType = BufferInfo::Ignore, uint offset = 0, uint mask = 0xffffffff);

	BufferData(BufferInfo *info, uint offset);

public:
	/** @internal
	 * Provides debug information on this class.
	 */
	void debugInfo() const;

	/** @internal
	 * Used for access to a global fake writable BufferData object.
	 */
	static BufferData &fake();

	/** @internal
	 * Grabs a the unique ID code - will be the same for all BufferData
	 * objects that share data.
	 */
	BufferID identity() const { return theInfo; }

	/** @internal
	 * Grabs the info object. This is the implicitly shared object throughout
	 * BufferData, and actually holds the data pointer.
	 *
	 * All BufferData objects that share this data will have the same info
	 * object.
	 */
	BufferInfo *info() { return theInfo; }

	/** @internal @overload
	 * Grabs the info object. This is the implicitly shared object throughout
	 * BufferData, and actually holds the data pointer.
	 *
	 * All BufferData objects that share this data will have the same info
	 * object.
	 */
	const BufferInfo *info() const { return theInfo; }

	/**
	 * Copies contents of a given BufferData to this object's data store.
	 *
	 * Can be useful when you need to output into a BufferData object different
	 * from one that you must put the output into (when in a SubProcessor).
	 *
	 * You generally shouldn't need to use this in a normal Processor object.
	 *
	 * @param source The BufferData object from which to copy the data. The
	 * two BufferData objects sizes must be the same (in elements).
	 */
	void copyFrom(const BufferData &source);

	/**
	 * Copies contents of src into this object's data store.
	 *
	 * @param source *must* be the same size as this, i.e. elements()
	 * floats long. If not you'll get memory corruption.
	 */
	void copyFrom(const float *source, uint _size);
	inline void copyFrom(QVector<float> const& _v) { copyFrom(_v.data(), _v.size()); }

	/**
	 * Copies all this data into a float array given by @a destination.
	 *
	 * @param destination *must* be the same size as this, i.e. elements()
	 * floats long. If not you'll get memory corruption.
	 */
	void copyTo(float *destination, uint _size) const;
	inline void copyTo(QVector<float>& _v) const { copyTo(_v.data(), _v.size()); }

	/**
	 * Used for checking if this data chunk was terminated by a plunger.
	 * Plungers cause premature readElements() calls.
	 *
	 * @return true iff this data is terminated by a plunger.
	 */
	bool plunger() const;

	/**
	 * Used to get the number of elements (single values) used to represent
	 * this data.
	 *
	 * You will generally not want to work in elements, but instead samples.
	 * Be very sure you know what you are doing before you use element-related
	 * methods.
	 *
	 * @return The number of elements in the data.
	 */
	uint elements() const { return theVisibleSize; }

	/** @internal
	 * Get the sampleSize of the signal data.
	 *
	 * @return The sampleSize of the signal data.
	 */
	uint sampleSize() const { return theInfo->m_sampleSize; }

	/**
	 * Used to get the number of elements inside the data chunk.
	 *
	 * @return The number of samples in the data.
	 */
	uint samples() const { return theInfo->m_sampleSize ? theVisibleSize / theInfo->m_sampleSize : 0; }

	uint checksum() const { uint ret = 0; for (uint i = 0; i < elements(); i++) ret += *(uint const*)&(operator[](i)); return ret; }

	/**
	 * Get another BufferData object referencing some portion of elements in
	 * this one.
	 *
	 * You will generally not want to work in elements, but instead samples.
	 * Be very sure you know what you are doing before you use element-related
	 * methods.
	 *
	 * @note The returned object is merely an illusionary object; the new
	 * object will, if pushed into a buffer, still push the same data as this
	 * one would. The sampleSize of the transformation is only use this in so far
	 * as reading from or writing to the this object. It has no effect on other
	 * external actions or objects.
	 *
	 * @param start The element on which the portion will begin.
	 * @param length The length of the portion to refer to in elements.
	 * @return A new (shared data) BufferData object that points to the section
	 * from this object's data.
	 */
	const BufferData mid(uint start, uint length) const;

	/** @overload
	 * Get another BufferData object referencing some portion of elements in
	 * this one.
	 *
	 * You will generally not want to work in elements, but instead samples.
	 * Be very sure you know what you are doing before you use element-related
	 * methods.
	 *
	 * @note The returned object is merely an illusionary object; the new
	 * object will, if pushed into a buffer, still push the same data as this
	 * one would. The sampleSize of the transformation is only use this in so far
	 * as reading from or writing to the this object. It has no effect on other
	 * external actions or objects.
	 *
	 * @param start The element on which the portion will begin.
	 * @param length The length of the portion to refer to in elements.
	 * @return A new (shared data) BufferData object that points to a section
	 * from this object's data.
	 */
	BufferData mid(uint start, uint length);

	/**
	 * Get another BufferData object referencing a sample of data in this one.
	 *
	 * @note The returned object is merely an illusionary object; the new
	 * object will, if pushed into a buffer, still push the same data as this
	 * one would. The sampleSize of the transformation is only use this in so far
	 * as reading from or writing to the this object. It has no effect on other
	 * external actions or objects.
	 *
	 * @param index The sample to be refered to.
	 * @return A new (shared data) BufferData object that points to the
	 * specific sample from this object's data.
	 */
	const BufferData sample(uint index) const;

	/** @overload
	 * Get another BufferData object referencing a sample of data in this one.
	 *
	 * @note The returned object is merely an illusionary object; the new
	 * object will, if pushed into a buffer, still push the same data as this
	 * one would. The sampleSize of the transformation is only use this in so far
	 * as reading from or writing to the this object. It has no effect on other
	 * external actions or objects.
	 *
	 * @param index The sample to be refered to.
	 * @return A new (shared data) BufferData object that points to the
	 * specific sample from this object's data.
	 */
	BufferData sample(uint index);

	/**
	 * Returns a new (shared data) BufferData object that points to specific
	 * samples from this object's data.
	 *
	 * @note The returned object is merely an illusionary object; the new
	 * object will, if pushed into a buffer, still push the same data as this
	 * one would. The sampleSize of the transformation is only use this in so far
	 * as reading from or writing to the this object. It has no effect on other
	 * external actions or objects.
	 *
	 * @param index The sample from which the range of data to be refered to
	 * begins.
	 * @param amount The number of samples to be refered to.
	 * @return A new (shared data) BufferData object that points to the
	 * specific samples from this object's data.
	 */
	const BufferData samples(uint index, uint amount) const;
	const BufferData samples(uint index) const { return samples(index, samples() - index); }

	/** @overload
	 * Returns a new (shared data) BufferData object that points to specific
	 * samples from this object's data.
	 *
	 * @note The returned object is merely an illusionary object; the new
	 * object will, if pushed into a buffer, still push the same data as this
	 * one would. The sampleSize of the transformation is only use this in so far
	 * as reading from or writing to the this object. It has no effect on other
	 * external actions or objects.
	 *
	 * @param index The sample from which the range of data to be refered to
	 * begins.
	 * @param amount The number of samples to be refered to.
	 * @return A new (shared data) BufferData object that points to the
	 * specific samples from this object's data.
	 */
	BufferData samples(uint index, uint amount);
	BufferData samples(uint index) { return samples(index, samples() - index); }

	float rms() const;

	inline BufferData leftSamples(uint _amount) { return samples(0, _amount); }
	inline BufferData rightSamples(uint _amount) { return samples(samples() - _amount, _amount); }
	inline BufferData const leftSamples(uint _amount) const { return samples(0, _amount); }
	inline BufferData const rightSamples(uint _amount) const { return samples(samples() - _amount, _amount); }

	/**
	 * Tests for validity (invalid means it should never be used).
	 *
	 * @return true iff the BufferData object is valid.
	 */
	bool isValid() const { return theInfo->theValid; }

	/**
	 * Makes this bufferdata invalid, ending the life of the previous data.
	 *
	 * If this is the last reference to the data, any death-effects of the data
	 * will be carried out as they would be if the last reference object was
	 * deleted.
	 */
	void nullify() { *this = BufferData(true); }

	/**
	 * Tests for nullness - a null BufferData object can be used (if valid),
	 * but access will be meaningless.
	 *
	 * @return true iff this BufferData is null.
	 */
	bool isNull() const { return theInfo->theValid && theInfo->theAccessibleSize == Undefined; }

	/**
	 * Operator to give easy and transparent access to the data contained
	 * within. The parentheses operator will return a specific element of a
	 * given sample is a table-esque lookup.
	 *
	 * @note If compiled with -DEDEBUG, they do access, bounds and validity
	 * checking.
	 *
	 * @param i The sample to be accessed.
	 * @param j The element of sample @a i to be accessed. This must be
	 * less than the size (sampleSize) of a sample.
	 * @return The value of the element at position @a j in sample @a i .
	 */
	const float &operator()(uint i, uint j) const
	{
#ifdef EDEBUG
		assert(theInfo->theValid);
		assert(i * theInfo->m_sampleSize + j < theVisibleSize);
		assert(j < theInfo->m_sampleSize || !theInfo->m_sampleSize);
#endif
		return theInfo->theData[(i * theInfo->m_sampleSize + j + theOffset) & theInfo->theMask];
	}

	/** @overload
	 * Operator to give easy and transparent access to the data contained
	 * within. The parentheses operator will return a specific element of a
	 * given sample is a table-esque lookup.
	 *
	 * @note If compiled with -DEDEBUG, they do access, bounds and validity
	 * checking.
	 *
	 * @param i The sample to be accessed.
	 * @param j The element of sample @a i to be accessed. This must be
	 * less than the size (sampleSize) of a sample.
	 * @return The value of the element at position @a j in sample @a i .
	 */
	float &operator()(uint i, uint j)
	{
#ifdef EDEBUG
		assert(theInfo->theValid);
		assert(i * theInfo->m_sampleSize + j < theVisibleSize);
		assert(j < theInfo->m_sampleSize || !theInfo->m_sampleSize);
		if (theInfo->theType == BufferInfo::Read)
			qWarning("*** WARNING: You should use a _const_ BufferData object for all reads, or you\n"
					 "             might accidentally taint the data.\n");
		if (theWritePointer)
			qWarning("*** WARNING: You still have a borrowed array active. Changing any data before\n"
					 "             that has been returned will not do anything.\n");
#endif
		return theInfo->theData[(i * theInfo->m_sampleSize + j + theOffset) & theInfo->theMask];
	}

	/** @overload
	 * Array subscript type Operator to give easy and transparent access to the
	 * data contained within. This access the data in a raw manner giving
	 * direct access to the elements.
	 *
	 * You will generally *not* want to use this, instead using the parentheses
	 * operator.
	 *
	 * @note If compiled with -DEDEBUG, they do access, bounds and validity
	 * checking.
	 *
	 * @param i The element to be accessed.
	 * @return The value of the element at position @a i .
	 */
	float &operator[](uint i)
	{
#ifdef EDEBUG
		assert(i < theVisibleSize);
		assert(theInfo->theValid);
		if (theInfo->theType == BufferInfo::Read)
			qWarning("*** WARNING: You should use a _const_ BufferData object for all reads, or you\n"
					 "             might accidentally taint the data.\n");
		if (theWritePointer)
			qWarning("*** WARNING: You still have a borrowed array active. Changing any data before\n"
					 "             that has been returned will not do anything.\n");
#endif
		return theInfo->theData[(i + theOffset) & theInfo->theMask];
	}

	/**
	 * Array subscript type Operator to give easy and transparent access to the
	 * data contained within. This access the data in a raw manner giving
	 * direct access to the elements.
	 *
	 * You will generally *not* want to use this, instead using the parentheses
	 * operator.
	 *
	 * @note If compiled with -DEDEBUG, they do access, bounds and validity
	 * checking.
	 *
	 * @param i The element to be accessed.
	 * @return The value of the element at position @a i .
	 */
	const float &operator[](uint i) const
	{
#ifdef EDEBUG
		assert(i < theVisibleSize);
		assert(theInfo->theValid);
#endif
		return theInfo->theData[(i + theOffset) & theInfo->theMask];
	}

	/**
	 * Gives a single array of floats, the same length as the number of
	 * elements in this BufferData.
	 *
	 * As it's a const method, we assume that the array will be used for
	 * reading the data from. You are therefore guaranteed that it will contain
	 * the same data as would be read from this BufferData object.
	 *
	 * This should only be used if you *really* need an array (e.g. for
	 * interfacing to external libraries). Use the []/() operators for normal
	 * usage.
	 *
	 * @note Using this method may invalidate any explicit data sharing of the
	 * buffer. If this was based on sharing a databank with another BufferData
	 * (such as through samples() or the assignment operator), they may now no
	 * longer share the same databank.
	 *
	 * @return An array representation of the datastore of this object, or zero
	 * if the BufferData is null (isNull() returns true). You should either
	 * check before using readPointer() with isNull(), or check the return value of
	 * readPointer(). If zero, there is no need to populate the BufferData since
	 * the data will not be used.
	 */
	const float *readPointer() const
	{
#ifdef EDEBUG
		assert(theInfo->theValid);
#endif
		if (theInfo->theAccessibleSize == Undefined) return 0;
		dontRollOver(true);
		return &(theInfo->theData[theOffset]);
	}

	/** @overload
	 * Gives a single array of floats, the same length as the number of
	 * elements in this BufferData. Writing to the array is tantamount to
	 * writing to the BufferData object.
	 *
	 * This should only be used if you *really* need an array (e.g. for
	 * interfacing to external libraries). Use the []/() operators for normal
	 * usage.
	 *
	 * As it's not a const method, we assume that BufferData doesn't contain
	 * data yet, so array wont contain anything.
	 *
	 * The pointer returned will only be valid as long as this object exists.
	 * As soon as this object goes out of sampleSize, the pointer will become
	 * invalid. This means that the following code is incorrect:
	 *
	 * @code
	 * BufferData &d(someBufferData);
	 * float *f = d.sample(0).writePointer();
	 * f[0] = 0;
	 * d.sample(0).endWritePointer()
	 * @endcode
	 *
	 * This is because by the time we come to write to the array, the
	 * BufferData object used to make the borrowArray() call (given by
	 * d.sample(0)) will have been destroyed through being anonymous and
	 * therefore out of sampleSize. The endWritePointer() call will be useless since
	 * it is being called on a different instance.
	 *
	 * You must instead deanonymise it with a named BufferData instance:
	 *
	 * @code
	 * BufferData &d(someBufferData);
	 * BufferData portion = d.sample(0);
	 * float *f = portion.writePointer();
	 * someExternalFunctionThatPopulates(f);
	 * portion.endWritePointer();
	 * @endcode
	 *
	 * @note You must call endWritePointer() when finished with this array, and
	 * the data in the array is not guaranteed to propogate to the data in this
	 * object until you do.
	 *
	 * @return An array representation of the datastore of this object, or zero
	 * if the BufferData is null (isNull() returns true). You should either
	 * check before using readPointer() with isNull(), or check the return value of
	 * readPointer(). If zero, there is no need to populate the BufferData since
	 * the data will not be used.
	 */
	float *writePointer()
	{
#ifdef EDEBUG
		assert(theInfo->theValid);
		if (theInfo->theType == BufferInfo::Read)
			qWarning("*** WARNING: You should use a _const_ BufferData object for all read, or you\n"
					 "             might accidentally taint the data.\n");
#endif
		if (theInfo->theAccessibleSize == Undefined) return 0;
		if (!rollsOver()) return theInfo->theData + theOffset;

		if (!theWritePointer)
			theWritePointer = new float[theVisibleSize];
		return theWritePointer;
	}

	/**
	 * Forces propogation of the previously borrowed array into this BufferData
	 * object. You should call this before you attempt to push() the data.
	 *
	 * This pointer is invalid after this call.
	 */
	void endWritePointer()
	{
		if (theWritePointer)
			copyFrom(theWritePointer);
		delete theWritePointer;
		theWritePointer = 0;
	}

	/**
	 * Assignment operator. This is used to set this BufferData object to
	 * become equivalent to the BufferData object @a source.
	 *
	 * This doesn't actually copy the data - it merely makes another reference
	 * to the data that @a source uses. Changing data in one may alter the
	 * other's content.
	 *
	 * We release our reference to the data contained in this and any resources
	 * it used may be freed.
	 *
	 * @note To duplicate the data (i.e. copy the data in @a source over our
	 * current data, use copyFrom() instead.
	 *
	 * @param source The BufferData object from which we will gather our data's
	 * reference.
	 * @return A reference to this object.
	 */
	BufferData &operator=(const BufferData &source);

	/**
	 * Creates a null bufferdata object.
	 *
	 * @param valid If false (default), any attempt to access it will result in an
	 * error. Otherwise, it will still be useable, but wont actually store anything.
	 */
	BufferData(bool valid = false);

	/**
	 * Creates a bufferdata object whose data is self-managed.
	 *
	 * Allocates @a size floats for its use that are deallocated when last
	 * reference dies. Allows use of readPointer() for easy integration with other
	 * systems.
	 *
	 * @param size The size of this BufferData object in elements.
	 * @param sampleSize The number of elements contained in each sample. This
	 * must be a divisor of @a size.
	 */
	BufferData(uint size, uint sampleSize = 1);

	/**
	 * Creates a read-only BufferData object whose data is foreign.
	 *
	 * Allows use of readPointer() for easy integration with other systems.
	 *
	 * @param data The databank which this BufferData will adopt. As you never
	 * want this array to be written to, declare the BufferData object const.
	 * @param size The size of this BufferData object in elements. This must
	 * not be larger than the size of the data given in @a data or memory
	 * corruption will occur.
	 * @param sampleSize The number of elements contained in each sample. This
	 * must be a divisor of @a size.
	 */
	BufferData(const float *data, uint size, uint sampleSize = 1);

	/** @overload
	 * Creates a writable BufferData object whose data is foreign.
	 *
	 * Allows use of readPointer() for easy integration with other systems.
	 *
	 * @param data The databank which this BufferData will adopt.
	 * @param size The size of this BufferData object in elements. This must
	 * not be larger than the size of the data given in @a data or memory
	 * corruption will occur.
	 * @param sampleSize The number of elements contained in each sample. This
	 * must be a divisor of @a size.
	 */
	BufferData(float *data, uint size, uint sampleSize = 1);

	/**
	 * Copy constructor. Acts like the assignment operator in that it adopts
	 * the same databank as the given BufferDatat @a source.
	 *
	 * Data changed in one may affect the other.
	 *
	 * @param source The BufferData object from which to adopt the data.
	 */
	BufferData(const BufferData &source);

	/**
	 * Intelligent destructor.
	 */
	~BufferData();
};

}
