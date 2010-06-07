/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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

#include <qmutex.h>

#include "processor.h"
#include "buffer.h"

#include "bufferreader.h"

#define qMESSAGES 0
#define MESSAGES 0

namespace Geddei
{

BufferReader::BufferReader(Buffer *buffer)
{
	theBuffer = buffer;

	lastReadSize = 0;
	QFastMutexLocker lock(&theBuffer->theDataFlux);
	readPos = theBuffer->readPos;
	theUsed = theBuffer->theUsed;
	theToBeSkipped = 0;
	theAlreadyPlungedHere = 0;
	theBuffer->theReaders.append(this);

	lastRead = new BufferInfo(theBuffer->theData, this, theBuffer->theMask, BufferInfo::Foreign, BufferInfo::Read);
}

BufferReader::~BufferReader()
{
	if (MESSAGES) qDebug("> [%p] ~BufferData()", this);

	QFastMutexLocker lock(&theBuffer->theDataFlux);
	clearUNSAFE();
	delete lastRead;
	theBuffer->theReaders.removeOne(this);
	theBuffer->theDataIn.wakeAll();
	theBuffer->theDataOut.wakeAll();
}

void BufferReader::debug()
{
	theBuffer->debug();
}

void BufferReader::skipPlunger()
{
	if (MESSAGES) qDebug("> skipPlunger (rP: %d, tAPH: %d)", readPos, theAlreadyPlungedHere);
	QFastMutexLocker lock(&theBuffer->theDataFlux);
	theAlreadyPlungedHere++;
	if (MESSAGES) qDebug("< skipPlunger (rP: %d, tAPH: %d)", readPos, theAlreadyPlungedHere);
}

uint BufferReader::elementsReady() const
{
	if (MESSAGES) qDebug("= elementsReady (rP: %d, tAPH: %d", readPos, theAlreadyPlungedHere);
	QFastMutexLocker lock(&theBuffer->theDataFlux);
	int untilPlunger = theBuffer->nextPlungerUNSAFE(readPos, theAlreadyPlungedHere);
	if (untilPlunger == -1 || untilPlunger > signed(theUsed))
		return theUsed;
	else
		return untilPlunger;
}

int BufferReader::nextPlunger() const
{
	if (MESSAGES) qDebug("= nextPlunger");
	QFastMutexLocker lock(&theBuffer->theDataFlux);
	return theBuffer->nextPlungerUNSAFE(readPos, theAlreadyPlungedHere);
}

void BufferReader::waitForElements(uint elements) const
{
	if (MESSAGES) qDebug("= waitForElements");
	QFastMutexLocker lock(&theBuffer->theDataFlux);
	theBuffer->waitForUNSAFE(elements, this);
}

void BufferReader::skipElements(uint elements)
{
	if (MESSAGES) qDebug("> skipElements");
	QFastMutexLocker lock(&theBuffer->theDataFlux);
#ifdef EDEBUG
	if (elements >= theBuffer->size())
		qWarning("*** WARNING: skipElements(): Size of buffer is critically low (size: %d,\n"
				 "             elements: %d). Make use of Processor::specifyInputSpace().", theBuffer->theSize, elements);
#endif
	if (MESSAGES) qDebug("* skipElements (elements: %d)", elements);
	skipElementsUNSAFE(elements);
	if (MESSAGES) qDebug("< skipElements");
}

void BufferReader::skipElementsUNSAFE(uint elements)
{
	if (qMESSAGES) qDebug("> skipElementsUNSAFE (%d)", elements);

	if (!elements)
	{	if (qMESSAGES) qDebug("< skipElementsUNSAFE: zero!");
		return;
	}

	uint toBeRead = elements;

	// If we have a plunger in our way, wait until the pending read (if there is one)
	// has finished, so we can actually advance past the elements "immediately".
	// TODO: currently this freaks if it just sees a plunger. would be better to freak only if plunger is actually within affecting distance
	// note this is a worst case scenario. if the last read was actually a peek, rather than a read (or was a read that will be
	// turned into a peek) then we just have to check that elements < lastReadSize. Unfortunately semantics are that we can't be sure
	// what's going to happen yet. this could be fixed by preventing arbitrary changes of mind between reads and peeks, by privatising
	// (or otherwise preventing non-end of life calls of) forgetRead and haveRead.
	// i.e. plunger distance > elements + last_read_size (+ theToBeSkipped?).
//	if (theBuffer->nextPlungerUNSAFE(readPos, theAlreadyPlungedHere) != -1)
	if (uint(theBuffer->nextPlungerUNSAFE(readPos, theAlreadyPlungedHere)) < elements + lastReadSize)
	{	if (qMESSAGES) qDebug("= skipElementsUNSAFE: Plunger detected in stream. Waiting for last read (%p) to end...", lastRead);
		while (lastRead->isActive() && !theBuffer->trapdoorUNSAFE())
			theBuffer->theDataOut.wait(&theBuffer->theDataFlux);
		if (qMESSAGES) qDebug("= skipElementsUNSAFE: Last read done (%p)...", lastRead);
	}

	if (qMESSAGES) qDebug("= skipElementsUNSAFE: Waiting until we have enough space, ignoring plungers...");
	while (toBeRead > 0 && !theBuffer->trapdoorUNSAFE())
		toBeRead -= theBuffer->waitForIgnorePlungersUNSAFE(toBeRead, this);

	if (theBuffer->trapdoorUNSAFE())
	{	if (qMESSAGES) qDebug("< skipElementsUNSAFE Q");
		return;
	}

	if (lastRead->isActive())
	{	if (qMESSAGES) qDebug("= skipElementsUNSAFE: Noting amount to be skipped...");
		theToBeSkipped += elements;
	}
	else
	{
#ifdef EDEBUG
		if (theToBeSkipped) qWarning("*** STRANGE: Still have elements to skip inside a clear skipElements.");
#endif
		if (qMESSAGES) qDebug("= skipElementsUNSAFE: Artificially moving pointers...");
		readPos = (readPos + elements) & theBuffer->theMask;
		theUsed -= elements;
		theAlreadyPlungedHere = 0;
		theBuffer->updateUNSAFE();
	}
	if (qMESSAGES) qDebug("< skipElementsUNSAFE");
}

const BufferData BufferReader::readElements(uint elements, bool autoFree)
{
	if (MESSAGES) qDebug("> [%p] readElements(%d)", this, elements);
	QFastMutexLocker lock(&theBuffer->theDataFlux);
#ifdef EDEBUG
	if (elements > theBuffer->size())
		qWarning("*** WARNING: readElements(): Size of buffer is critically low (size: %d,\n"
				 "             elements: %d). Make use of Processor::specifyInputSpace().", theBuffer->theSize, elements);
#endif
	if (MESSAGES) qDebug("* readElements");
	uint ready = theBuffer->waitForUNSAFE(elements, this);
	if (MESSAGES) qDebug("Found %d ready. Want %d.", ready, elements);
	if (theBuffer->trapdoorUNSAFE())
	{	if (MESSAGES) qDebug("< readElements Q");
		return BufferData();
	}
	if (lastRead->isActive())
	{	qWarning("*** WARNING: An old BufferData object for reading from this object still exists\n"
				 "             on a further read. They should be nullified first with nullify().\n"
				 "             Trying damage limitation, though this is dangerous. Assuming that you\n"
				 "             want the read ignored.");
		// There's still another BufferData floating around. We'll just jettison it.
		// TODO: actually do what should be done at the end of its life.
		// Do this by making a new method in BufferInfo - kill
		lastRead->jettison();
		lastRead = new BufferInfo(theBuffer->theData, this, theBuffer->theMask, BufferInfo::Foreign, BufferInfo::Read);
	}

#ifdef EDEBUG
	if (lastRead->theLife == BufferInfo::Managed)
	{	qDebug("Our BufferData's theLife is managed!");
		assert(0);
	}
#endif
	lastRead->theValid = true;
	lastRead->theAccessibleSize = ready;
	lastRead->theEndType = autoFree ? BufferInfo::Activate : BufferInfo::Forget;
	lastRead->m_sampleSize = theBuffer->theType->size();
	lastRead->theValid = true;
	lastRead->thePlunger = (ready < elements);

	BufferData ret = BufferData(lastRead, readPos);

	lastReadSize = elements;
	if (MESSAGES) qDebug("< [%p] readElements (r: %p)", this, lastRead);
	return ret;
}

void BufferReader::haveRead(const BufferData &data)
{
	if (MESSAGES) qDebug("> haveRead");
	QFastMutexLocker lock(&theBuffer->theDataFlux);
	if (!data.info()->isLive())
	{	qWarning("*** ERROR: Cannot haveRead() on a non-live BufferInfo (v: %d, t: %s)", data.info()->theValid, data.info()->theLife == BufferInfo::Foreign ? "F" : "M");
		assert(0);
		return;
	}
	if (data.info() != lastRead)
	{	qWarning("*** ERROR: Cannot haveRead() on a BufferInfo (%p) that isn't our last one (%p)", data.info(), lastRead);
		return;
	}
	if (MESSAGES) qDebug("= [%p] haveRead(%p)", this, data.info());
	lastRead->invalidateAndIgnore();
	if (lastRead->theAccessibleSize)
	{	readPos = (readPos + lastRead->theAccessibleSize) & theBuffer->theMask;
		theUsed -= lastRead->theAccessibleSize;
		theAlreadyPlungedHere = 0;
	}
	if (lastRead->thePlunger) theAlreadyPlungedHere++;
	lastReadSize = 0;
	uint skipNow = theToBeSkipped;
	theToBeSkipped = 0;
	if (skipNow) skipElementsUNSAFE(skipNow);
	theBuffer->updateUNSAFE();
	if (MESSAGES) qDebug("< haveRead");
}

void BufferReader::forgetRead(const BufferData &data)
{
	if (MESSAGES) qDebug("> [%p] forgetRead (d: %p, lr: %p)", this, data.info(), lastRead);
	QFastMutexLocker lock(&theBuffer->theDataFlux);
	// next assertion: it shouldn't ever happen since this function cannot be called before last instance of bufferdata is destroyed
	// and it ignores death after this has been called, so must mean the user is freeing it twice.
	assert(data.info() == lastRead);
	if (MESSAGES) qDebug("= [%p] forgetRead (size: %d)", this, lastRead->theAccessibleSize);
	lastRead->theEndType = BufferInfo::Ignore;
	lastRead->theValid = false;
	lastReadSize = 0;
	uint skipNow = theToBeSkipped;
	theToBeSkipped = 0;
	if (MESSAGES) qDebug("= [%p] forgetRead (skipNow: %d, theUsed: %d)", this, skipNow, theUsed);
	if (skipNow) skipElementsUNSAFE(skipNow);
	theBuffer->updateUNSAFE();
	if (MESSAGES) qDebug("< [%p] forgetRead ", this);
}

void BufferReader::clearUNSAFE()
{
	if (MESSAGES) qDebug("> [%p] clearUNSAFE()", this);
	theUsed = 0;
	readPos = 0;
	theAlreadyPlungedHere = 0;
	theToBeSkipped = 0;
	if (lastRead->isActive())
	{
#ifdef EDEBUG
		qWarning("*** WARNING: An old and unsafe BufferData read object is left having cleared\n"
				 "             the Buffer. You should nullify it first.");
#endif
		lastRead->invalidateAndIgnore();
	}
}

void BufferReader::openTrapdoor(const Processor *processor)
{
	theBuffer->openTrapdoor(processor);
}

void BufferReader::closeTrapdoor(const Processor *processor)
{
	theBuffer->closeTrapdoor(processor);
}

}

#undef MESSAGES
