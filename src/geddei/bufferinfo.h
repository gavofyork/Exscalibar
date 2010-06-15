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

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "qcounter.h"
#else
#include <qtextra/qcounter.h>
#endif

namespace Geddei
{

class Auxilliary;
class BufferData;

/** @internal @ingroup Geddei
 * @brief The real guts of a BufferData object.
 * @author Gav Wood <gav@kde.org>
 *
 */
class BufferInfo
{
	friend std::ostream &operator<<(std::ostream &out, const BufferData &me);

public:
	enum Access { Read, Write };
	enum Payload { Managed, Foreign };
	enum Legacy { Ignore = 0, Forget, Activate };

private:
	// Reference counter
	QCounter theCounter;

	// true if we are indestructible since we are a resident BufferInfo of a Buffer or BufferReader.
	bool theResident;

	void retire(BufferData &client);
	void destruct(BufferData &client);

public: //TODO make private:
	// Info about the data - these never change in the lifetime of the data
	uint theMask, theAccessibleSize;
	uint m_sampleSize;
	mutable Auxilliary *theAux;
	bool theValid;
	Access theType;
	Payload theLife;
	Legacy theEndType;

	// The data itself
	float *theData;
	bool thePlunger;

public:

	/**
	 * Check to see if the BufferInfo object is still actually referencing a
	 * part of a Buffer, and that it could ever possibly alter the buffer.
	 *
	 * If this object isn't valid, then it is not allowed to effect the Buffer.
	 *
	 * If this object is Foreign, then there is no Buffer associated with it to
	 * alter.
	 *
	 * If this object has no references then there is no way to effect it to
	 * alter the Buffer.
	 *
	 * @return true if this object is still a live portion of a Buffer.
	 */
	bool isLive() const
	{
		bool ret = (theLife == Foreign);
#ifdef EDEBUG
		if (ret) assert(theValid);
#endif
		return ret;
	}

	/**
	 * Check to see if the BufferInfo object is still actually referencing a
	 * part of a Buffer, and that it could ever possibly alter the buffer.
	 *
	 * If this object isn't valid, then it is not allowed to effect the Buffer.
	 *
	 * If this object is Foreign, then there is no Buffer associated with it to
	 * alter.
	 *
	 * If this object has no references then there is no way to effect it to
	 * alter the Buffer.
	 *
	 * @return true if this object is still a live portion of a Buffer.
	 */
	bool isActive() const
	{
		bool ret = (theLife == Foreign && theCounter > 0);
#ifdef EDEBUG
		if (ret) assert(theValid);
#endif
		return ret;
	}

	bool isReferenced() const { return theCounter > 0; }
	void reference();
	void unreference(BufferData &client);

	/**
	 * Change this object so that it will no longer do any automatic action on
	 * destruction.
	 *
	 * It doesn't change whether or not it gets destroyed (theInternal).
	 */
	void ignore() { theEndType = Ignore; }

	/**
	 * Change this object so that it will no longer do any automatic action on
	 * destruction and using it is not allowed.
	 *
	 * It should only be called on foreign objects (i.e. objects whose data is
	 * not managed internally).
	 *
	 * It doesn't change whether or not it gets destroyed (theInternal).
	 */
	void invalidateAndIgnore()
	{
#ifdef EDEBUG
		assert(theLife == Foreign);
#endif
		theEndType = Ignore;
		theValid = false;
	}

	/**
	 * To be called on internal objects (to a Buffer/BufferReader).
	 *
	 * This object becomes invalid and will be automatically destroyed after
	 * the last refering BufferData is destroyed (or reassigned).
	 *
	 * TODO: Make it automatically carry out the end action also.
	 */
	void jettison();

	BufferInfo(float *data, Auxilliary *aux, uint mask, const Payload life, const Access type):
		theCounter(0), theResident(true), theMask(mask), theAux(aux), theType(type), theLife(life), theData(data) {}
	BufferInfo(uint accessibleSize, uint sampleSize, float *data, Auxilliary *aux, const Legacy endType,
		uint mask, bool valid, const Access type, const Payload life, bool plunger):
		theCounter(1), theResident(false), theMask(mask), theAccessibleSize(accessibleSize), m_sampleSize(sampleSize), theAux(aux),
		theValid(valid), theType(type), theLife(life), theEndType(endType), theData(data), thePlunger(plunger) {}
};

}
