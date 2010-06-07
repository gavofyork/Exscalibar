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

#include "bufferdata.h"
#include "buffer.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

BufferData *BufferData::theFake = 0;

BufferData &BufferData::fake()
{
	if (theFake) return *theFake;
	return *(theFake = new BufferData(true));
}

BufferData::BufferData(bool valid) : theWritePointer(0)
{
	theInfo = new BufferInfo(valid ? Undefined : 0, 0, valid ? new float[1] : 0, 0, BufferInfo::Ignore, 0, valid, valid ? BufferInfo::Write : BufferInfo::Read, valid ? BufferInfo::Managed : BufferInfo::Foreign, false);
	theOffset = 0;
	theVisibleSize = valid ? Undefined : 0;
}

BufferData::BufferData(uint size, uint sampleSize, float *data, ScratchOwner *scratch, BufferInfo::Legacy endType, uint offset, uint mask) : theWritePointer(0)
{
	theInfo = new BufferInfo(size, sampleSize, data ? data : new float[size], scratch, endType, mask, true, BufferInfo::Write, data ? BufferInfo::Foreign : BufferInfo::Managed, false);
	theOffset = offset;
	theVisibleSize = size;
}

BufferData::BufferData(uint size, uint sampleSize, float *data, ScreenOwner *screen, BufferInfo::Legacy endType, uint offset, uint mask) : theWritePointer(0)
{
	theInfo = new BufferInfo(size, sampleSize, data ? data : new float[size], screen, endType, mask, true, BufferInfo::Read, data ? BufferInfo::Foreign : BufferInfo::Managed, false);
	theOffset = offset;
	theVisibleSize = size;
}

BufferData::BufferData(uint size, uint sampleSize) : theWritePointer(0)
{
	theInfo = new BufferInfo(size, sampleSize, new float[size], 0, BufferInfo::Ignore, ~(uint)(0), true, BufferInfo::Write, BufferInfo::Managed, false);
	theVisibleSize = size;
	theOffset = 0;
}

BufferData::BufferData(const float *data, uint size, uint sampleSize) : theWritePointer(0)
{
	theInfo = new BufferInfo(size, sampleSize, (float *)data, 0, BufferInfo::Ignore, ~(uint)(0), true, BufferInfo::Read, BufferInfo::Foreign, false);
	theVisibleSize = size;
	theOffset = 0;
}

BufferData::BufferData(float *data, uint size, uint sampleSize) : theWritePointer(0)
{
	theInfo = new BufferInfo(size, sampleSize, data, 0, BufferInfo::Ignore, ~(uint)(0), true, BufferInfo::Write, BufferInfo::Foreign, false);
	theVisibleSize = size;
	theOffset = 0;
}

BufferData::BufferData(BufferInfo *info, uint offset): theInfo(info), theWritePointer(0)
{
	theInfo->reference();
	theVisibleSize = theInfo->theAccessibleSize;
	theOffset = offset;
}

BufferData::BufferData(const BufferData &source) : theWritePointer(0)
{
	theInfo = source.theInfo;
	theInfo->reference();
	theVisibleSize = source.theVisibleSize;
	theOffset = source.theOffset;
}

BufferData::~BufferData()
{
	if (theWritePointer)
		endWritePointer();
	theInfo->unreference(*this);
}

BufferData &BufferData::operator=(const BufferData &source)
{
	if (source.theInfo != theInfo)
	{
		if (theWritePointer)
			endWritePointer();
		theInfo->unreference(*this);
		theInfo = source.theInfo;
		theInfo->reference();
	}
	theVisibleSize = source.theVisibleSize;
	theOffset = source.theOffset;
	return *this;
}

ostream &operator<<(ostream &out, const BufferData &me)
{
	out << "[ ";
	if (me.theInfo->m_sampleSize == 1)
		for (uint i = 0; i < me.theVisibleSize; i++)
			out << me[i] << " ";
	else
		for (uint i = 0; i < me.theVisibleSize; i += me.theInfo->m_sampleSize)
		{	out << "( ";
			for (uint j = 0; j < me.theInfo->m_sampleSize; j++)
				out << me[i + j] << " ";
			out << ") ";
		}
	return out << "]";
}

bool BufferData::plunger() const
{
	return theInfo->thePlunger;
}

void BufferData::copyFrom(const float *source)
{
	if (isNull())
		return;
	if (rollsOver())
	{	memcpy(firstPart(), source, sizeFirstPart() * 4);
		memcpy(secondPart(), source + sizeFirstPart(), sizeSecondPart() * 4);
	}
	else
		memcpy(firstPart(), source, sizeOnlyPart() * 4);
}

void BufferData::copyTo(float *destination) const
{
	if (isNull())
		return;
	if (rollsOver())
	{	memcpy(destination, firstPart(), sizeFirstPart() * 4);
		memcpy(destination + sizeFirstPart(), secondPart(), sizeSecondPart() * 4);
	}
	else
		memcpy(destination, firstPart(), sizeOnlyPart() * 4);
}

