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

#include "commandcodes.h"
#include "rscoupling.h"
#include "properties.h"
#include "types.h"
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
			theSubProc->initFromProperties(Properties(a));
			if (MESSAGES) qDebug("RSC: InitFromProperties: Done.");
			break;
		}
		case SpecifyTypes:
		{
			if (MESSAGES) qDebug("RSC: SpecifyTypes...");
			Types inTypes(theSession.safeReceiveWord<int>());
			for (uint i = 0; i < inTypes.count(); i++)
				inTypes[i] = TransmissionType::receive(theSession);
			Types outTypes(theSession.safeReceiveWord<int>());
			for (uint i = 0; i < outTypes.count(); i++)
				outTypes[i] = TransmissionType::receive(theSession);

			Types dummyOutTypes(outTypes.count());
			if (!theSubProc->proxyVSTypes(inTypes, dummyOutTypes))
				qDebug("*** CRITICAL: SubProcessor does not verify previously validated types.");
			theSubProc->theOutTypes = outTypes;
			if (MESSAGES) qDebug("RSC: SpecifyTypes: Done.");
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
				uint sampleSize = theSession.safeReceiveWord<int>();
				BufferData *data = new BufferData(size, sampleSize);
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
			theSubProc->processChunks(ins, outs, chunks);
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
			theSubProc->defineIO(i, o);
			break;
		}
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
