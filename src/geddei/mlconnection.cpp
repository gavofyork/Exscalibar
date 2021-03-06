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

#include "bufferreader.h"
#include "lmconnection.h"
#include "mlconnection.h"
#include "buffer.h"
#include "processor.h"
#include "mark.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

MLConnection::MLConnection(Sink *sink, uint sinkIndex, LMConnection *connection)
	: xLConnection(sink, sinkIndex), theConnection(connection)
{
	theConnection->theConnections.append(this);
	theReader = new BufferReader(&(theConnection->theBuffer));
	m_samplesRead = 0;
	m_latestPeeked = 0;
	m_latestTime = 0.0;
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

bool MLConnection::require(uint samples, uint preferSamples)
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: requireSamples() cannot be called on a MLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return true;
	}
#endif
	if (MESSAGES) qDebug("xLC::require(%d, %d)...", samples, preferSamples);

	while (1)
	{
		int np = theReader->nextPlunger();
		if (np != -1 && (uint)np < samples * type().size())
		{
			theReader->skipElements(np);
			theReader->skipPlunger();
			theSink->plunged(theSinkIndex);
		}
		else if (np != -1 && (uint)np < preferSamples * type().size())
		{
			return true;
		}
		else
			return theReader->elementsReady() >= preferSamples * type().size();
	}
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

void MLConnection::enforceMinimumRead(uint elements)
{
	theConnection->enforceMinimumRead(elements);
}

void MLConnection::enforceMinimumWrite(uint elements)
{
	theConnection->enforceMinimumWrite(elements);
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

	BufferData ret = theReader->readElements(theType->size() * samples, false);
	theSink->checkExit();
	if (ret.plunger())
	{
		m_samplesRead = 0;
		m_latestPeeked = 0;
		m_latestTime = 0.0;
		theReader->haveRead(ret);
		theSink->plunged(theSinkIndex);
		return false;
	}
	return true;
}

const BufferData MLConnection::readElements(uint _elements)
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: readElements() cannot be called on an MLConnection object after\n"
				 "             killReader() has been called. Ignoring this call.\n");
		return BufferData();
	}
#endif

	while (1)
	{	BufferData ret = theReader->readElements(_elements, true);
		theSink->checkExit();
		if (!ret.plunger())
		{
			if (type().isA<Mark>())
				if (!Mark::isEndOfTime(ret))
					m_latestTime = Mark::timestamp(ret);
				else{}
			else
			{
				m_samplesRead += _elements / theType->size();
				m_latestPeeked = max(m_latestPeeked, m_samplesRead);
			}
			return ret;
		}
		m_samplesRead = 0;
		m_latestPeeked = 0;
		m_latestTime = 0.0;
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
		if (!ret.plunger())
		{
			if (type().isA<Mark>())
				if (!Mark::isEndOfTime(ret))
					m_latestTime = Mark::timestamp(ret);
				else{}
			else
				m_latestPeeked = m_samplesRead + elements / theType->size();
			return ret;
		}
		m_samplesRead = 0;
		m_latestPeeked = 0;
		m_latestTime = 0.0;
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
	m_samplesRead = 0;
	m_latestPeeked = 0;
	m_latestTime = 0.0;
}

void MLConnection::reset()
{
	m_samplesRead = 0;
	m_latestPeeked = 0;
	m_latestTime = 0.0;
}

void MLConnection::resetType()
{
	theType.nullify();
}

void MLConnection::setType(Type const& _type)
{
	theType = _type;
}

bool MLConnection::waitUntilReady()
{
//	return theSink->confirmTypes();
	return theSink->waitUntilReady();
}

Connection::Tristate MLConnection::isReadyYet()
{
//	return theSink->confirmTypes();
	return theSink->isGoingYet();
}

Type const& MLConnection::type() const
{
	if (theType.isNull())
		theType = theConnection->type();
	return theType;
}

}

#undef MESSAGES
