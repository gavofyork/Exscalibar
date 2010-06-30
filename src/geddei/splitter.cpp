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

#include <cassert>
using namespace std;

#include "processor.h"
#include "bufferdata.h"
#include "splitter.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

Splitter::Splitter(Source *source, uint sourceIndex) : LxConnection(source, sourceIndex)
{
}

Splitter::~Splitter()
{
	while (theConnections.size())
		delete theConnections.takeLast();
}

void Splitter::enforceMinimumWrite(uint _elements)
{
	foreach (LxConnection* i, theConnections)
		i->setMinimumWrite(_elements / theType.size());
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

Type const& Splitter::type() const
{
	const_cast<Splitter*>(this)->theSource->confirmTypes();
	return theType;
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

void Splitter::setType(Type const& _type)
{
	LxConnection::setType(_type);
	foreach (LxConnection* i, theConnections)
		i->setType(theType);
}

void Splitter::resetType()
{
	theType.nullify();
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
	QList<LxConnection*>::iterator i = theConnections.begin();
	uint ret = (*i)->bufferElementsFree() / theType.size();
	for (i++; i != theConnections.end(); i++)
		ret = ::min(ret, (*i)->bufferElementsFree() / theType.size());
	return ret * theType.size();
}

uint Splitter::freeInDestinationBuffer(uint minimum)
{
	QList<LxConnection*>::iterator i = theConnections.begin();
	uint ret = (*i)->maximumScratchSamples((minimum + theType.size() - 1) / theType.size());
	for (i++; i != theConnections.end(); i++)
		ret = ::min(ret, (*i)->maximumScratchSamples((minimum + theType.size() - 1) / theType.size()));
	return ret * theType.size();
}

uint Splitter::freeInDestinationBufferEver()
{
	QList<LxConnection*>::iterator i = theConnections.begin();
	uint ret = (*i)->freeInDestinationBufferEver() / theType.size();
	for (i++; i != theConnections.end(); i++)
		ret = ::min(ret, (*i)->freeInDestinationBufferEver() / theType.size());
	return ret * theType.size();
}

BufferData Splitter::makeScratchElements(uint elements, bool autoPush)
{
	BufferData ret = theConnections.first()->makeScratchSamples(elements/theType.size(), autoPush);
	ret.adopt(dynamic_cast<ScratchOwner *>(this));
	return ret;
}

void Splitter::pushBE(const BufferData &data)
{
	QList<LxConnection*>::iterator i = theConnections.begin();
	for (i++; i != theConnections.end(); i++)
	{
		BufferData r = dynamic_cast<LxConnection *>(*i)->makeScratchSamples(data.elements() / theType.size());
		r.copyFrom(data);
		dynamic_cast<LxConnection *>(*i)->push(r);
	}
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

}

#undef MESSAGES
