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

#include <iostream>
using namespace std;

#include <QList>

#include "processor.h"
#include "buffer.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

ostream &operator<<(ostream &out, Buffer &me)
{
	out << "[";
	me.theDataFlux.lock();
	for (uint i = 0; i < me.theSize; i++)
		out << (i==me.writePos?(i==me.readPos?'X':'>'):(i==me.readPos?'<':' ')) << me.theData[i];
	me.theDataFlux.unlock();
	return out << " ]";
}

void Buffer::debug()
{
	QFastMutexLocker lock(&theDataFlux);
	QString out = "[";

	for (uint i = 0; i < theSize; i++)
	{
		char c = 'A';
		foreach (BufferReader* j, theReaders)
		{
			if (i == j->readPos) out += c; else out += " ";
			c++;
		}
		QList<uint>::Iterator p; for (p = thePlungers.begin(); p != thePlungers.end(); p++)
			if (*p == i) break;
		out += (readPos == i) ? "R" : " ";
		if (p == thePlungers.end()) out += " "; else out += "#";
		out += (writePos == i) ? "W" : " ";

		out += ":" + QString::number(theData[i & theMask]) + ":";
	}
	out += "]";
	qDebug("%s", qPrintable(out));
}

void Buffer::updateUNSAFE()
{
	// figure out how much theUsed will be moved on.
	if (MESSAGES) qDebug("Updating position");
	uint newUsed = 0;
	foreach (BufferReader* i, theReaders)
		if (newUsed < i->theUsed)
			newUsed = i->theUsed;

	uint moveBy = theUsed - newUsed;
	if (MESSAGES) qDebug("Moving by %d", moveBy);

	// clear old plunger (if any) - could be multiple due to potential for skipping
	if (MESSAGES) qDebug("Next plunger is %d away", (nextPlungerUNSAFE() - readPos) & theMask);
	while (1)
		if (thePlungers.begin() != thePlungers.end())
			if (((*thePlungers.begin() - readPos) & theMask) < moveBy)
				thePlungers.removeFirst();
			else
				break;
		else
			break;

	readPos = (readPos + moveBy) & theMask;
	theUsed -= moveBy;
	theDataOut.wakeAll();
}

Buffer::Buffer(uint size, Type const& _type) : theDataFlux(QFastMutex::Recursive)
{
	theDataFlux.lock();
	theSize = 4;
	theLogSize = 2;
	while (theSize < size) { theSize <<= 1; theLogSize++; }
/*#ifdef EDEBUG
	if (theSize != size)
		qDebug("*** Buffer increased from %d to %d in order that it be a power of 2.", size, theSize);
#endif*/
	theMask = theSize - 1;
	readPos = 0;
	writePos = 0;
	theUsed = 0;
	theData = new float[theSize];
	for (uint i = 0; i < theSize; i++) theData[i] = i + 0.666;
	theType = _type;
	lastScratch = new BufferInfo(theData, this, theMask, BufferInfo::Foreign, BufferInfo::Write);
	if (MESSAGES) qDebug("Creating new scratch: %p", lastScratch);
	lastScratch->thePlunger = false;

	theTrapdoors.clear();
	theDataFlux.unlock();
}

Buffer::~Buffer()
{
	if (MESSAGES) qDebug("> ~Buffer");
	theDataFlux.lock();
	while (theReaders.size())
		delete theReaders.takeLast();
	delete [] theData;
	theData = 0;
	theDataFlux.unlock();
	if (MESSAGES) qDebug("< ~Buffer");
}

