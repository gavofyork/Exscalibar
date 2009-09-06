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

#include "processor.h"
#include "multiprocessor.h"
#include "processorport.h"

namespace Geddei
{

void ProcessorPort::connect(const ProcessorPort &input) const
{
	theParent->connect(thePort, input.theParent, input.thePort, input.theBufferSize);
}

void ProcessorPort::connect(MultiProcessor &input) const
{
	share();
	for (uint i = 0; i < input.multiplicity(); i++)
		connect(input.sourcePort(i));
}

void ProcessorPort::disconnect() const
{
	theParent->disconnect(thePort);
}

void ProcessorPort::share() const
{
	theParent->share(thePort, theBufferSize);
}

void ProcessorPort::split() const
{
	theParent->split(thePort);
}

ProcessorPort &ProcessorPort::setSize(uint bufferSize)
{
	theBufferSize = bufferSize;
	return *this;
}

}
