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

#include "qsocketsession.h"

#include "subprocessorfactory.h"
#include "processorfactory.h"
using namespace Geddei;

#include "commcodes.h"
#include "remotesession.h"
using namespace rGeddei;

#define MESSAGES 0

namespace rGeddei
{

RemoteSession::RemoteSession(const QString &host, uint port) : theKeeper(this), theTerminating(false)
{
	theSession = 0;
	theProcessorCount = 0;
	theHost = host;

	//TODO: Connect
	Q3SocketDevice *link = new Q3SocketDevice;
	if(!link->connect(QHostAddress(host), port))
	{
		if(MESSAGES)
			qDebug("*** ERROR: Couldn't connect to host %s. Code %d.", host.latin1(), (int)link->error());
		delete link;
		return;
	}

	if(MESSAGES)
		qDebug("Creating session...");
	theSession = new QSocketSession(link);

	if(MESSAGES)
		qDebug("Handshaking...");
	theSession->handshake(true);

	if(MESSAGES)
		qDebug("Getting session key...");
	theKey = theSession->safeReceiveWord<int>();

	if(MESSAGES)
		qDebug("OK. Key is %d.", theKey);
	theKeeper.start();
}

RemoteSession::~RemoteSession()
{
	if(MESSAGES) qDebug("> ~RemoteSession (%d): Nicely ending keeper...", theKey);
	theTerminating = true;
	if(!theKeeper.wait(1100))
	{
		if(MESSAGES) qDebug("= ~RemoteSession (%d): Terminating keeper", theKey);
		theKeeper.terminate();
		if(MESSAGES) qDebug("= ~RemoteSession (%d): Waiting for termination", theKey);
		theKeeper.wait(1000);
	}

	if(theSession)
	{
		if(MESSAGES) qDebug("= ~RemoteSession (%d): Sending EndSession command", theKey);
		theSession->sendByte(EndSession);
		if(MESSAGES) qDebug("= ~RemoteSession (%d): Waiting for Ack", theKey);
		theSession->waitForAck();
		if(MESSAGES) qDebug("= ~RemoteSession (%d): Deleting session", theKey);
		delete theSession;
	}

	if(MESSAGES) qDebug("< ~RemoteSession (%d):", theKey);
}

void RemoteSession::Keeper::run()
{
	while(!theOwner->theTerminating)
	{
		sleep(1);
		theOwner->keepAlive();
	}
}

bool RemoteSession::isValid()
{
	return theSession ? theSession->isOpen() : false;
}

bool RemoteSession::available(const QString &type)
{
	return typeAvailable(type) && typeVersion(type) == ProcessorFactory::versionId(type);
}

int RemoteSession::version(const QString &type)
{
	return typeAvailable(type) ? typeVersion(type) : -1;
}

bool RemoteSession::subAvailable(const QString &type)
{
	return typeSubAvailable(type) && typeSubVersion(type) == SubProcessorFactory::versionId(type);
}

int RemoteSession::subVersion(const QString &type)
{
	return typeSubAvailable(type) ? typeSubVersion(type) : -1;
}

bool RemoteSession::newProcessor(const QString &type, const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(NewProcessor);
	theSession->sendString(type.local8Bit());
	theSession->sendString(name.local8Bit());
	bool ret = theSession->waitForAck();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: NewProcessor(%s, %s)", type.latin1(), name.latin1());
		theLastError = NewProcessor;
	}
	return ret;
}

void RemoteSession::deleteProcessor(const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(DeleteProcessor);
	theSession->sendString(name.local8Bit());
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: DeleteProcessor(%s)", name.latin1());
		theLastError = DeleteProcessor;
	}
}

bool RemoteSession::newDomProcessor(const QString &subType, const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(NewDomProcessor);
	theSession->sendString(subType.local8Bit());
	theSession->sendString(name.local8Bit());
	bool ret = theSession->waitForAck();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: NewDomProcessor(%s, %s)", subType.latin1(), name.latin1());
		theLastError = NewDomProcessor;
	}
	return ret;
}

