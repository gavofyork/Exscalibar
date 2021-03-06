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
#include "llconnection.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

LLConnection::LLConnection(Source *newSource, uint sourceIndex, Sink *newSink, uint newSinkIndex, uint bufferSize): LxConnectionReal(newSource, sourceIndex), xLConnectionReal(newSink, newSinkIndex, bufferSize)
{
}

bool LLConnection::pullType()
{
	theSource->confirmTypes();
	return !theType.isNull();
}

void LLConnection::setType(Type const& _type)
{
	LxConnectionReal::setType(_type);
	theBuffer.setType(theType);
}

void LLConnection::resetType()
{
	if (MESSAGES) qDebug("LLConnection: Resetting type...");
	theType = TransmissionType();

	// TODO: should really reset the buffer's type...
	theBuffer.clear();
//	theBuffer.reset();
}

void LLConnection::bufferWaitForFree()
{
	theBuffer.waitForFreeElements(1);
	theSource->checkExit();
}

uint LLConnection::bufferElementsFree()
{
	return theBuffer.elementsFree();
}

BufferData LLConnection::makeScratchElements(uint elements, bool autoPush)
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

bool LLConnection::waitUntilReady()
{
	// Perhaps this shouldn't always call confirmTypes(), since it will clear stuff that we may not want cleared
	//theSink->confirmTypes();
	return theSink->waitUntilReady();
}

Connection::Tristate LLConnection::isReadyYet()
{
	// Perhaps this shouldn't always call confirmTypes(), since it will clear stuff that we may not want cleared
	//theSink->confirmTypes();
	return theSink->isGoingYet();
}

void LLConnection::pushPlunger()
{
	if (MESSAGES) qDebug("> LLConnection::pushPlunger(): Appending plunger to buffer");
	theBuffer.appendPlunger();
	if (MESSAGES) qDebug("< LLConnection::pushPlunger()");
}

void LLConnection::plungerSent()
{
	theSink->plungerSent(theSinkIndex);
}

void LLConnection::startPlungers()
{
	theSink->startPlungers();
}

void LLConnection::noMorePlungers()
{
	theSink->noMorePlungers();
}

void LLConnection::transport(const BufferData &data)
{
	// TODO: some error checking...
	theBuffer.push(data);
	theSource->checkExit();
}

void LLConnection::sourceStopping()
{
	theBuffer.openTrapdoor(theSource->processor());
}

void LLConnection::sourceStopped()
{
	theBuffer.closeTrapdoor(theSource->processor());
}

}

#undef MESSAGES
