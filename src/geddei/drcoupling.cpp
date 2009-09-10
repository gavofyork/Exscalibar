/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include "qsocketsession.h"

#include "domprocessor.h"
#include "drcoupling.h"
#include "commandcodes.h"
#include "processorforwarder.h"

#define MESSAGES 0

namespace Geddei
{

DRCoupling::DRCoupling(DomProcessor *dom, QTcpSocket *remote) : DxCoupling(dom), theRemote(remote)
{
	if (MESSAGES) qDebug("DRC: Handshaking...");
	theRemote.handshake(true);
	if (MESSAGES) qDebug("DRC: Handshaking finished (open=%d).", theRemote.isOpen());
	if (!theRemote.isOpen())
		qWarning("*** CRITICAL: DRCoupling failed. Remote side not handshaking.");
	dom->ratify(this);
}

DRCoupling::~DRCoupling()
{
	if (theRemote.isOpen())
	{	if (MESSAGES) qDebug("DRC: Sending close command...");
		if (theComm.tryLock())
		{	if (MESSAGES) qDebug("DRC: Lock aquired - sending command...");
			theRemote.sendByte(Close);
			theComm.unlock();
		}
		else
			qWarning("WARNING: DRCoupling: Couldn't aquire lock. Closing regardless.");
		if (MESSAGES) qDebug("DRC: Telling remote forwarder to initiate deletion...");
		ProcessorForwarder::deleteCoupling(theRemoteHost, theRemoteKey, theRemoteSubProcessorKey);
		if (MESSAGES) qDebug("DRC: Closing connection.");
		theRemote.close();
	}
}

void DRCoupling::setCredentials(const QString &remoteHost, uint remoteKey, uint remoteSubProcessorKey)
{
	theRemoteHost = remoteHost;
	theRemoteKey = remoteKey;
	theRemoteSubProcessorKey = remoteSubProcessorKey;
}

void DRCoupling::go()
{
	if (MESSAGES) qDebug("> DRCoupling::go()");
	QMutexLocker lock(&theComm);
	theRemote.sendByte(Go);
	theRemote.waitForAck(501);
	if (MESSAGES) qDebug("< DRCoupling::go()");
}

void DRCoupling::stop()
{
	if (MESSAGES) qDebug("> DRCoupling::stop()");
	QMutexLocker lock(&theComm);
	if (MESSAGES) qDebug("= DRCoupling::stop(): Sending byte");
	theRemote.sendByte(Stop);
	theRemote.waitForAck(502);
	if (MESSAGES) qDebug("< DRCoupling::stop()");
}

void DRCoupling::stoppingR()
{
	if (MESSAGES) qDebug("> DRCoupling::stoppingR()");
	// Cant just send byte as caller is a different thread to rest of comms - could clash.
	QMutexLocker lock(&theComm);
	if (MESSAGES) qDebug("= DRCoupling::stoppingR(): Sending byte");
	theRemote.sendByte(Stopping);
	theRemote.waitForAck(503);
	if (MESSAGES) qDebug("< DRCoupling::stoppingR()");
}

void DRCoupling::stoppedR()
{
	if (MESSAGES) qDebug("> DRCoupling::stoppedR()");
	QMutexLocker lock(&theComm);
	if (MESSAGES) qDebug("= DRCoupling::stoppedR(): Sending byte");
	theRemote.sendByte(Stopped);
	theRemote.waitForAck(504);
	if (MESSAGES) qDebug("< DRCoupling::stoppedR()");
}

void DRCoupling::specifyTypes(const SignalTypeRefs &inTypes, const SignalTypeRefs &outTypes)
{
	if (MESSAGES) qDebug("> DRCoupling::specifyTypes()");
	QMutexLocker lock(&theComm);
	theRemote.sendByte(SpecifyTypes);
	// Send inTypes.count(), Go through each of inTypes, sending each one.
	theRemote.safeSendWord(inTypes.count());
	for (uint i = 0; i < inTypes.count(); i++)
		inTypes.ptrAt(0)->send(theRemote);
	// Do same from outTypes.
	theRemote.safeSendWord(outTypes.count());
	for (uint i = 0; i < outTypes.count(); i++)
		outTypes.ptrAt(0)->send(theRemote);
	if (MESSAGES) qDebug("< DRCoupling::specifyTypes()");
}

void DRCoupling::initFromProperties(const Properties &p)
{
	if (MESSAGES) qDebug("> DRCoupling::initFromProperties()");
	QMutexLocker lock(&theComm);
	if (MESSAGES) for (uint i = 0; i < (uint)p.keys().count(); i++) qDebug("p[%s] = %s", qPrintable(p.keys()[i]), qPrintable(p[p.keys()[i]].toString()));
	theRemote.sendByte(InitFromProperties);
	// Serialise and send properties.
	QByteArray sp = p.serialise();
	theRemote.safeSendWord(sp.size());
	theRemote.sendChunk((uchar *)sp.data(), sp.size());
	if (MESSAGES) qDebug("< DRCoupling::initFromProperties()");
}

void DRCoupling::transact(const BufferDatas &d, uint chunks)
{
	if (MESSAGES) qDebug("> DRCoupling::transact() (%d chunks)", chunks);
	QMutexLocker lock(&theComm);
	theRemote.sendByte(Transact);
	// Go through each BufferData in d, send each
	theRemote.safeSendWord(d.size());
	for (uint i = 0; i < d.size(); i++)
	{	// TODO: maybe take this into BufferData?
		theRemote.safeSendWord(d[i].elements());
		theRemote.safeSendWord(d[i].scope());
		if (d[i].rollsOver())
		{	theRemote.safeSendWordArray((int *)d[i].firstPart(), d[i].sizeFirstPart());
			theRemote.safeSendWordArray((int *)d[i].secondPart(), d[i].sizeSecondPart());
		}
		else
			theRemote.safeSendWordArray((int *)d[i].firstPart(), d[i].sizeOnlyPart());
	}
	theRemote.safeSendWord(chunks);
	if (MESSAGES) qDebug("< DRCoupling::transact()");
}

BufferDatas DRCoupling::deliverResults(uint *timeTaken)
{
	if (MESSAGES) qDebug("> DRCoupling::deliverResults()");
	QMutexLocker lock(&theComm);
	theRemote.sendByte(DeliverResults);
	// Wait for results to be sent.
	BufferDatas d(theRemote.safeReceiveWord<int>());
	for (uint i = 0; i < d.size(); i++)
	{	uint size = theRemote.safeReceiveWord<int>();
		uint scope = theRemote.safeReceiveWord<int>();
		BufferData *data = new BufferData(size, scope);
		theRemote.safeReceiveWordArray((int *)data->firstPart(), size);
		d.setData(i, data);
	}
	uint tt = theRemote.safeReceiveWord<int>();
	if (timeTaken) *timeTaken = tt;
	if (MESSAGES) qDebug("< DRCoupling::deliverResults(): Returning %d samples (tt=%d)", d.size() ? d[0].samples() : 0, tt);
	return d;
}

void DRCoupling::defineIO(uint inputs, uint outputs)
{
	QMutexLocker lock(&theComm);
	theRemote.sendByte(DefineIO);
	theRemote.safeSendWord(inputs);
	theRemote.safeSendWord(outputs);
}

}

#undef MESSAGES
