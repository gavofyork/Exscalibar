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

}
