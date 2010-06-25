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

#include "bufferreader.h"
#include "processor.h"
#include "bufferdata.h"
#include "xlconnectionreal.h"
#include "mark.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

xLConnectionReal::xLConnectionReal(Sink *newSink, uint newSinkIndex, uint bufferSize)
	: xLConnection(newSink, newSinkIndex), theBuffer(bufferSize), m_minRead(0), m_minWrite(0)
{
	theReader = new BufferReader(&theBuffer);
	m_samplesRead = 0;
	m_latestPeeked = 0;
	m_latestTime = 0.0;
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

void xLConnectionReal::enforceMinimumRead(uint _elements)
{
	enforceMinimum((m_minRead = _elements) + m_minWrite);
}

void xLConnectionReal::enforceMinimumWrite(uint _elements)
{
	enforceMinimum((m_minWrite = _elements) + m_minRead);
}

void xLConnectionReal::enforceMinimum(uint elements)
{
	if (theBuffer.size() < elements)
	{
		qDebug() << "Enforcing minimum of" << elements << "on" << (void*)this << "(" << m_minRead << "+" << m_minWrite << ")";
		theBuffer.resize(elements);
	}
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

bool xLConnectionReal::require(uint samples, uint preferSamples)
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: requireSamples() cannot be called on a xLConnection object after\n"
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

	if (MESSAGES) qDebug("xLC::plungeSync(%d): Peeking %d elements...", samples, theType->size() * samples);
	BufferData ret = theReader->readElements(theType->size() * samples, false);
	theSink->checkExit();
	if (ret.plunger())
	{
		if (MESSAGES) qDebug("xLC: Plunger found. Discarding peeked elements...");
		theReader->haveRead(ret);
		if (MESSAGES) qDebug("xLC: Plunging sink...");
		theSink->plunged(theSinkIndex);
		m_samplesRead = 0;
		m_latestPeeked = 0;
		m_latestTime = 0.0;
		return false;
	}
	if (MESSAGES) qDebug("xLC: Plunger not found; read must have succeeded.");
	return true;
}

const BufferData xLConnectionReal::readElements(uint _elements)
{
#ifdef EDEBUG
	if (!theReader)
	{	qWarning("*** WARNING: readElements() cannot be called on a xLConnection object after\n"
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

void xLConnectionReal::sinkStopping()
{
	if (MESSAGES) qDebug("xLConnectionReal::sinkStopping(): theReader=%p", theReader);
	theBuffer.openTrapdoor(dynamic_cast<Processor *>(theSink));
}

void xLConnectionReal::sinkStopped()
{
	if (MESSAGES) qDebug("xLConnectionReal::sinkStopped(): theReader=%p", theReader);
	theBuffer.closeTrapdoor(dynamic_cast<Processor *>(theSink));
	m_samplesRead = 0;
	m_latestPeeked = 0;
	m_latestTime = 0.0;
}

Type const& xLConnectionReal::type() const
{
	if (theType.isNull())
		const_cast<xLConnectionReal*>(this)->pullType();
	return theType;
}

}

#undef MESSAGES
