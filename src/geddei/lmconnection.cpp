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

#include "lxconnectionreal.h"
#include "buffer.h"
#include "bufferdata.h"
#include "processor.h"
#include "mlconnection.h"
#include "lmconnection.h"
//Added by qt3to4:
#include <Q3PtrList>
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

LMConnection::LMConnection(Source *source, uint sourceIndex, uint bufferSize)
	: LxConnectionReal(source, sourceIndex), theBuffer(bufferSize)
{
	// we want to own the outputs.
	theConnections.setAutoDelete(true);
}

LMConnection::~LMConnection()
{
	theConnections.clear();
}

void LMConnection::enforceMinimum(uint elements)
{
	if (theBuffer.size() < elements)
		theBuffer.resize(elements);
}

bool LMConnection::waitUntilReady()
{
	for (Q3PtrList<MLConnection>::Iterator i = theConnections.begin(); i != theConnections.end(); i++)
		if (!(*i)->waitUntilReady()) return false;
	return true;
}

void LMConnection::reset()
{
	theBuffer.clear();
}

const SignalTypeRef LMConnection::type()
{
	if (!theType) theSource->confirmTypes();
	return SignalTypeRef(theType);
}

void LMConnection::setType(const SignalType *type)
{
	delete theType;
	theType = type->copy();
	for (Q3PtrList<MLConnection>::Iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->setType(type);
	theBuffer.setType(theType);
}

void LMConnection::resetType()
{
	delete theType;
	theType = 0;
	for (Q3PtrList<MLConnection>::Iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->resetType();

	// TODO: Reset the type in the buffer too really...
	theBuffer.clear();
//	theBuffer.reset();
}

void LMConnection::bufferWaitForFree()
{
	theBuffer.waitForFreeElements(1);
	theSource->checkExit();
}

uint LMConnection::bufferElementsFree()
{
	return theBuffer.elementsFree();
}

uint LMConnection::maximumScratchElementsEver()
{
	return theBuffer.size();
}

uint LMConnection::maximumScratchElements(uint minimum)
{
	while (bufferElementsFree() < minimum)
		bufferWaitForFree();
	return bufferElementsFree();
}

BufferData LMConnection::makeScratchElements(uint elements, bool autoPush)
{
	BufferData ret;
	if (theBuffer.size() >= elements)
		ret = theBuffer.makeScratchElements(elements, autoPush);
	else
		ret = LxConnection::makeScratchElements(elements, autoPush);
	theSource->checkExit();
	assert(ret.isValid());
	return ret;
}

void LMConnection::startPlungers()
{
	for (Q3PtrList<MLConnection>::Iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->startPlungers();
}

void LMConnection::plungerSent()
{
	if (MESSAGES) qDebug("> LMConnection::plungerSent()");
	for (Q3PtrList<MLConnection>::Iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->plungerSent();
	if (MESSAGES) qDebug("< LMConnection::plungerSent()");
}

void LMConnection::pushPlunger()
{
	if (MESSAGES) qDebug("> LMConnection::pushPlunger()");
	theBuffer.appendPlunger();
	if (MESSAGES) qDebug("< LMConnection::pushPlunger()");
}

void LMConnection::noMorePlungers()
{
	for (Q3PtrList<MLConnection>::Iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->noMorePlungers();
}

void LMConnection::transport(const BufferData &data)
{
	// TODO: some error checking...
	theBuffer.push(data);
	theSource->checkExit();
}

void LMConnection::sourceStopping()
{
	theBuffer.openTrapdoor(theSource->processor());
}

void LMConnection::sourceStopped()
{
	theBuffer.closeTrapdoor(theSource->processor());
}

}

#undef MESSAGES
