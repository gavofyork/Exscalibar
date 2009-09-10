/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
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
#include "xlconnection.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

xLConnection::xLConnection(Sink *newSink, uint newSinkIndex) : Connection()
{
	theSink = newSink;
	theSinkIndex = newSinkIndex;
	theSink->doRegisterIn(this, theSinkIndex);
	theSink->resetTypes();
}

xLConnection::~xLConnection()
{
	// If theSink has already been set to zero, it means that we have already reset the
	// processor's port, and that there's no need to again.
	if (theSink)
	{
//		if (MESSAGES) qDebug("Deleting input link on %s...", theSink->name().toLatin1());
		theSink->undoRegisterIn(this, theSinkIndex);
	}
}

}

#undef MESSAGES
