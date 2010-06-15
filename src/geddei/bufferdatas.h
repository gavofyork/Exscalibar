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

#include "bufferdata.h"
#else
#include <geddei/bufferdata.h>
#endif

namespace Geddei
{

/** @ingroup Geddei
 * @brief Array-like class for a collection of BufferData objects.
 * @author Gav Wood <gav@kde.org>
 *
 * This class is a simple, array-like class but has extensions for holding
 * BufferData objects. This class will only be used publicly by SubProcessor
 * developers, the needs of which are very limited. It however needs extensive
 * utility methods for Geddei's internal use and as such most methods are for
 * internal use only and therefore undocumented.
 *
 * Don't let this worry you. Unless you're working on Geddei's core, you can
 * completely ignore them and you'll really only need to use the array
 * operator. The size() method has been left in too, just for fun.
 */
class DLLEXPORT BufferDatas
{
	friend class SubProcessor;
	uint theCount;
	const BufferData **theData;

public:
	/** @internal
	 * Sets the BufferData at index @a i to be a duplicate of @a d. This means
	 * that @a d must be deleted explicity by the caller. It is not adopted.
	 *
	 * In practise, the data will not actually be copied; the copy takes place
	 * in the sense that there are two seperate references to the same data, so
	 * you may happily delete @a d in your own time and be blissfully unaware
	 * about what has happened with this object.
	 *
	 * This should not need to be used in normal operation.
	 *
	 * @param i The index to be poplulated.
	 * @param d The BufferData object to be "duplicated".
	 */
	void copyData(uint i, const BufferData &d) { theData[i] = new BufferData(d); }

	/** @internal
	 * Sets the BufferData at index @a i to be a reference to the BufferData at
	 * @a d. The data at @a d is adopted into this object and it must not be
	 * explicity deleted. Doing so will cause memory corruption.
	 *
	 * This should not need to be used in normal operation.
	 *
	 * @param i The index to be poplulated.
	 * @param d The BufferData object to be adopted.
	 */
	void setData(uint i, const BufferData *d) { theData[i] = d; }

	/** @internal
	 * Get a subset of samples from the BufferData objects.
	 *
	 * This should not need to be used in normal operation.
	 *
	 * @return The array containing only a subset of samples from each of the
	 * BufferData objects
	 */
	const BufferDatas samples(uint index, uint amount) const;

	/** @internal @overload
	 * Get a subset of samples from the BufferData objects.
	 *
	 * This should not need to be used in normal operation.
	 *
	 * @return The array containing only a subset of samples from each of the
	 * BufferData objects
	 */
	BufferDatas samples(uint index, uint amount);

	inline BufferDatas samples(uint index) { if (!theCount) return *this; return samples(index, theData[0]->samples() - index); }
	inline BufferDatas leftSamples(uint amount) { if (!theCount) return *this; return samples(0, amount); }
	inline BufferDatas rightSamples(uint amount) { if (!theCount) return *this; return samples(theData[0]->samples() - amount, amount); }

	inline BufferDatas const samples(uint index) const { if (!theCount) return *this; return samples(index, theData[0]->samples() - index); }
	inline BufferDatas const leftSamples(uint amount) const { if (!theCount) return *this; return samples(0, amount); }
	inline BufferDatas const rightSamples(uint amount) const { if (!theCount) return *this; return samples(theData[0]->samples() - amount, amount); }

	/** @internal
	 * Discards all BufferData objects from the array and resizes it to
	 * @a count.
	 *
	 * This should not need to be used in normal operation.
	 *
	 * @param count The new size of this BufferDatas object.
	 */
	void resize(uint count);

	/** @internal
	 * Discards all BufferData objects from the array.
	 *
	 * This should not need to be used in normal operation.
	 */
	void nullify();

	/** @internal
	 */
	bool isNull() const { return !theCount || !theData[0]; }

	/**
	 * Get a single BufferData from the array.
	 *
	 * @param i The index to retrieve the BufferData object from.
	 * @return The BufferData object at position @a i .
	 */
	const BufferData &operator[](uint i) const
	{
#ifdef EDEBUG
		assert(i < theCount);
		assert(theData[i]);
#endif
		return *((const BufferData *)theData[i]);
	}

	/** @overload
	 * Get a single BufferData from the array.
	 *
	 * For notice of internal development:
	 * Don't be tempted to use this method to set the contents BufferDatas.
	 * Use setData() instead. It is only non-for access to non-const
	 * methods of the returned BufferData. BufferDatas itself should be left
	 * unchanged.
	 *
	 * @param i The index to retrieve the BufferData object from.
	 * @return The BufferData object at position @a i .
	 */
	BufferData &operator[](uint i)
	{
#ifdef EDEBUG
		assert(i < theCount);
		assert(theData[i]);
#endif
		return *((BufferData *)theData[i]);
	}

	/**
	 * Get the number of BufferDatas objects stored.
	 *
	 * @return The number of slots for BufferData objects.
	 */
	uint size() const { return theCount; }

	/**
	 * Get the number of BufferDatas objects stored.
	 *
	 * @return The number of slots for BufferData objects.
	 */
	uint count() const { return theCount; }

	/** @internal
	 * Assignment operator.
	 *
	 * Creates a new "array" of BufferData objects copying those provided. Any
	 * objects currently held are discarded.
	 *
	 * @note BufferData's data is implicitly shared, so changes in one will
	 * affect the other.
	 *
	 * @param src The object whose data will be copied from.
	 * @return A reference to this object.
	 */
	BufferDatas &operator=(const BufferDatas &src);

	/** @internal
	 * Copy constructor.
	 *
	 * Creates a new "array" that holds copies of the BufferData objects.
	 *
	 * @note BufferData's data is implicitly shared, so changes in one will
	 * affect the other.
	 *
	 * @param src The object whose data will be copied from.
	 */
	BufferDatas(const BufferDatas &src);

	/** @internal
	 * Constructs a new empty array for holding @a count BufferDatas.
	 *
	 * @param count The number of slots for BufferData objects this object
	 * should have.
	 */
	BufferDatas(uint count = 0);

	/** @internal
	 * Default destructor.
	 */
	~BufferDatas();
};

}
