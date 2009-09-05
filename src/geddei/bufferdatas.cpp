/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include "bufferdata.h"
#include "bufferdatas.h"

namespace Geddei
{

BufferDatas &BufferDatas::operator=(const BufferDatas &src)
{
	for(uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
	theCount = src.theCount;
	theData = new const BufferData *[theCount];
	for(uint i = 0; i < theCount; i++)
		theData[i] = new BufferData(*(src.theData[i]));
	return *this;
}

BufferDatas::BufferDatas(const BufferDatas &src)
{
	theCount = src.theCount;
	theData = new const BufferData *[theCount];
	for(uint i = 0; i < theCount; i++)
		theData[i] = src.theData[i] ? new BufferData(*(src.theData[i])) : 0;
}

BufferDatas::BufferDatas(uint count) : theCount(count)
{
	theData = new const BufferData *[count];
	for(uint i = 0; i < theCount; i++)
		theData[i] = 0;
}

BufferDatas::~BufferDatas()
{
	for(uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
}

void BufferDatas::resize(uint count)
{
	for(uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
	theCount = count;
	theData = new const BufferData *[theCount];
	for(uint i = 0; i < theCount; i++)
		theData[i] = 0;
}

const BufferDatas BufferDatas::samples(uint index, uint amount) const
{
	BufferDatas ret(theCount);
	for(uint i = 0; i < theCount; i++)
		ret.copyData(i, theData[i]->samples(index, amount));
	return ret;
}

BufferDatas BufferDatas::samples(uint index, uint amount)
{
	BufferDatas ret(theCount);
	for(uint i = 0; i < theCount; i++)
		ret.copyData(i, theData[i]->samples(index, amount));
	return ret;
}

void BufferDatas::nullify()
{
	for(uint i = 0; i < theCount; i++)
	{	delete theData[i];
		theData[i] = 0;
	}
}

};
