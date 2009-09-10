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

#include "subprocessorfactory.h"
using namespace Geddei;

#include "localsession.h"
#include "localprocessor.h"
#include "remoteprocessor.h"
#include "remotedomprocessor.h"
using namespace rGeddei;

#define MESSAGES 0

namespace rGeddei
{

RemoteDomProcessor::RemoteDomProcessor(RemoteSession &session, const QString &type) : RemoteProcessor(session)
{
	if (!theSession->newDomProcessor(type, theHandle))
	{	qWarning("*** WARNING: RemoteProcessor: Attempt to create DomProcessor of subtype %s failed.", qPrintable(type));
		return;
	}
	// Any accesses to type must be virtual, since it will change its meaning depending upon
	// whether the class is RemoteDomProcessor or just RemoteProcessor.
	theType = type;
	theGroup = 0;
}

RemoteDomProcessor::~RemoteDomProcessor()
{
	theSession->deleteDomProcessor(theHandle);
	// Need to invalidate it so that RemoteProcessor doesn't delete it too!
	theType = "";
}

bool RemoteDomProcessor::spawnWorker(RemoteSession &session)
{
	if (MESSAGES) qDebug("RemoteDomProcessor::spawnWorker(Node)");
	if (theSession == &session)
		return theSession->domProcessorCreateAndAdd(theHandle);
	else
		return theSession->domProcessorCreateAndAdd(theHandle, session.theHost, session.theKey);
}

bool RemoteDomProcessor::spawnWorker(LocalSession &session)
{
	if (MESSAGES) qDebug("RemoteDomProcessor::spawnWorker(Local)");
	return theSession->domProcessorCreateAndAdd(theHandle, session.theHost, session.thePort);
}

};
