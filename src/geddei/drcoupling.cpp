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

#include "qsocketsession.h"

#include "domprocessor.h"
#include "drcoupling.h"
#include "commandcodes.h"
#include "processorforwarder.h"

#define MESSAGES 0

namespace Geddei
{

DRCoupling::DRCoupling(DomProcessor *dom, QTcpSocket *remote) : DxCoupling(dom), theRemote(remote), m_isReady(true)
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

void DRCoupling::specifyTypes(const Types &inTypes, const Types &outTypes)
{
	if (MESSAGES) qDebug("> DRCoupling::specifyTypes()");
	QFastMutexLocker lock(&theComm);
	theRemote.sendByte(SpecifyTypes);
	// Send inTypes.count(), Go through each of inTypes, sending each one.
	theRemote.safeSendWord(inTypes.count());
	for (uint i = 0; i < inTypes.count(); i++)
		inTypes[i]->send(theRemote);
	// Do same from outTypes.
	theRemote.safeSendWord(outTypes.count());
	for (uint i = 0; i < outTypes.count(); i++)
		outTypes[i]->send(theRemote);
	if (MESSAGES) qDebug("< DRCoupling::specifyTypes()");
}

void DRCoupling::initFromProperties(const Properties &p)
{
	if (MESSAGES) qDebug("> DRCoupling::initFromProperties()");
	QFastMutexLocker lock(&theComm);
	if (MESSAGES) for (uint i = 0; i < (uint)p.keys().count(); i++) qDebug("p[%s] = %s", qPrintable(p.keys()[i]), qPrintable(p[p.keys()[i]].toString()));
	theRemote.sendByte(InitFromProperties);
	// Serialise and send properties.
	QByteArray sp = p.serialise();
	theRemote.safeSendWord(sp.size());
	theRemote.sendChunk((uchar *)sp.data(), sp.size());
	if (MESSAGES) qDebug("< DRCoupling::initFromProperties()");
}

void DRCoupling::processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _chunks)
{
	if (MESSAGES) qDebug("> DRCoupling::processChunks() (%d chunks)", _chunks);
	m_outs = _outs;
	m_isReady = false;
	QFastMutexLocker lock(&theComm);
	theRemote.sendByte(ProcessChunks);
	// Go through each BufferData in d, send each
	theRemote.safeSendWord(_ins.size());
	for (uint i = 0; i < _ins.size(); i++)
	{	// TODO: maybe take this into BufferData?
		theRemote.safeSendWord(_ins[i].elements());
		theRemote.safeSendWord(_ins[i].sampleSize());
		if (_ins[i].rollsOver())
		{	theRemote.safeSendWordArray((int *)_ins[i].firstPart(), _ins[i].sizeFirstPart());
			theRemote.safeSendWordArray((int *)_ins[i].secondPart(), _ins[i].sizeSecondPart());
		}
		else
			theRemote.safeSendWordArray((int *)_ins[i].firstPart(), _ins[i].sizeOnlyPart());
	}
	theRemote.safeSendWord(_outs.size());
	for (uint i = 0; i < _outs.size(); i++)
	{	theRemote.safeSendWord(_outs[i].elements());
		theRemote.safeSendWord(_outs[i].sampleSize());
	}
	theRemote.safeSendWord(_chunks);
	if (MESSAGES) qDebug("< DRCoupling::processChunks()");
}

bool DRCoupling::isReady()
{
	if (m_isReady)
		return true;
	if (MESSAGES) qDebug("> DRCoupling::isReady()");
	QFastMutexLocker lock(&theComm);
	if (theRemote.bytesAvailable())
	{
		for (uint i = 0; i < m_outs.size(); i++)
		{	theRemote.safeReceiveWordArray((int *)m_outs[i].firstPart(), m_outs[i].sizeFirstPart());
			theRemote.safeReceiveWordArray((int *)m_outs[i].secondPart(), m_outs[i].sizeSecondPart());
		}
		theRemote.safeReceiveWord<int>();
		m_outs.nullify();
		m_isReady = true;
	}
	if (MESSAGES) qDebug("< DRCoupling::isReady(): Returning (isReady=%d)", m_isReady);
	return m_isReady;
}

void DRCoupling::defineIO(uint inputs, uint outputs)
{
	QFastMutexLocker lock(&theComm);
	theRemote.sendByte(DefineIO);
	theRemote.safeSendWord(inputs);
	theRemote.safeSendWord(outputs);
}

}

#undef MESSAGES
