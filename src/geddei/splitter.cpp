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
//Added by qt3to4:
#include <Q3PtrList>
using namespace std;

#include "processor.h"
#include "bufferdata.h"
#include "splitter.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

Splitter::Splitter(Processor *source, const uint sourceIndex) : LxConnection(source, sourceIndex)
{
	theConnections.setAutoDelete(true);
}

Splitter::~Splitter()
{
	theConnections.clear();
}

void Splitter::enforceMinimum(const uint elements)
{
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->enforceMinimum(elements);
}

void Splitter::forgetScratch(const BufferData &data)
{
	data.ignoreDeath();
	data.invalidate();
}

void Splitter::pushScratch(const BufferData &data)
{
	data.ignoreDeath();
	push(data);
	data.invalidate();
}

const SignalTypeRef Splitter::type()
{
	theSource->confirmTypes();
	return SignalTypeRef(theType);
}

void Splitter::checkExit()
{
	theSource->checkExit();
}

const bool Splitter::confirmTypes()
{
	return theSource->confirmTypes();
}

void Splitter::doRegisterOut(LxConnection *me, const uint port)
{
	if(MESSAGES) qDebug("Registering splitter link from a splitter connected to %s (port %d).", dynamic_cast<Processor *>(theSource)->name().latin1(), port);
	theConnections.append(me);
}

void Splitter::undoRegisterOut(LxConnection *me, const uint port)
{
	if(MESSAGES) qDebug("Unregistering splitter link from a splitter connected to %s (port %d).", dynamic_cast<Processor *>(theSource)->name().latin1(), port);
	theConnections.setAutoDelete(false);
	theConnections.remove(me);
	theConnections.setAutoDelete(true);
}

const bool Splitter::waitUntilReady()
{
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		if(!((*i)->waitUntilReady()))
			return false;
	return true;
}

void Splitter::setType(const SignalType *type)
{
	delete theType;
	theType = type->copy();
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->setType(type);
}

void Splitter::resetType()
{
	delete theType;
	theType = 0;
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->resetType();
}

void Splitter::sourceStopping()
{
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->sourceStopping();
}

void Splitter::sourceStopped()
{
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->sourceStopped();
}

void Splitter::reset()
{
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->reset();
}

BufferData Splitter::makeScratchElements(const uint elements, bool autoPush)
{
	BufferData ret = theConnections.first()->makeScratchElements(elements, autoPush);
	ret.adopt(dynamic_cast<ScratchOwner *>(this));
	return ret;
}

void Splitter::push(const BufferData &data)
{
	Q3PtrList<LxConnection>::iterator i = theConnections.begin();
	for(i++; i != theConnections.end(); i++)
		dynamic_cast<LxConnection *>(*i)->push(data);
	dynamic_cast<LxConnection *>(theConnections.first())->push(data);
}

void Splitter::pushPlunger()
{
	if(MESSAGES) qDebug("> Splitter::pushPlunger()");
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->pushPlunger();
	if(MESSAGES) qDebug("< Splitter::pushPlunger()");
}

void Splitter::startPlungers()
{
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->startPlungers();
}

void Splitter::plungerSent()
{
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->plungerSent();
}

void Splitter::noMorePlungers()
{
	for(Q3PtrList<LxConnection>::iterator i = theConnections.begin(); i != theConnections.end(); i++)
		(*i)->noMorePlungers();
}

const uint Splitter::maximumScratchElements(const uint minimum)
{
	Q3PtrList<LxConnection>::iterator i = theConnections.begin();
	uint ret = (*i)->maximumScratchElements(minimum);
	for(i++; i != theConnections.end(); i++)
		ret = ::min(ret, (*i)->maximumScratchElements(minimum));
	return ret;
}

const uint Splitter::maximumScratchElementsEver()
{
	Q3PtrList<LxConnection>::iterator i = theConnections.begin();
	uint ret = (*i)->maximumScratchElementsEver();
	for(i++; i != theConnections.end(); i++)
		ret = ::min(ret, (*i)->maximumScratchElementsEver());
	return ret;
}

};
