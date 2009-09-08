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

#include "bufferreader.h"
#include "lmconnection.h"
#include "mlconnection.h"
#include "buffer.h"
#include "processor.h"
using namespace Geddei;

namespace Geddei
{

MLConnection::MLConnection(Sink *sink, uint sinkIndex, LMConnection *connection)
	: xLConnection(sink, sinkIndex), theConnection(connection)
{
	theConnection->theConnections.append(this);
	theReader = new BufferReader(&(theConnection->theBuffer));
}

MLConnection::~MLConnection()
{
	delete theReader;
	theConnection->theConnections.removeAll(this);
}

void MLConnection::plungerSent()
{
	theSink->plungerSent(theSinkIndex);
}

void MLConnection::startPlungers()
{
	theSink->startPlungers();
}

void MLConnection::noMorePlungers()
{
	theSink->noMorePlungers();
}

uint MLConnection::capacity() const
{
	return theReader->size();
}

void MLConnection::killReader()
{
	delete theReader;
	theReader = 0;
}

void MLConnection::resurectReader()
{
	if (!theReader)
		theReader = new BufferReader(&(theConnection->theBuffer));
}

BufferReader *MLConnection::newReader()
{
	return new BufferReader(&(theConnection->theBuffer));
}

void MLConnection::enforceMinimum(uint elements)
{
	theConnection->enforceMinimum(elements);
}

uint MLConnection::elementsReady() const
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: elementsReady() cannot be called on an MLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return 0;
	}
#endif

	uint ret = theReader->elementsReady();
	return ret;
}

void MLConnection::waitForElements(uint elements) const
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: waitForElements() cannot be called on an MLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return;
	}
#endif

	theReader->waitForElements(elements);
	theSink->checkExit();
}

bool MLConnection::plungeSync(uint samples) const
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: plungeSync() cannot be called on an MLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return false;
	}
#endif

	BufferData ret = theReader->readElements(theType->scope() * samples, false);
	theSink->checkExit();
	if (ret.plunger())
	{
		theReader->haveRead(ret);
		theSink->plunged(theSinkIndex);
		return false;
	}
	return true;
}

const BufferData MLConnection::readElements(uint elements)
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: readElements() cannot be called on an MLConnection object after\n"
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

const BufferData MLConnection::peekElements(uint elements)
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: peekElements() cannot be called on an MLConnection object after\n"
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

void MLConnection::sinkStopping()
{
	theConnection->openBufferTrapdoor(dynamic_cast<Processor *>(theSink));
}

void MLConnection::sinkStopped()
{
	theConnection->closeBufferTrapdoor(dynamic_cast<Processor *>(theSink));
}

void MLConnection::reset()
{
}

void MLConnection::resetType()
{
	delete theType;
	theType = 0;
}

void MLConnection::setType(const SignalType *type)
{
	delete theType;
	theType = type->copy();
}

bool MLConnection::waitUntilReady()
{
//	return theSink->confirmTypes();
	return theSink->waitUntilReady();
}

const SignalTypeRef MLConnection::type()
{
	if (!theType) theType = (theConnection->type().thePtr ? theConnection->type().thePtr->copy() : 0);
	return SignalTypeRef(theType);
}

};

