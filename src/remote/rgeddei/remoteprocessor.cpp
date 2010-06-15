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

	if (!theSession->newProcessor(type, theHandle))
	{	qWarning("*** WARNING: RemoteProcessor: Attempt to create Processor of type %s failed.", qPrintable(type));
		return;
	}
	theType = type;
	theGroup = 0;
}

RemoteProcessor::~RemoteProcessor()
{
	setNoGroup();
	if (isValid())
		theSession->deleteProcessor(theHandle);
}

void RemoteProcessor::doInit(const QString &name, AbstractProcessorGroup *g, const Properties &p)
{
	theName = name;
	if (g) setGroup(*g);
	theSession->processorInit(theHandle, p, theName);
	theHandle = theName;
}

bool RemoteProcessor::connect(uint sourceIndex, const LocalProcessor *sink, uint sinkIndex, uint bufferSize)
{
	return theSession->processorConnect(theHandle, bufferSize, sourceIndex, sink->theSession->theHost, sink->theSession->thePort, qPrintable(sink->theProcessor->name()), sinkIndex);
}

bool RemoteProcessor::connect(uint sourceIndex, const RemoteProcessor *sink, uint sinkIndex, uint bufferSize)
{
	if (MESSAGES) qDebug("RemoteProcessor::connect(): key=%d, host=%s, processor=%s", sink->theSession->theKey, qPrintable(sink->theSession->theHost), qPrintable(sink->theHandle));
	if (sink->theSession != theSession)
		return theSession->processorConnect(theHandle, bufferSize, sourceIndex, sink->theSession->theHost, sink->theSession->theKey, sink->theHandle, sinkIndex);
	else
		return theSession->processorConnect(theHandle, bufferSize, sourceIndex, sink->theHandle, sinkIndex);
}

}