void RemoteSession::deleteDomProcessor(const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(DeleteDomProcessor);
	theSession->sendString(name.local8Bit());
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: DeleteDomProcessor(%s)", name.latin1());
		theLastError = DeleteDomProcessor;
	}
}

void RemoteSession::processorInit(const QString &name, const Properties &p, const QString &newName)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorInit);
	theSession->sendString(name.local8Bit());
	QByteArray a = p.serialise();
	theSession->safeSendWord(a.size());
	theSession->sendChunk((uchar *)a.data(), a.size());
	theSession->sendString(newName.local8Bit());
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorInit(%s, <Properties>)", name.latin1());
		theLastError = ProcessorInit;
	}
}

bool RemoteSession::processorGo(const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorGo);
	theSession->sendString(name.local8Bit());
	bool ret = theSession->waitForAck();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorGo(%s)", name.latin1());
		theLastError = ProcessorInit;
		return false;
	}
	return ret;
}

int RemoteSession::processorWaitUntilGoing(const QString &name, int &errorData)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorWaitUntilGoing);
	theSession->sendString(name.local8Bit());
	errorData = theSession->safeReceiveWord<int>();
	int ret = theSession->safeReceiveWord<int>();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error:ProcessorWaitUntilGoing(%s, %d, %d)", name.latin1(), errorData, ret);
		theLastError = ProcessorWaitUntilGoing;
		return false;
	}
	return ret;
}

void RemoteSession::processorWaitUntilDone(const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorWaitUntilDone);
	theSession->sendString(name.local8Bit());
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorWaitUntilDone(%s)", name.latin1());
		theLastError = ProcessorWaitUntilDone;
	}
}

void RemoteSession::processorStop(const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	if(MESSAGES) qDebug("> RS::processorStop(%s)", name.latin1());
	QMutexLocker lock(&theCalling);
	if(MESSAGES) qDebug("= RS::processorStop(%s): Got lock!", name.latin1());
	theSession->sendByte(ProcessorStop);
	theSession->sendString(name.local8Bit());
	if(MESSAGES) qDebug("= RS::processorStop(%s): wFA", name.latin1());
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorStop(%s)", name.latin1());
		theLastError = ProcessorStop;
	}
	if(MESSAGES) qDebug("< RS::processorStop(%s)", name.latin1());
}

void RemoteSession::processorReset(const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	if(MESSAGES) qDebug("> RS::processorReset(%s)", name.latin1());
	QMutexLocker lock(&theCalling);
	if(MESSAGES) qDebug("= RS::processorReset(%s): Got lock!", name.latin1());
	theSession->sendByte(ProcessorReset);
	theSession->sendString(name.local8Bit());
	if(MESSAGES) qDebug("= RS::processorReset(%s): wFA", name.latin1());
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorReset(%s)", name.latin1());
		theLastError = ProcessorReset;
	}
	if(MESSAGES) qDebug("< RS::processorReset(%s)", name.latin1());
}

bool RemoteSession::processorConnect(const QString &name, uint bufferSize, uint output, const QString &destName, uint destInput)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorConnectL);
	theSession->sendString(name.local8Bit());
	theSession->safeSendWord(bufferSize);
	theSession->safeSendWord(output);
	theSession->sendString(destName.local8Bit());
	theSession->safeSendWord(destInput);
	bool ret = theSession->waitForAck();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorConnectL(%s, %d, %d, %s, %d)", name.latin1(), bufferSize, output, destName.latin1(), destInput);
		theLastError = ProcessorConnectL;
		return false;
	}
	return ret;
}

