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

#include "bufferdata.h"
#include "bufferdatas.h"

namespace Geddei
{

BufferDatas &BufferDatas::operator=(const BufferDatas &src)
{
	for (uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
	theCount = src.theCount;
	theData = new const BufferData *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = new BufferData(*(src.theData[i]));
	return *this;
}

BufferDatas::BufferDatas(const BufferDatas &src)
{
	theCount = src.theCount;
	theData = new const BufferData *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = src.theData[i] ? new BufferData(*(src.theData[i])) : 0;
}

BufferDatas::BufferDatas(uint count) : theCount(count)
{
	theData = new const BufferData *[count];
	for (uint i = 0; i < theCount; i++)
		theData[i] = 0;
}

BufferDatas::~BufferDatas()
{
	for (uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
}

void BufferDatas::resize(uint count)
{
	for (uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
	theCount = count;
	theData = new const BufferData *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = 0;
}

const BufferDatas BufferDatas::samples(uint index, uint amount) const
{
	BufferDatas ret(theCount);
	for (uint i = 0; i < theCount; i++)
		ret.copyData(i, theData[i]->samples(index, amount));
	return ret;
}

BufferDatas BufferDatas::samples(uint index, uint amount)
{
	BufferDatas ret(theCount);
	for (uint i = 0; i < theCount; i++)
		ret.copyData(i, theData[i]->samples(index, amount));
	return ret;
}

void BufferDatas::nullify()
{
	for (uint i = 0; i < theCount; i++)
	{	delete theData[i];
		theData[i] = 0;
	}
}

}

#undef MESSAGES