void Buffer::resize(uint size)
{
	theDataFlux.lock();
	delete [] theData;
	theData = 0;

	theSize = 4;
	theLogSize = 2;
	while (theSize < size) { theSize <<= 1; theLogSize++; }
/*#ifdef EDEBUG
	if (theSize != size)
		qWarning("*** WARNING: Buffer increased from %d to %d in order that it be a power of 2.", size, theSize);
#endif*/
	theMask = theSize - 1;
	readPos = 0;
	writePos = 0;
	theUsed = 0;
	theData = new float[theSize];
	for (uint i = 0; i < theSize; i++) theData[i] = i + 0.666;

	if (lastScratch->isReferenced())
		qFatal("FATAL: Resizing buffer when scratch BufferData objects are still around.");
	lastScratch->theData = theData;
	lastScratch->theMask = theMask;
	lastScratch->thePlunger = false;

	theTrapdoors.clear();
	foreach (BufferReader* i, theReaders)
	{
		i->clearUNSAFE();
		i->lastRead->theData = theData;
		i->lastRead->theMask = theMask;
	}

	theDataFlux.unlock();
}

void Buffer::appendPlunger()
{
	if (MESSAGES) qDebug("Appending plunger to position %d", writePos);
	QFastMutexLocker lock(&theDataFlux);

#ifdef EDEBUG
	if (theSize - theUsed == 0 && !thePlungers.count())
		qWarning("*** WARNING: appendPlunger(): Size of buffer is critically low (size: %d).\n"
				 "             There is not enough room for one plunger. Please\n"
				 "             make use of Processor::specifyOutputSpace().", theSize);
#endif

	// Need at least 1 element free or theUsed == theSize, meaning that the plunger position
	// will be confused with 0 (theUsed & theMark == 0), and so a reader will erroneously
	// exit immediately.
	waitForFreeUNSAFE(1);
	thePlungers.push_back(writePos);
	if (MESSAGES) qDebug("Waking readers...");
	theDataIn.wakeAll();
	theDataOut.wakeAll();
}

void Buffer::discardNextPlunger()
{
	if (MESSAGES) qDebug("> discardNextPlungerUNSAFE");
	QFastMutexLocker lock(&theDataFlux);
	if (MESSAGES) qDebug("* discardNextPlungerUNSAFE");
	discardNextPlungerUNSAFE();
	if (MESSAGES) qDebug("< discardNextPlungerUNSAFE");
}

void Buffer::discardNextPlungerUNSAFE()
{
	if (MESSAGES) qDebug("* discardNextPlungerUNSAFE");
	if (thePlungers.begin() != thePlungers.end())
		thePlungers.removeFirst();
}

bool Buffer::trapdoorUNSAFE() const
{
	if (MESSAGES) qDebug("Checking trapdoor for %s...", Processor::threadProcessor() ? qPrintable(Processor::threadProcessor()->name()) : "[SubProcessor]");
	QVector<const Processor *>::const_iterator i;
	for (i = theTrapdoors.begin(); i != theTrapdoors.end(); i++)
		if (*i == Processor::threadProcessor()) break;
	return i != theTrapdoors.end();
}

void Buffer::openTrapdoor(const Processor *processor)
{
	if (MESSAGES) qDebug("> openTrapdoor %p (for %p: %s)", this, processor, processor ? qPrintable(processor->name()) : "<n/a>");
	if (MESSAGES) qDebug("= openTrapdoor(%p): Going to lock mutex: %p", this, &theDataFlux);
	theDataFlux.lock();
	theTrapdoors.push_back(processor);
	if (MESSAGES) qDebug("Size: %d", theTrapdoors.size());
	theDataIn.wakeAll();
	theDataOut.wakeAll();
	theDataFlux.unlock();
	if (MESSAGES) qDebug("< openTrapdoor");
}

void Buffer::closeTrapdoor(const Processor *processor)
{
	if (MESSAGES) qDebug("> closeTrapdoor %p (for %p: %s)", this, processor, processor ? qPrintable(processor->name()) : "<n/a>");
	theDataFlux.lock();
	if (MESSAGES) qDebug("* closeTrapdoor");
//	for (i = theTrapdoors.begin(); i != theTrapdoors.end(); i++)
//		qDebug("List: iterator %p", *i);
	if (MESSAGES) qDebug("Size: %d", theTrapdoors.size());

	QVector<const Processor *>::iterator i;
	for (i = theTrapdoors.begin(); i != theTrapdoors.end(); i++)
		if (*i == processor) break;
	assert(i != theTrapdoors.end());	// assert trapdoor is open.
	theTrapdoors.erase(i);
	theDataIn.wakeAll();
	theDataOut.wakeAll();
	theDataFlux.unlock();
	if (MESSAGES) qDebug("< closeTrapdoor");
}

