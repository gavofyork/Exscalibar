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

#include "commandcodes.h"
#include "rscoupling.h"
#include "properties.h"
#include "signaltyperefs.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

RSCoupling::RSCoupling(QTcpSocket *dev, SubProcessor *sub) : xSCoupling(sub), QThread(0), theSession(dev)
{
	theBeingDeleted = false;
	if (MESSAGES) qDebug("RSC: Handshaking...");
	theSession.handshake(false);
	if (MESSAGES) qDebug("RSC: Handshaking finished.");
	if (theSession.isOpen())
		start(HighPriority);
	else
		qWarning("*** CRITICAL: RSCoupling failed. Remote side not handshaking.");
}

RSCoupling::~RSCoupling()
{
	// This flag should never have to be used as the thread should be stopped before deletion, however
	// this is here for a fail-safe.
	theBeingDeleted = true;
	if (isRunning())
	{	if (MESSAGES) qDebug("RSCoupling::~RSCoupling(): Thread still running on RSCoupling destruction. Safely stopping...");
		theSession.close();
		// Trapdoor opening needed?
		if (!wait(2000))
		{	qWarning("*** WARNING: Thread not responding. Terminating anyway.");
			terminate();
			wait(10000);
		}
		// Trapdoor closing needed?
	}
}

void RSCoupling::run()
{
	if (MESSAGES) qDebug("> RSC::run(): isOpen() = %d", theSession.isOpen());
	bool breakOut = false;
	while (theSession.isOpen())
	{
		if (MESSAGES) qDebug("= RSC::run(): Receiving...");
		uchar command;
		while (theSession.isOpen() && !theSession.receiveChunk(&command, 1, 501)) {}
		if (!theSession.isOpen()) break;

		if (MESSAGES) qDebug("= RSC::run(): command = %d", (int)command);
		switch (command)
		{
		case InitFromProperties:
		{
			if (MESSAGES) qDebug("RSC: InitFromProperties...");
			int s = theSession.safeReceiveWord<int>();
			QByteArray a(s, ' ');
			theSession.receiveChunk((uchar *)a.data(), s);
			initFromProperties(Properties(a));
			if (MESSAGES) qDebug("RSC: InitFromProperties: Done.");
			break;
		}
		case SpecifyTypes:
		{
			if (MESSAGES) qDebug("RSC: SpecifyTypes...");
			SignalTypeRefs inTypes(theSession.safeReceiveWord<int>());
			for (uint i = 0; i < inTypes.count(); i++)
				inTypes.mutablePtrAt(i) = SignalType::receive(theSession);
			SignalTypeRefs outTypes(theSession.safeReceiveWord<int>());
			for (uint i = 0; i < outTypes.count(); i++)
				outTypes.mutablePtrAt(i) = SignalType::receive(theSession);
			specifyTypes(inTypes, outTypes);
			if (MESSAGES) qDebug("RSC: SpecifyTypes: Done.");
			break;
		}
		case Go:
			if (MESSAGES) qDebug("RSC: Go: Setting off.");
			go();
			theSession.ack();
			break;
		case Stop:
			if (MESSAGES) qDebug("RSC: Stop: Stopping.");
			stop();
			theSession.ack();
			break;
		case Transact:
		{
			if (MESSAGES) qDebug("RSC: Transact...");
			uint channels = theSession.safeReceiveWord<int>();
			if (MESSAGES) qDebug("RSC: BufferDatas size = %d", channels);
			BufferDatas d(channels);
			for (uint i = 0; i < d.size(); i++)
			{	uint size = theSession.safeReceiveWord<int>();
				uint scope = theSession.safeReceiveWord<int>();
				BufferData *data = new BufferData(size, scope);
				theSession.safeReceiveWordArray((int *)data->firstPart(), size);
				d.setData(i, data);
			}
			uint chunks = theSession.safeReceiveWord<int>();
			if (MESSAGES) qDebug("RSC: BufferDatas chunks = %d", chunks);
			transact(d, chunks);
			if (MESSAGES) qDebug("RSC: Transact: Done.");
			break;
		}
		case DeliverResults:
		{
			if (MESSAGES) qDebug("RSC: DeliverResults...");
			uint tt;
			BufferDatas d = deliverResults(&tt);
			theSession.safeSendWord(d.size());
			for (uint i = 0; i < d.size(); i++)
			{	// TODO: maybe take this into BufferData?
				theSession.safeSendWord(d[i].elements());
				theSession.safeSendWord(d[i].scope());
				if (d[i].rollsOver())
				{	theSession.safeSendWordArray((int *)d[i].firstPart(), d[i].sizeFirstPart());
					theSession.safeSendWordArray((int *)d[i].secondPart(), d[i].sizeSecondPart());
				}
				else
					theSession.safeSendWordArray((int *)d[i].firstPart(), d[i].sizeOnlyPart());
			}
			theSession.safeSendWord(tt);
			if (MESSAGES) qDebug("RSC: DeliverResults: Done.");
			break;
		}
		case ProcessChunks:
		{
			if (MESSAGES) qDebug("RSC: ProcessChunks...");
			uint channels = theSession.safeReceiveWord<int>();
			if (MESSAGES) qDebug("RSC: BufferDatas size = %d", channels);
			BufferDatas ins(channels);
			for (uint i = 0; i < ins.size(); i++)
			{	uint size = theSession.safeReceiveWord<int>();
				uint scope = theSession.safeReceiveWord<int>();
				BufferData *data = new BufferData(size, scope);
				theSession.safeReceiveWordArray((int *)data->firstPart(), size);
				ins.setData(i, data);
			}
			BufferDatas outs(theSession.safeReceiveWord<int>());
			for (uint i = 0; i < outs.count(); i++)
			{
				int sz = theSession.safeReceiveWord<int>();
				int sc = theSession.safeReceiveWord<int>();
				outs.setData(i, new BufferData(sz, sc));
			}
			uint chunks = theSession.safeReceiveWord<int>();
			if (MESSAGES) qDebug("RSC: BufferDatas chunks = %d", chunks);
			processChunks(ins, outs, chunks);
			for (uint i = 0; i < ins.size(); i++)
				if (outs[i].rollsOver())
				{	theSession.safeSendWordArray((int *)outs[i].firstPart(), outs[i].sizeFirstPart());
					theSession.safeSendWordArray((int *)outs[i].secondPart(), outs[i].sizeSecondPart());
				}
				else
					theSession.safeSendWordArray((int *)outs[i].firstPart(), outs[i].sizeOnlyPart());
			theSession.safeSendWord(0);
			ins.nullify();
			outs.nullify();
			if (MESSAGES) qDebug("RSC: ProcessChunks: Done.");
			break;
		}
		case DefineIO:
		{
			uint i = theSession.safeReceiveWord<int>();
			uint o = theSession.safeReceiveWord<int>();
			defineIO(i, o);
			break;
		}
		case Stopping:
			if (MESSAGES) qDebug("RSC: Got stopping command...");
			stopping();
			theSession.ack();
			if (MESSAGES) qDebug("RSC: Relayed.");
			break;
		case Stopped:
			if (MESSAGES) qDebug("RSC: Got stopped command...");
			stopped();
			theSession.ack();
			if (MESSAGES) qDebug("RSC: Relayed.");
			break;
		case Close:
			if (MESSAGES) qDebug("RSC: Got close command. Exiting immediately...");
			breakOut = true;
			break;
		default:;
		}

		if (breakOut) break;
	}
	if (MESSAGES) qDebug("RSC: Exiting session (open=%d)...", theSession.isOpen());
	if (theSession.isOpen()) theSession.close();
}

}

#undef MESSAGES
