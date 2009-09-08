/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
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

void LRConnection::setType(const SignalType *type)
{
	theSink.sendByte(SetType);
	type->send(theSink);
	delete theType;
	theType = type->copy();
}

void LRConnection::resetType()
{
	theSink.sendByte(ResetType);
	delete theType;
	theType = 0;
}

const SignalTypeRef LRConnection::type()
{
	if (theSource->confirmTypes()) return SignalTypeRef(theType);
	return SignalTypeRef(theType = 0);
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

