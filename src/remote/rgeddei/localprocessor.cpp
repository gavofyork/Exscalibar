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
	if(MESSAGES) qDebug("> LocalProcessor::~LocalProcessor(): Setting no group...");
	setNoGroup();
	if(MESSAGES) qDebug("= LocalProcessor::~LocalProcessor(): Checking if adopted...");
	if(theAdopted)
	{
		if(MESSAGES) qDebug("= LocalProcessor::~LocalProcessor(): Deleting Processor object...");
		delete theProcessor;
	}
	if(MESSAGES) qDebug("< LocalProcessor::~LocalProcessor()");
}

void LocalProcessor::doInit(const QString &name, AbstractProcessorGroup *g, const Properties &p)
{
	theProcessor->init(name, *theSession, p);
	if(g) setGroup(*g);
}

bool LocalProcessor::connect(uint sourceIndex, const LocalProcessor *sink, uint sinkIndex, uint bufferSize)
{
	if(MESSAGES) qDebug("LocalProcessor::connect(): port=%d, host=%s, processor=%s", sink->theSession->thePort, sink->theSession->theHost.latin1(), sink->theProcessor->name().latin1());
	if(theSession != sink->theSession)
		return theProcessor->connect(sourceIndex, sink->theSession->theHost, sink->theSession->thePort, sink->theProcessor->name().latin1(), sinkIndex, bufferSize);
	else
		return theProcessor->connect(sourceIndex, sink->theProcessor, sinkIndex, bufferSize);
}

bool LocalProcessor::connect(uint sourceIndex, const RemoteProcessor *sink, uint sinkIndex, uint bufferSize)
{
	if(MESSAGES) qDebug("LocalProcessor::connect(): key=%d, host=%s, processor=%s", sink->theSession->theKey, sink->theSession->theHost.latin1(), sink->theName.latin1());
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

};
