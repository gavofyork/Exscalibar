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

#include "bufferreader.h"
#include "processor.h"
#include "bufferdata.h"
#include "xlconnectionreal.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

xLConnectionReal::xLConnectionReal(Sink *newSink, uint newSinkIndex, uint bufferSize)
	: xLConnection(newSink, newSinkIndex), theBuffer(bufferSize)
{
	theReader = new BufferReader(&theBuffer);
}

xLConnectionReal::~xLConnectionReal()
{
	delete theReader;
}

void xLConnectionReal::killReader()
{
	delete theReader;
	theReader = 0;
}

void xLConnectionReal::resurectReader()
{
	if (!theReader)
		theReader = new BufferReader(&theBuffer);
}

uint xLConnectionReal::elementsReady() const
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: elementsReady() cannot be called on a xLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return 0;
	}
#endif
	
	uint ret = theReader->elementsReady();
	return ret;
}

void xLConnectionReal::waitForElements(uint elements) const
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: waitForElements() cannot be called on a xLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return;
	}
#endif
	
	theReader->waitForElements(elements);
/*	while (theReader->elementsReady() < elements)
	{
		// Plunge
		theReader->skipElements(elementsReady());
		theReader->skipPlunger();
		theSink->plunged(theSinkIndex);
		
		// And wait for some more
		theReader->waitForElements(elements);
	}
*/	theSink->checkExit();
}

bool xLConnectionReal::plungeSync(uint samples) const
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: plungeSync() cannot be called on a xLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return true;
	}
#endif

	if (MESSAGES) qDebug("xLC::plungeSync(%d): Peeking %d elements...", samples, theType->scope() * samples);
	BufferData ret = theReader->readElements(theType->scope() * samples, false);
	theSink->checkExit();
	if (ret.plunger())
	{
		if (MESSAGES) qDebug("xLC: Plunger found. Discarding peeked elements...");
		theReader->haveRead(ret);
		if (MESSAGES) qDebug("xLC: Plunging sink...");
		theSink->plunged(theSinkIndex);
		return false;
	}
	if (MESSAGES) qDebug("xLC: Plunger not found; read must have succeeded.");
	return true;
}

void xLConnectionReal::enforceMinimum(uint elements)
{
	if (theBuffer.size() < elements)
		theBuffer.resize(elements);
}

const BufferData xLConnectionReal::readElements(uint elements)
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: readElements() cannot be called on a xLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return BufferData();
	}
#endif
	
	while (1)
	{	BufferData ret = theReader->readElements(elements, true);
		theSink->checkExit();
		if (!ret.plunger()) return ret;
		// This is a workaround for a buggy gcc (3.2.2).
		// If it wasn't here stuff wouldn't get freed up in the buffer.
		// As it is, there are deallocation problems, since the last instance of ret
		// will never get destroyed.
		ret.nullify();
		theSink->plunged(theSinkIndex);
	}
}

const BufferData xLConnectionReal::peekElements(uint elements)
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: peekElements() cannot be called on a xLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return BufferData();
	}
#endif
	
	while (1)
	{	BufferData ret = theReader->readElements(elements, false);
		theSink->checkExit();
		if (!ret.plunger()) return ret;
		theReader->haveRead(ret);
		theSink->plunged(theSinkIndex);
	}
}

void xLConnectionReal::sinkStopping()
{
	if (MESSAGES) qDebug("xLConnectionReal::sinkStopping(): theReader=%p", theReader);
	theBuffer.openTrapdoor(dynamic_cast<Processor *>(theSink));
}

void xLConnectionReal::sinkStopped()
{
	if (MESSAGES) qDebug("xLConnectionReal::sinkStopped(): theReader=%p", theReader);
	theBuffer.closeTrapdoor(dynamic_cast<Processor *>(theSink));
}

const SignalTypeRef xLConnectionReal::type()
{
	if (!theType) pullType();
	return SignalTypeRef(theType);
}

};