int Buffer::nextPlungerUNSAFE() const
{
	if (thePlungers.begin() == thePlungers.end()) return -1;
	return *thePlungers.begin();
}

int Buffer::nextPlungerUNSAFE(uint pos, uint ignore) const
{
	// 'ii' starts at first plunger
	QList<uint>::const_iterator ii = thePlungers.begin();

	// First we skip down the plunger list until we find the first plunger at or past 'pos'
	for (; ii != thePlungers.end() && ((*ii - readPos) & theMask) < ((pos - readPos) & theMask); ii++) {}

	// Then we skip upto ignore plungers from the list while they are situated on 'pos'
	for (uint i = 0; i < ignore && ii != thePlungers.end() && *ii == pos; i++, ii++) {}

	// If we ran out of plungers, return -1
	if (ii == thePlungers.end()) return -1;

	// Otherwise return the relative position of the next plunger
	return (*ii - pos) & theMask;
}

/**
 * Returns r:
 *  r = elements: If correct amount of elements are now available
 *  0 <= r < elements: If a plunger occurs before elements are available
 *  r = Undefined: If trapdoor is open(-ed).
 */
uint Buffer::waitForUNSAFE(uint elements) const
{
	if (MESSAGES) qDebug("Waiting for %d elements...", elements);
	int nextPlunger = -1;
	while (!trapdoorUNSAFE())
	{
		if (nextPlunger == -1) nextPlunger = nextPlungerUNSAFE();
		if (nextPlunger < signed(elements) && nextPlunger != -1)
		{	if (MESSAGES) qDebug("Too close. Exiting...");
			return nextPlunger;
		}
		if (theUsed >= elements) return elements;
		theDataIn.wait(&theDataFlux);
	}
	return Undefined;
}

uint Buffer::waitForUNSAFE(uint elements, const BufferReader *reader) const
{
	if (MESSAGES) qDebug("Waiting for %d elements...", elements);
	int nextPlunger = -1;
	while (!trapdoorUNSAFE())
	{
		if (nextPlunger == -1)
			nextPlunger = nextPlungerUNSAFE(reader->readPos, reader->theAlreadyPlungedHere);

		if (nextPlunger < signed(elements) && nextPlunger != -1)
		{	if (MESSAGES) qDebug("Too close. Exiting...");
			return nextPlunger;
		}
		if (reader->theUsed >= elements) return elements;

		theDataIn.wait(&theDataFlux);
	}
	return Undefined;
}

uint Buffer::waitForIgnorePlungersUNSAFE(uint elements, const BufferReader *reader) const
{
	if (MESSAGES) qDebug("Waiting for %d elements, ignoring plungers...", elements);
	while (!trapdoorUNSAFE())
	{	if (reader->theUsed >= elements) return elements;
		theDataIn.wait(&theDataFlux);
	}
	return Undefined;
}

uint Buffer::elementsFree() const
{
	if (MESSAGES) qDebug("= elementsFree");
	QFastMutexLocker lock(&theDataFlux);
	return theSize - theUsed;
}

void Buffer::waitForFreeUNSAFE(uint elements) const
{
	if (MESSAGES) qDebug("> waitForFreeUNSAFE(%d): size: %d, used: %d", elements, theSize, theUsed);
	while (theSize - theUsed < elements && !trapdoorUNSAFE())
		theDataOut.wait(&theDataFlux);
	if (MESSAGES) qDebug("< waitForFreeUNSAFE(%d)", elements);
}

