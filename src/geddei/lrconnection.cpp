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
#include "commandcodes.h"
#include "lrconnection.h"
#include "processorforwarder.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

LRConnection::LRConnection(Source *newSource, uint sourceIndex, QTcpSocket *sinkSocketDevice) : LxConnectionReal(newSource, sourceIndex), theSink(sinkSocketDevice)
{
	if (MESSAGES) qDebug("LRC: Handshaking...");
	theSink.handshake(true);
	if (MESSAGES) qDebug("LRC: Handshaking finished (open=%d).", theSink.isOpen());
	if (!theSink.isOpen())
		qWarning("*** CRITICAL: LRConnection failed. Remote side not handshaking.");
}

LRConnection::~LRConnection()
{
	if (theSink.isOpen())
	{	if (MESSAGES) qDebug("LRC: Sending close command...");
		theSink.sendByte(Close);
		if (MESSAGES) qDebug("LRC: Telling remote forwarder to initiate deletion...");
		ProcessorForwarder::deleteConnection(theRemoteHost, theRemoteKey, theRemoteProcessorName, theRemoteIndex);
//		if (MESSAGES) qDebug("LRC: Waiting for ack...");
//		theSink.waitForAck(2000);
		if (MESSAGES) qDebug("LRC: Closing connection.");
		theSink.close();
	}
}

void LRConnection::setCredentials(const QString &remoteHost, uint remoteKey, const QString &remoteProcessorName, uint remoteIndex)
{
	theRemoteHost = remoteHost;
	theRemoteKey = remoteKey;
	theRemoteProcessorName = remoteProcessorName;
	theRemoteIndex = remoteIndex;
}

void LRConnection::sourceStopping()
{
	openTrapdoor();
}

void LRConnection::sourceStopped()
{
	closeTrapdoor();
}

void LRConnection::pushPlunger()
{
	theSink.sendByte(AppendPlunger);
}

void LRConnection::startPlungers()
{
	theSink.sendByte(StartPlungers);
}

void LRConnection::plungerSent()
{
	theSink.sendByte(PlungerSent);
}

void LRConnection::noMorePlungers()
{
	theSink.sendByte(NoMorePlungers);
}

void LRConnection::enforceMinimum(uint size)
{
	theSink.sendByte(EnforceMinimum);
	theSink.safeSendWord(size);
}

void LRConnection::setType(Type const& _type)
{
	LxConnectionReal::setType(_type);
	theSink.sendByte(SetType);
	_type->send(theSink);
}

void LRConnection::resetType()
{
	theSink.sendByte(ResetType);
	theType.nullify();
}

Type const& LRConnection::type() const
{
	if (!const_cast<LRConnection*>(this)->theSource->confirmTypes())
		theType.nullify();
	return theType;
}

void LRConnection::bufferWaitForFree()
{
	if (MESSAGES) qDebug("> LRC::bWFF()");
	if (theSink.isOpen())
	{	theSink.sendByte(BufferWaitForFree);
		while (!trapdoor() && theSink.isOpen() && !theSink.waitForAck(502)) {}
	}
	theSource->checkExit();
	if (MESSAGES) qDebug("< LRC::bWFF()");
}

uint LRConnection::bufferElementsFree()
{
	if (MESSAGES) qDebug("> LRC::bEF()");
	uint ret;
	if (theSink.isOpen())
	{	theSink.sendByte(BufferSpaceFree);
		ret = theSink.safeReceiveWord<int>();
	}
	else ret = 0;
	if (MESSAGES) qDebug("< LRC::bEF()");
	return ret;
}

bool LRConnection::waitUntilReady()
{
	if (MESSAGES) qDebug("> LRC::wUR()");
	if (theSink.isOpen())
	{	theSink.sendByte(WaitUntilReady);
		if (MESSAGES) qDebug("= LRC::wUR(): isOpen() = %d", theSink.isOpen());
		while (!trapdoor() && theSink.isOpen() && !theSink.waitForAck(503)) {}
	}
	if (MESSAGES) qDebug("= LRC::wUR(): checkExit() (isOpen() = %d)", theSink.isOpen());
	theSource->checkExit();
	if (MESSAGES) qDebug("< LRC::wUR()");
	return theSink.isOpen();
}

Connection::Tristate LRConnection::isReadyYet()
{
	Tristate ret = Failed;
	if (MESSAGES) qDebug("> LRC::iRY()");
	if (theSink.isOpen())
	{	theSink.sendByte(IsReadyYet);
		if (MESSAGES) qDebug("= LRC::iRY(): isOpen() = %d", theSink.isOpen());
		if (!trapdoor() && theSink.isOpen()) ret = (Tristate)theSink.receiveByte();
	}
	if (MESSAGES) qDebug("= LRC::iRY(): checkExit() (isOpen() = %d)", theSink.isOpen());
	theSource->checkExit();
	if (MESSAGES) qDebug("< LRC::iRY()");
	return theSink.isOpen() ? ret : Failed;
}

void LRConnection::transport(const BufferData &data)
{
	if (MESSAGES) qDebug("> LRC::transport() (L=%s, size=%d)", qPrintable(dynamic_cast<Processor *>(theSource)->name()), data.elements());
	// TODO: Currently this silently discards the data.
	// It should really block until the connection is remade or until it's stopped.
	// But I dont need to implement that until i want dynamic connections sorted.
	if (theSink.isOpen())
	{	theSink.sendByte(Transfer);
		// FIXME: thread could block here if opposite processor is stopped; trapdoor wouldn't work then.

		theSink.safeSendWord(data.elements());
		if (data.rollsOver())
		{	theSink.safeSendWordArray((int *)data.firstPart(), data.sizeFirstPart());
			theSink.safeSendWordArray((int *)data.secondPart(), data.sizeSecondPart());
		}
		else
			theSink.safeSendWordArray((int *)data.firstPart(), data.sizeOnlyPart());
		if (MESSAGES) qDebug("= LRC::transport(): Transport completed.");
	}
	theSource->checkExit();
	if (MESSAGES) qDebug("< LRC::transport()");
}

}

#undef MESSAGES

