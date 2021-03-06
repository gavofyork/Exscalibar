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

#include "lxconnectionreal.h"
#include "buffer.h"
#include "bufferdata.h"
#include "processor.h"
#include "mlconnection.h"
#include "lmconnection.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

LMConnection::LMConnection(Source *source, uint sourceIndex, uint bufferSize)
	: LxConnectionReal(source, sourceIndex), theBuffer(bufferSize), m_minRead(0), m_minWrite(0)
{
}

LMConnection::~LMConnection()
{
	while (theConnections.size())
		delete theConnections.takeLast();
}

void LMConnection::enforceMinimumRead(uint _elements)
{
	enforceMinimum((m_minRead = _elements) + m_minWrite);
}

void LMConnection::enforceMinimumWrite(uint _elements)
{
	enforceMinimum((m_minWrite = _elements) + m_minRead);
}

void LMConnection::enforceMinimum(uint elements)
{
	if (theBuffer.size() < elements)
		theBuffer.resize(elements);
}

bool LMConnection::waitUntilReady()
{
	foreach (MLConnection* i, theConnections)
		if (!i->waitUntilReady()) return false;
	return true;
}

Connection::Tristate LMConnection::isReadyYet()
{
	Tristate ret = Succeeded;
	foreach (MLConnection* i, theConnections)
		if (i->isReadyYet() == Failed) return Failed;
		else if (i->isReadyYet() == Pending) return ret = Pending;
	return ret;
}

void LMConnection::reset()
{
	theBuffer.clear();
	m_minRead = m_minWrite = 0;
}

Type const& LMConnection::type() const
{
	if (theType.isNull())
		const_cast<LMConnection*>(this)->theSource->confirmTypes();
	return theType;
}

void LMConnection::setType(Type const& _type)
{
	LxConnectionReal::setType(_type);
	foreach (MLConnection* i, theConnections)
		i->setType(theType);
	theBuffer.setType(theType);
}

void LMConnection::resetType()
{
	theType = TransmissionType();
	foreach (MLConnection* i, theConnections)
		i->resetType();

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

uint LMConnection::freeInDestinationBufferEver()
{
	return theBuffer.size();
}

uint LMConnection::freeInDestinationBuffer(uint minimum)
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
	foreach (MLConnection* i, theConnections)
		i->startPlungers();
}

void LMConnection::plungerSent()
{
	if (MESSAGES) qDebug("> LMConnection::plungerSent()");
	foreach (MLConnection* i, theConnections)
		i->plungerSent();
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
	foreach (MLConnection* i, theConnections)
		i->noMorePlungers();
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