bool RemoteSession::processorConnect(const QString &name, uint bufferSize, uint output, const QString &destHost, uint destKey, const QString &destName, uint destInput)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorConnectR);
	theSession->sendString(name.local8Bit());
	theSession->safeSendWord(bufferSize);
	theSession->safeSendWord(output);
	theSession->sendString(destHost.local8Bit());
	theSession->safeSendWord(destKey);
	theSession->sendString(destName.local8Bit());
	theSession->safeSendWord(destInput);
	bool ret = theSession->waitForAck();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorConnectR(%s, %d, %d, %s, %d, %s, %d)", name.latin1(), bufferSize, output, destHost.latin1(), destKey, destName.latin1(), destInput);
		theLastError = ProcessorConnectR;
		return false;
	}
	return ret;
}

void RemoteSession::processorDisconnect(const QString &name, uint output)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorDisconnect);
	theSession->sendString(name.local8Bit());
	theSession->safeSendWord(output);
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorDisconnect(%s, %d)", name.latin1(), output);
		theLastError = ProcessorDisconnect;
	}
}

void RemoteSession::processorDisconnectAll(const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorDisconnectAll);
	theSession->sendString(name.local8Bit());
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorDisconnectAll(%s)", name.latin1());
		theLastError = ProcessorDisconnectAll;
	}
}

void RemoteSession::processorSplit(const QString &name, uint output)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorSplit);
	theSession->sendString(name.local8Bit());
	theSession->safeSendWord(output);
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorSplit(%s, %d)", name.latin1(), output);
		theLastError = ProcessorSplit;
	}
}

void RemoteSession::processorShare(const QString &name, uint output)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(ProcessorShare);
	theSession->sendString(name.local8Bit());
	theSession->safeSendWord(output);
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: ProcessorShare(%s, %d)", name.latin1(), output);
		theLastError = ProcessorShare;
	}
}

bool RemoteSession::domProcessorCreateAndAdd(const QString &name)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(DomProcessorCreateAndAddL);
	theSession->sendString(name.local8Bit());
	bool ret = theSession->waitForAck();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: DomProcessorCreateAndAddL(%s)", name.latin1());
		theLastError = DomProcessorCreateAndAddL;
		return false;
	}
	return ret;
}

bool RemoteSession::domProcessorCreateAndAdd(const QString &name, const QString &host, uint hostKey)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(DomProcessorCreateAndAddR);
	theSession->sendString(name.local8Bit());
	theSession->sendString(host.local8Bit());
	theSession->safeSendWord(hostKey);
	bool ret = theSession->waitForAck();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: DomProcessorCreateAndAddR(%s, %s, %d)", name.latin1(), host.latin1(), hostKey);
		theLastError = DomProcessorCreateAndAddR;
		return false;
	}
	return ret;
}

bool RemoteSession::typeAvailable(const QString &type)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(TypeAvailable);
	theSession->sendString(type.local8Bit());
	bool ret = theSession->waitForAck();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: TypeAvailable(%s)", type.latin1());
		theLastError = TypeAvailable;
	}
	return ret;
}

int RemoteSession::typeVersion(const QString &type)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(TypeVersion);
	theSession->sendString(type.local8Bit());
	int ret = theSession->safeReceiveWord<int>();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: TypeVersion(%s)", type.latin1());
		theLastError = TypeVersion;
	}
	return ret;
}

bool RemoteSession::typeSubAvailable(const QString &type)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(TypeSubAvailable);
	theSession->sendString(type.local8Bit());
	bool ret = theSession->waitForAck();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: TypeSubAvailable(%s)", type.latin1());
		theLastError = TypeSubAvailable;
	}
	return ret;
}

int RemoteSession::typeSubVersion(const QString &type)
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(TypeSubVersion);
	theSession->sendString(type.local8Bit());
	int ret = theSession->safeReceiveWord<int>();
	if(!theSession->waitForAck())
	{	qWarning("*** ERROR: RemoteSession: Session error: TypeSubVersion(%s)", type.latin1());
		theLastError = TypeSubVersion;
	}
	return ret;
}

void RemoteSession::keepAlive()
{
	if(!theSession) { qFatal("*** FATAL: RemoteSession: Session to %s is not open.", theHost.latin1()); }
	QMutexLocker lock(&theCalling);
	theSession->sendByte(Nop);
	theSession->waitForAck();
}

};
