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

#include <cassert>
using namespace std;

#include "processorforwarder.h"
#include "processor.h"
#include "bufferdata.h"
#include "commandcodes.h"
#include "rlconnection.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

RLConnection::RLConnection(QTcpSocket *sourceSocketDevice, Sink *newSink, int newSinkIndex, uint bufferSize) : xLConnectionReal(newSink, newSinkIndex, bufferSize), QThread(0), theSource(sourceSocketDevice)
{
	theBeingDeleted = false;
	theHaveType = false;
	if (MESSAGES) qDebug("RLC: Handshaking...");
	theSource.handshake(false);
	if (MESSAGES) qDebug("RLC: Handshaking finished.");
	if (theSource.isOpen())
		start(HighPriority);
	else
		qWarning("*** CRITICAL: RLConnection failed. Remote side not handshaking.");
}

RLConnection::~RLConnection()
{
	// This flag should never have to be used as the thread should be stopped before deletion, however
	// this is here for a fail-safe.

	theBeingDeleted = true;
	if (isRunning())
	{	if (MESSAGES) qDebug("RLConnection::~RLConnection(): Thread still running on RLConnection destruction. Safely stopping...");
		theSource.close();
		theBuffer.openTrapdoor(0);
		if (!wait(2000))
		{	qWarning("*** WARNING: Thread not responding. Terminating anyway.");
			terminate();
			wait(10000);
		}
		theBuffer.closeTrapdoor(0);
	}
}

void RLConnection::run()
{
	if (MESSAGES) qDebug("> RLC::run(): isOpen() = %d", theSource.isOpen());
	bool breakOut = false;
	while (theSource.isOpen())
	{
		if (MESSAGES) qDebug("= RLC::run(): Receiving...");
		uchar command;
		while (theSource.isOpen() && !theSource.receiveChunk(&command, 1, 501)) {}
		if (!theSource.isOpen()) break;

		if (MESSAGES) qDebug("= RLC::run(): command = %d", (int)command);
		switch (command)
		{
		case BufferWaitForFree:
		{	if (MESSAGES) qDebug("= RLC::run(): BufferWaitForFree");
			theBuffer.waitForFreeElements(1);
			if (MESSAGES) qDebug("= RLC::run(): Acking.");
			theSource.ack();
			break;
		}
		case BufferSpaceFree:
		{	if (MESSAGES) qDebug("= RLC::run(): BufferSpaceFree");
			theSource.safeSendWord(theBuffer.elementsFree());
			if (MESSAGES) qDebug("= RLC::run(): Done.");
			break;
		}
		case Transfer:
		{	if (MESSAGES) qDebug("= RLC::run(): Received transfer request.");
			int size = theSource.safeReceiveWord<int>();
			if (MESSAGES) qDebug("= RLC::run(): Creating buffer (size=%d).", size);
			BufferData data = theBuffer.makeScratchElements(size, false);
			if (!theSource.isOpen()) { breakOut = true; break; }
			if (MESSAGES) qDebug("= RLC::run(): Got buffer (size=%d). Reading data (rollsover=%d).", data.elements(), data.rollsOver());
			if (data.rollsOver())
			{	theSource.safeReceiveWordArray((int *)data.firstPart(), data.sizeFirstPart());
				theSource.safeReceiveWordArray((int *)data.secondPart(), data.sizeSecondPart());
			}
			else
				theSource.safeReceiveWordArray((int *)data.firstPart(), data.sizeOnlyPart());
			if (MESSAGES) qDebug("= RLC::run(): Pushing data.");
			theBuffer.push(data);
			if (MESSAGES) qDebug("= RLC::run(): Transfer completed.");
			break;
		}
		case SetType:
		{	if (MESSAGES) qDebug("= RLC::run(): SetType");
			delete theType;
			theType = TransmissionType::receive(theSource);
			if (MESSAGES) qDebug("= RLC::run(): theBuffer.setType()");
			theBuffer.setType(theType);
			if (MESSAGES) qDebug("= RLC::run(): Make lock");
			QFastMutexLocker lock(&theGotTypeM);
			if (MESSAGES) qDebug("= RLC::run(): Set haveType");
			theHaveType = true;
			if (MESSAGES) qDebug("= RLC::run(): wakeAll()");
			theGotType.wakeAll();
			break;
		}
		case ResetType:
		{	if (MESSAGES) qDebug("= RLC::run(): ResetType");
			delete theType;
			theType = 0;
			theBuffer.clear();
//			theBuffer.reset();
			break;
		}
		case WaitUntilReady:
		{	if (MESSAGES) qDebug("= RLC::run(): WaitUntilReady");
//			theSource.ack(theSink->confirmTypes());
			theSource.ack(theSink->waitUntilReady());
			if (MESSAGES) qDebug("= RLC::run(): All done.");
			break;
		}
		case IsReadyYet:
		{	if (MESSAGES) qDebug("= RLC::run(): IsReadyYet");
//			theSource.ack(theSink->confirmTypes());
			theSource.sendByte((uchar)theSink->isGoingYet());
			if (MESSAGES) qDebug("= RLC::run(): All done.");
			break;
		}
		case AppendPlunger:
		{	if (MESSAGES) qDebug("= RLC::run(): Appending plunger!");
			theBuffer.appendPlunger();
			if (MESSAGES) qDebug("= RLC::run(): Done.");
			break;
		}
		case StartPlungers:
		{	if (MESSAGES) qDebug("= RLC::run(): Starting Plungers.");
			theSink->startPlungers();
			if (MESSAGES) qDebug("= RLC::run(): Done.");
			break;
		}
		case PlungerSent:
		{	if (MESSAGES) qDebug("= RLC::run(): Plunger sent!");
			theSink->plungerSent(theSinkIndex);
			if (MESSAGES) qDebug("= RLC::run(): Done.");
			break;
		}
		case NoMorePlungers:
		{	if (MESSAGES) qDebug("= RLC::run(): No More Plungers!");
			theSink->noMorePlungers();
			if (MESSAGES) qDebug("= RLC::run(): Done.");
			break;
		}
		case EnforceMinimum:
		{	if (MESSAGES) qDebug("= RLC::run(): Enforcing minimum.");
			enforceMinimum(theSource.safeReceiveWord<int>());
			if (MESSAGES) qDebug("= RLC::run(): Done.");
			break;
		}
		case Close:
		{
			if (MESSAGES) qDebug("RLC: Got close command. Exitting immediately...");
//			theSource.ack();
//			if (MESSAGES) qDebug("RLC: Closing...");
			breakOut = true;
			break;
		}
		default:;
		}

		if (breakOut) break;
	}

	if (MESSAGES) qDebug("= RLC::run(): EXITING. isOpen() = %d", theSource.isOpen());

	if (theSource.isOpen()) theSource.close();
	// TODO: make thread safe.
	// TODO: if we want to have dynamic connections, we'll have to sort
	//       out some sort of pausing mechanism. for now we'll just block
	//       until the processor has stopped permenantly.
	if (MESSAGES) qDebug("= RLC::run(): Waiting on processor");
	theSink->waitToStop();
/*	if (theSink->theInputs[theSinkIndex] == this)
		theSink->theInputs[theSinkIndex] = 0;
	theSink = 0;
	if (!theBeingDeleted)
		ProcessorForwarder::deleteMeLater(this);
*/	if (MESSAGES) qDebug("< RLC::run()");
}

bool RLConnection::pullType()
{
	QFastMutexLocker lock(&theGotTypeM);
	while (!theHaveType) theGotType.wait(&theGotTypeM);
	return theType;
}

}

#undef MESSAGES