void BufferData::copyFrom(const BufferData &data)
{
	if (isNull())
		return;
	if (rollsOver())
	{	if (data.rollsOver())
			if (sizeFirstPart() > data.sizeFirstPart())
			{	memcpy(firstPart(), data.firstPart(), data.sizeFirstPart() * 4);
				memcpy(firstPart() + data.sizeFirstPart(), data.secondPart(), (sizeFirstPart() - data.sizeFirstPart()) * 4);
				memcpy(secondPart(), data.secondPart() + sizeFirstPart() - data.sizeFirstPart(), sizeSecondPart() * 4);
			}
			else
			{	memcpy(firstPart(), data.firstPart(), sizeFirstPart() * 4);
				memcpy(secondPart(), data.firstPart() + sizeFirstPart(), (data.sizeFirstPart() - sizeFirstPart()) * 4);
				memcpy(secondPart() + data.sizeFirstPart() - sizeFirstPart(), data.secondPart(), data.sizeSecondPart() * 4);
			}
		else
		{	memcpy(firstPart(), data.firstPart(), sizeFirstPart() * 4);
			memcpy(secondPart(), data.firstPart() + sizeFirstPart(), sizeSecondPart() * 4);
		}
	}
	else
		if (data.rollsOver())
		{	memcpy(firstPart(), data.firstPart(), data.sizeFirstPart() * 4);
			memcpy(firstPart() + data.sizeFirstPart(), data.secondPart(), data.sizeSecondPart() * 4);
		}
		else
			memcpy(firstPart(), data.firstPart(), data.sizeOnlyPart() * 4);
}

BufferData &BufferData::dontRollOver(bool makeCopy)
{
	if (!isNull() && rollsOver())
	{	BufferData temp(*this);
		BufferData *ret = this;
		(*ret) = BufferData(theVisibleSize, theInfo->m_sampleSize);
		if (makeCopy) ret->copyFrom(temp);
	}
	return *this;
}

const BufferData &BufferData::dontRollOver(bool makeCopy) const
{
	if (!isNull() && rollsOver())
	{	const BufferData temp(*this);
		BufferData *ret = (BufferData *)this;
		(*ret) = BufferData(theVisibleSize, theInfo->m_sampleSize);
		if (makeCopy) ret->copyFrom(temp);
	}
	return *this;
}

const BufferData BufferData::sample(uint index) const
{
#ifdef EDEBUG
	assert(theInfo->m_sampleSize);
	assert(index < samples());
#endif
	return mid(index * theInfo->m_sampleSize, theInfo->m_sampleSize);
}

BufferData BufferData::sample(uint index)
{
#ifdef EDEBUG
	assert(theInfo->m_sampleSize);
	assert(index < samples());
#endif
	return mid(index * theInfo->m_sampleSize, theInfo->m_sampleSize);
}

const BufferData BufferData::samples(uint index, uint amount) const
{
#ifdef EDEBUG
	assert(theInfo->m_sampleSize);
	assert(index + amount <= samples());
#endif
	return mid(index * theInfo->m_sampleSize, theInfo->m_sampleSize * amount);
}

BufferData BufferData::samples(uint index, uint amount)
{
#ifdef EDEBUG
	assert(theInfo->m_sampleSize);
	assert(index + amount <= samples());
#endif
	return mid(index * theInfo->m_sampleSize, theInfo->m_sampleSize * amount);
}
/*
void BufferData::debugInfo() const
{
	qDebug("Elements/sampleSize/samples: %d/%d/%d", theSize, m_sampleSize, theSize / m_sampleSize);
	qDebug("Mask/Offset/Data: %d/%d/%p", theMask, theOffset, theData);
	qDebug("Valid/EndType: %d/%d", *theValid, (int)*theEndType);
	qDebug("Aux/Life/Type: %p/%d/%d", theAux, (int)theLife, (int)theType);
//	for (uint i = 0; i < theSize; i++)
//		qDebug("%d: %f", i, theData[(i + theOffset) & theMask]);
	if (*theValid)
	{	float min = theData[theOffset & theMask], max = theData[theOffset & theMask];
		for (uint i = 1; i < theSize; i++)
		{	float nw = theData[(i + theOffset) & theMask];
			min = ::min(min, nw);
			max = ::max(max, nw);
//			qDebug("off: %d, new: %f, min: %f, max: %f", i, nw, min, max);
		}
		qDebug("Min/Max: %f/%f", min, max);
	}
}
*/
const BufferData BufferData::mid(uint start, uint length) const
{
	BufferData ret = *this;
	if (!isNull())
	{
		if (theInfo->theMask == (uint)~0)
			// wrap around according to size as it's a custom buffer and mask is unavailable.
			// NOTE: the buffer is theAccessibleSize big (it does not just refer to the
			//       usable content (as it does with foreign Buffer theDatas)).
			ret.theOffset = (theOffset + start) % theInfo->theAccessibleSize;
		else
			ret.theOffset = (theOffset + start) & theInfo->theMask;	// normally should just wrap around according to mask
		ret.theVisibleSize = length;
	}
	return ret;
}

BufferData BufferData::mid(uint start, uint length)
{
	BufferData ret = *this;
	if (!isNull())
	{
		if (theInfo->theMask == (uint)~0)
			ret.theOffset = (theOffset + start) % theInfo->theAccessibleSize;
		else
			ret.theOffset = (theOffset + start) & theInfo->theMask;
		ret.theVisibleSize = length;
	}
	return ret;
}

}

#undef MESSAGES
