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

#include "remoteprocessor.h"
#include "localprocessor.h"
#include "abstractprocessor.h"
using namespace rGeddei;

namespace rGeddei
{

bool AbstractProcessorPort::split()
{
	theParent->split(thePort);
	return true;
}

bool AbstractProcessorPort::share()
{
	theParent->share(thePort);
	return true;
}

bool AbstractProcessorPort::connect(const AbstractProcessorPort &input)
{
	if(dynamic_cast<RemoteProcessor *>(input.theParent))
		return theParent->connect(thePort, dynamic_cast<RemoteProcessor *>(input.theParent), input.thePort, input.theBufferSize);
	else if(dynamic_cast<LocalProcessor *>(input.theParent))
		return theParent->connect(thePort, dynamic_cast<LocalProcessor *>(input.theParent), input.thePort, input.theBufferSize);
	else
		return false;
}

bool AbstractProcessorPort::disconnect()
{
	theParent->disconnect(thePort);
	return true;
}

}
