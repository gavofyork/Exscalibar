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

#include <cassert>
using namespace std;

#include "processor.h"
#include "bufferdata.h"
#include "splitter.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

Splitter::Splitter(Processor *source, uint sourceIndex) : LxConnection(source, sourceIndex)
{
}

Splitter::~Splitter()
{
	while (theConnections.size())
		delete theConnections.takeLast();
}

void Splitter::enforceMinimum(uint elements)
{
	foreach (LxConnection* i, theConnections)
		i->enforceMinimum(elements);
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

bool Splitter::confirmTypes()
{
	return theSource->confirmTypes();
}

void Splitter::doRegisterOut(LxConnection *me, uint port)
{
	if (MESSAGES) qDebug("Registering splitter link from a splitter connected to %s (port %d).", qPrintable(dynamic_cast<Processor *>(theSource)->name()), port);
	theConnections.append(me);
}

void Splitter::undoRegisterOut(LxConnection *me, uint port)
{
	if (MESSAGES) qDebug("Unregistering splitter link from a splitter connected to %s (port %d).", qPrintable(dynamic_cast<Processor *>(theSource)->name()), port);
	theConnections.removeAll(me);
}

bool Splitter::waitUntilReady()
{
	foreach (LxConnection* i, theConnections)
		if (!i->waitUntilReady())
			return false;
	return true;
}

Connection::Tristate Splitter::isReadyYet()
{
	Tristate ret = Succeeded;
	foreach (LxConnection* i, theConnections)
		if (i->isReadyYet() == Failed) return Failed;
		else if (i->isReadyYet() == Pending) ret = Pending;
	return ret;
}

void Splitter::setType(const SignalType *type)
{
	delete theType;
	theType = type->copy();
	foreach (LxConnection* i, theConnections)
		i->setType(type);
}

void Splitter::resetType()
{
	delete theType;
	theType = 0;
	foreach (LxConnection* i, theConnections)
		i->resetType();
}

void Splitter::sourceStopping()
{
	foreach (LxConnection* i, theConnections)
		i->sourceStopping();
}

void Splitter::sourceStopped()
{
	foreach (LxConnection* i, theConnections)
		i->sourceStopped();
}

void Splitter::reset()
{
	foreach (LxConnection* i, theConnections)
		i->reset();
}

uint Splitter::bufferElementsFree()
{
	uint ret = theConnections.first()->bufferElementsFree();
	QList<LxConnection*>::iterator i = theConnections.begin();
	for (i++; i != theConnections.end(); i++)
		ret = min(ret, (*i)->bufferElementsFree());
	return ret;
}

BufferData Splitter::makeScratchElements(uint elements, bool autoPush)
{
	BufferData ret = theConnections.first()->makeScratchElements(elements, autoPush);
	ret.adopt(dynamic_cast<ScratchOwner *>(this));
	return ret;
}

void Splitter::push(const BufferData &data)
{
	QList<LxConnection*>::iterator i = theConnections.begin();
	for (i++; i != theConnections.end(); i++)
		dynamic_cast<LxConnection *>(*i)->push(data);
	dynamic_cast<LxConnection *>(theConnections.first())->push(data);
}

void Splitter::pushPlunger()
{
	if (MESSAGES) qDebug("> Splitter::pushPlunger()");
	foreach (LxConnection* i, theConnections)
		i->pushPlunger();
	if (MESSAGES) qDebug("< Splitter::pushPlunger()");
}

void Splitter::startPlungers()
{
	foreach (LxConnection* i, theConnections)
		i->startPlungers();
}

void Splitter::plungerSent()
{
	foreach (LxConnection* i, theConnections)
		i->plungerSent();
}

void Splitter::noMorePlungers()
{
	foreach (LxConnection* i, theConnections)
		i->noMorePlungers();
}

uint Splitter::maximumScratchElements(uint minimum)
{
	QList<LxConnection*>::iterator i = theConnections.begin();
	uint ret = (*i)->maximumScratchElements(minimum);
	for (i++; i != theConnections.end(); i++)
		ret = ::min(ret, (*i)->maximumScratchElements(minimum));
	return ret;
}

uint Splitter::maximumScratchElementsEver()
{
	QList<LxConnection*>::iterator i = theConnections.begin();
	uint ret = (*i)->maximumScratchElementsEver();
	for (i++; i != theConnections.end(); i++)
		ret = ::min(ret, (*i)->maximumScratchElementsEver());
	return ret;
}

}

#undef MESSAGES
