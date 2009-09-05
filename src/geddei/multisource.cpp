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

#include "multisink.h"
#include "multisource.h"
#include "processor.h"

#define MESSAGES 0

namespace Geddei
{

void MultiSource::disconnect()
{
	for(uint i = 0; i < multiplicity(); i++)
		sourcePort(i) --;
}

bool MultiSource::deferConnect(MultiSink *sink, uint bufferSize)
{
	// If our multiplicity is explicitly defined
	if(knowMultiplicity())
		// If the sink's multiplicity is explicitly defined
		if(sink->knowMultiplicity())
		{	if(multiplicity() != sink->multiplicity())
			{	qWarning("*** MultiSource::deferConnect(): Multiplicity incompatibility: Attempting to connect %d outputs to %d inputs!", multiplicity(), sink->multiplicity());
				if(dynamic_cast<Processor *>(this)) qWarning("                                 Name of source: %s", dynamic_cast<Processor *>(this)->name().latin1());
				if(dynamic_cast<Processor *>(sink)) qWarning("                                 Name of sink:   %s", dynamic_cast<Processor *>(sink)->name().latin1());
				// TODO: error reporting code.
				return true;
			}
		}
		else
			sink->setMultiplicity(multiplicity());
	else
		if(sink->knowMultiplicity())
			setMultiplicity(sink->multiplicity());
		else
		{	theDeferredConnect = true;
			theDeferredBufferSize = bufferSize;
			theDeferredSink = sink;
			sink->appendDeferral(this);
			return true;
		}
	return false;
}

void MultiSource::setSourceMultiplicity(uint multiplicity)
{
	if(MESSAGES) qDebug("MultiSource::setSourceMultiplicity(%d) DC=%d", multiplicity, theDeferredConnect);
	if(theDeferredConnect)
	{	if(MESSAGES) qDebug("Deferred connect. Connecting...");
		connect(theDeferredSink, theDeferredBufferSize);
	}
}

void MultiSource::connect(MultiSink *sink, uint bufferSize)
{
	// TODO: Warn & exit.
	assert(!theConnected);

	if(deferConnect(sink, bufferSize)) return;

	assert(sink->knowMultiplicity());
	assert(knowMultiplicity());
	connectCheck();

	if(MESSAGES) qDebug("MultiSource::connect(): %d -> %d", multiplicity(), sink->multiplicity());

	if(sink->multiplicity() != multiplicity())
		qFatal("MultiProcessor: Error in connecting to sink.");

	for(uint i = 0; i < multiplicity(); i++)
		sourcePort(i) >>= sink->sinkPort(i);
	theDeferredConnect = false;
	sink->removeDeferral(this);
}

};
