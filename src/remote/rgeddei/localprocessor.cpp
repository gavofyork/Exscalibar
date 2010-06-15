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

#include "localprocessor.h"

#include "processor.h"
#include "processorfactory.h"
using namespace Geddei;

#include "remoteprocessor.h"
#include "abstractprocessorgroup.h"
#include "localsession.h"
using namespace rGeddei;

#define MESSAGES 0

namespace rGeddei
{

LocalProcessor::LocalProcessor(LocalSession &session, Processor &processor)
{
	theSession = &session;
	theProcessor = &processor;
	theAdopted = false;
}

LocalProcessor::LocalProcessor(LocalSession &session, Processor *processor)
{
	theSession = &session;
	theProcessor = processor;
	theAdopted = true;
}

LocalProcessor::LocalProcessor(LocalSession &session, const QString &type)
{
	theSession = &session;
	theProcessor = ProcessorFactory::create(type);
	assert(theProcessor);
	theAdopted = true;
}

LocalProcessor::~LocalProcessor()
{
	if (MESSAGES) qDebug("> LocalProcessor::~LocalProcessor(): Setting no group...");
	setNoGroup();
	if (MESSAGES) qDebug("= LocalProcessor::~LocalProcessor(): Checking if adopted...");
	if (theAdopted)
	{
		if (MESSAGES) qDebug("= LocalProcessor::~LocalProcessor(): Deleting Processor object...");
		delete theProcessor;
	}
	if (MESSAGES) qDebug("< LocalProcessor::~LocalProcessor()");
}

void LocalProcessor::doInit(const QString &name, AbstractProcessorGroup *g, const Properties &p)
{
	theProcessor->init(name, *theSession, p);
	if (g) setGroup(*g);
}

bool LocalProcessor::connect(uint sourceIndex, const LocalProcessor *sink, uint sinkIndex, uint bufferSize)
{
	if (MESSAGES) qDebug("LocalProcessor::connect(): port=%d, host=%s, processor=%s", sink->theSession->thePort, qPrintable(sink->theSession->theHost), qPrintable(sink->theProcessor->name()));
	if (theSession != sink->theSession)
		return theProcessor->connect(sourceIndex, sink->theSession->theHost, sink->theSession->thePort, qPrintable(sink->theProcessor->name()), sinkIndex, bufferSize);
	else
		return theProcessor->connect(sourceIndex, sink->theProcessor, sinkIndex, bufferSize);
}

bool LocalProcessor::connect(uint sourceIndex, const RemoteProcessor *sink, uint sinkIndex, uint bufferSize)
{
	if (MESSAGES) qDebug("LocalProcessor::connect(): key=%d, host=%s, processor=%s", sink->theSession->theKey, qPrintable(sink->theSession->theHost), qPrintable(sink->theName));
	return theProcessor->connect(sourceIndex, sink->theSession->theHost, sink->theSession->theKey, sink->theHandle, sinkIndex, bufferSize);
}

void LocalProcessor::disconnect(uint sourceIndex)
{
	theProcessor->disconnect(sourceIndex);
}

void LocalProcessor::disconnectAll()
{
	theProcessor->disconnectAll();
}

}
