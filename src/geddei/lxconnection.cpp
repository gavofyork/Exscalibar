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

#include <cassert>
using namespace std;

#include "processor.h"
#include "bufferdata.h"
#include "lxconnection.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

LxConnection::LxConnection(Source *source, uint sourceIndex) : theSource(source), theSourceIndex(sourceIndex)
{
	theSource->doRegisterOut(this, sourceIndex);

	theScratch = NULL;
	theScratchSize = 0;
}

LxConnection::~LxConnection()
{
	if(theScratch) delete [] theScratch;

	theSource->undoRegisterOut(this, theSourceIndex);
}

BufferData LxConnection::makeScratchSamples(uint samples, bool autoPush)
{
	return makeScratchElements(theType->elementsFromSamples(samples), autoPush);
}

BufferData LxConnection::makeScratchSeconds(float seconds, bool autoPush)
{
	return makeScratchElements(theType->elementsFromSeconds(seconds), autoPush);
}

BufferData LxConnection::makeScratchElements(uint elements, bool autoPush)
{
	if(theScratchSize != elements)
	{	if(theScratch) delete [] theScratch;
		theScratch = new float[elements];
		theScratchSize = elements;
	}
	const BufferData &ret = BufferData(elements, theType->scope(), theScratch, dynamic_cast<ScratchOwner *>(this), autoPush ? BufferInfo::Activate : BufferInfo::Forget);
	lastScratch = ret.identity();
	return ret;
}

void LxConnection::forgetScratch(const BufferData &data)
{
	assert(lastScratch == data.identity());
	lastScratch = 0;
	data.ignoreDeath();
}

void LxConnection::pushScratch(const BufferData &data)
{
	assert(lastScratch == data.identity());
	data.ignoreDeath();
	push(data);
	data.invalidate();
}

};
