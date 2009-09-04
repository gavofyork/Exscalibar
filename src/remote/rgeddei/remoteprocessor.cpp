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

#include "processorfactory.h"
using namespace Geddei;

#include "remotesession.h"
#include "localprocessor.h"
#include "localsession.h"
#include "remoteprocessor.h"
#include "abstractprocessorgroup.h"
using namespace rGeddei;

#define MESSAGES 0

namespace rGeddei
{

RemoteProcessor::RemoteProcessor(RemoteSession &session)
{
	theSession = &session;
	theType = "";
	theHandle = theSession->makeUniqueProcessorName();
}

RemoteProcessor::RemoteProcessor(RemoteSession &session, const QString &type)
{
	theSession = &session;
	theType = "";
	theHandle = theSession->makeUniqueProcessorName();

	if(!theSession->newProcessor(type, theHandle))
	{	qWarning("*** WARNING: RemoteProcessor: Attempt to create Processor of type %s failed.", type.latin1());
		return;
	}
	theType = type;
	theGroup = 0;
}

RemoteProcessor::~RemoteProcessor()
{
	setNoGroup();
	if(isValid())
		theSession->deleteProcessor(theHandle);
}

void RemoteProcessor::doInit(const QString &name, AbstractProcessorGroup *g, const Properties &p)
{
	theName = name;
	if(g) setGroup(*g);
	theSession->processorInit(theHandle, p, theName);
	theHandle = theName;
}

const bool RemoteProcessor::connect(const uint sourceIndex, const LocalProcessor *sink, const uint sinkIndex, const uint bufferSize)
{
	return theSession->processorConnect(theHandle, bufferSize, sourceIndex, sink->theSession->theHost, sink->theSession->thePort, sink->theProcessor->name().latin1(), sinkIndex);
}

const bool RemoteProcessor::connect(const uint sourceIndex, const RemoteProcessor *sink, const uint sinkIndex, const uint bufferSize)
{
	if(MESSAGES) qDebug("RemoteProcessor::connect(): key=%d, host=%s, processor=%s", sink->theSession->theKey, sink->theSession->theHost.latin1(), sink->theHandle.latin1());
	if(sink->theSession != theSession)
		return theSession->processorConnect(theHandle, bufferSize, sourceIndex, sink->theSession->theHost, sink->theSession->theKey, sink->theHandle, sinkIndex);
	else
		return theSession->processorConnect(theHandle, bufferSize, sourceIndex, sink->theHandle, sinkIndex);
}

};