void Buffer::waitForFreeElements(uint elements) const
{
	if (MESSAGES) qDebug("= waitForFreeElements");
	QFastMutexLocker lock(&theDataFlux);
	return waitForFreeUNSAFE(elements);
}

BufferData Buffer::makeScratchElements(uint elements, bool autoPush)
{
	if (MESSAGES) qDebug("> makeScratchElements");
	QFastMutexLocker lock(&theDataFlux);
	if (MESSAGES) qDebug("* makeScratchElements (elements: %d)", elements);
#ifdef EDEBUG
	if (elements >= size())
		qWarning("*** WARNING: makeScratchElements(): Size of buffer is critically low (size: %d,\n"
				 "             elements: %d). Make use of Processor::specifyOutputSpace().", theSize, elements);
#endif
	waitForFreeUNSAFE(elements);
	if (trapdoorUNSAFE())
	{	if (MESSAGES) qDebug("< makeScratchElements Q");
		return BufferData();
	}
	if (lastScratch->isReferenced())
	{	qWarning("*** WARNING: An old BufferData object for writing from this object still exists\n"
				 "             on a further read. They should be nullified first with nullify().\n"
				 "             Trying damage limitation, though this is dangerous. Assuming that you\n"
				 "             want the write ignored.");
		// There's still another BufferData floating around. We'll just jettison it.
		if (MESSAGES) qDebug("Jettisoning old scratch: %p", lastScratch);
		lastScratch->jettison();
		lastScratch = new BufferInfo(theData, this, theMask, BufferInfo::Foreign, BufferInfo::Write);
		if (MESSAGES) qDebug("Creating new scratch due to jettison: %p", lastScratch);
		lastScratch->thePlunger = false;
	}
	if (MESSAGES) qDebug("Configuring BufferInfo (lastScratch)...");
	lastScratch->theValid = true;
	lastScratch->theAccessibleSize = elements;
	lastScratch->theEndType = autoPush ? BufferInfo::Activate : BufferInfo::Forget;
	lastScratch->m_sampleSize = theType->size();
	lastScratch->theValid = true;

	if (MESSAGES) qDebug("< Creating BufferData...");
	return BufferData(lastScratch, writePos);
}

void Buffer::pushScratch(const BufferData &data)
{
	if (MESSAGES) qDebug("> pushScratch");
	QFastMutexLocker lock(&theDataFlux);
	if (MESSAGES) qDebug("* pushScratch");
	//TODO: change to Fatal.
	assert(data.info() == lastScratch);
	lastScratch->invalidateAndIgnore();

	writePos = (writePos + lastScratch->theAccessibleSize) & theMask;
	theUsed += lastScratch->theAccessibleSize;
	foreach (BufferReader* i, theReaders)
		i->theUsed += lastScratch->theAccessibleSize;

	theDataIn.wakeAll();
	if (MESSAGES) qDebug("< pushScratch");
}

void Buffer::forgetScratch(const BufferData &data)
{
	QFastMutexLocker lock(&theDataFlux);

	if (data.info() != lastScratch)
	{	qFatal("*** FATAL: forgetScratch called on object whose info is not owned by this.\n"
			   "           We own %p, given %p. It's aux is %p. We are %p", lastScratch, data.info(), data.info()->theAux, this);
	}
	lastScratch->invalidateAndIgnore();
}

void Buffer::pushData(const BufferData &data)
{
	if (MESSAGES) qDebug("> pushData");
	QFastMutexLocker lock(&theDataFlux);
#ifdef EDEBUG
	if (data.theInfo->theAccessibleSize >= size())
		qWarning("*** WARNING: pushData(): Size of buffer is critically low (size: %d,\n"
				 "             data.size: %d). Make use of Processor::specifyOutputSpace().", theSize, data.theInfo->theAccessibleSize);
#endif
	if (MESSAGES) qDebug("* pushData");
	waitForFreeUNSAFE(data.theVisibleSize);
	if (trapdoorUNSAFE()) return;
	if (lastScratch->isReferenced())
	{
#ifdef EDEBUG
		qWarning("*** WARNING: An old and unsafe BufferData scatch object is left having used\n"
				 "             another to write to the Buffer. Nullify it first.");
#endif
		// We just want to invalidate the old object, not jettison it, since we won't be creating another to
		// take over its duty.
		//
		// We could jettison it and then create another, but we don't need to since we don't actually need
		// another BufferData --- unlike makeScratch we just want to write into the Buffer, not return a BD.
		lastScratch->invalidateAndIgnore();
	}

	if (writePos + data.theVisibleSize > theSize)
	{	uint sizeFirstPart = theSize - writePos, sizeSecondPart = data.theVisibleSize - (theSize - writePos);
		float *firstPart = theData + writePos, *secondPart = theData;
		if (data.rollsOver())
			if (sizeFirstPart > data.sizeFirstPart())
			{	memcpy(firstPart, data.firstPart(), data.sizeFirstPart() * 4);
				memcpy(firstPart + data.sizeFirstPart(), data.secondPart(), (sizeFirstPart - data.sizeFirstPart()) * 4);
				memcpy(secondPart, data.secondPart() + sizeFirstPart - data.sizeFirstPart(), sizeSecondPart * 4);
			}
			else
			{	memcpy(firstPart, data.firstPart(), sizeFirstPart * 4);
				memcpy(secondPart, data.firstPart() + sizeFirstPart, (data.sizeFirstPart() - sizeFirstPart) * 4);
				memcpy(secondPart + data.sizeFirstPart() - sizeFirstPart, data.secondPart(), data.sizeSecondPart() * 4);
			}
		else
		{	memcpy(firstPart, data.firstPart(), sizeFirstPart * 4);
			memcpy(secondPart, data.firstPart() + sizeFirstPart, sizeSecondPart * 4);
		}
	}
	else
		if (data.rollsOver())
		{	memcpy(theData + writePos, data.firstPart(), data.sizeFirstPart() * 4);
			memcpy(theData + writePos + data.sizeFirstPart(), data.secondPart(), data.sizeSecondPart() * 4);
		}
		else
			memcpy(theData + writePos, data.theInfo->theData + data.theOffset, data.theVisibleSize * 4);
	writePos = (writePos + data.theVisibleSize) & theMask;
	theUsed += data.theVisibleSize;
	foreach (BufferReader* i, theReaders)
		i->theUsed += data.theVisibleSize;
	theDataIn.wakeAll();
	if (MESSAGES) qDebug("< pushData");
}

void Buffer::push(const BufferData &data)
{
	if (data.info()->theValid)
		if (data.info() == lastScratch)
			pushScratch(data);
		else
			pushData(data);
	else
		qWarning("*** WARNING: You are trying to push an invalid BufferData object. This is\n"
				 "             probably because the source of the scratch has since been used\n"
				 "             for another write.\n"
				 "             You have to push the data *before* you create another scratch.\n"
				 "             This push will be ignored.");
}

void Buffer::clear()
{
	if (MESSAGES) qDebug("> clear");
	QFastMutexLocker lock(&theDataFlux);
	if (MESSAGES) qDebug("* clear");
	if (lastScratch->isReferenced())
	{
#ifdef EDEBUG
		qWarning("*** WARNING: An old and unsafe BufferData scatch object is left having cleared\n"
				 "             the Buffer (%p). You should nullify it first.", lastScratch);
#endif
		lastScratch->invalidateAndIgnore();
	}
	writePos = 0;
	readPos = 0;
	theUsed = 0;
	for (uint i = 0; i < theSize; i++) theData[i] = i + 0.667;
	foreach (BufferReader* i, theReaders)
		i->clearUNSAFE();
	thePlungers.clear();
	theDataOut.wakeAll();
	if (MESSAGES) qDebug("< clear");
}

void Buffer::setType(Type const& _type)
{
	theType = _type;
	clear();
}

}

#undef MESSAGES
