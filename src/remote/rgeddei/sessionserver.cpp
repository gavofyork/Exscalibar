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

#include "processorfactory.h"
#include "subprocessorfactory.h"
#include "domprocessor.h"
using namespace Geddei;

#include "sessionserver.h"
using namespace rGeddei;

#define MESSAGES 0

namespace rGeddei
{

QMap<uint, SessionServer *> SessionServer::theSessionKeyMap;
QMutex SessionServer::mutSessionKeyMap(QMutex::Recursive);
SessionServer::SessionServerReaper *SessionServer::theReaper = 0;

void SessionServer::SessionServerReaper::run()
{
	while (1)
	{
		sleep(5);
		SessionServer::reap();
	}
}

SessionServer::SessionServer()
{
	if (MESSAGES) qDebug("> SessionServer()");
	theReaping = false;
	theSessionKey = 0;
	// session keys < 10 are reserved.
	while (theSessionKey < 65536)
		theSessionKey = random();
	if (MESSAGES) qDebug("= SessionServer(): key = %d. Registering in key map...", theSessionKey);
	{	QMutexLocker lock(&mutSessionKeyMap);
		theSessionKeyMap[theSessionKey] = this;
	}
	if (MESSAGES) qDebug("= SessionServer(): Locking theCalling and registering alive...");
	{	QMutexLocker lock(&theCalling);
		theAlive = true;
	}
	if (MESSAGES) qDebug("= SessionServer(): Creating and starting reaper if it doesn't yet exist...");
	if (!theReaper) (theReaper = new SessionServerReaper)->start();
	if (MESSAGES) qDebug("< SessionServer(): All done!");
}

SessionServer::~SessionServer()
{
	if (MESSAGES) qDebug("> ~SessionServer()");
	{
		QMutexLocker lock(&mutSessionKeyMap);
		theSessionKeyMap.remove(theSessionKey);
	}
	if (theProcessors.count())
		qWarning("*** Session %d is being deleted. %d processors left undeleted!", theSessionKey, theProcessors.count());
	for (QMap<QString, Processor *>::Iterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.value()->stop();
	// TODO: Delete all processors when I believe I can do it safely.
//	theProcessors.deleteAll();
	if (MESSAGES) qDebug("< ~SessionServer(): key = %d", theSessionKey);
}

void SessionServer::safeDelete(uint key)
{
	QMutexLocker lock(&mutSessionKeyMap);
	if (theSessionKeyMap.count(key))
		delete theSessionKeyMap[key];
}

void SessionServer::reap()
{
	QMutexLocker lock(&mutSessionKeyMap);
	QMap<uint, SessionServer *> skm = theSessionKeyMap;
	for (QMap<uint, SessionServer *>::Iterator i = skm.begin(); i != skm.end(); i++)
	{	if (MESSAGES) qDebug("Checking if session %d is still alive...", i.key());
		if (!(*i)->alive())
			delete *i;
		else
			(*i)->resetAlive();
	}
}

bool SessionServer::alive() const
{
	QMutexLocker lock(&theCalling);
	return theAlive;
}

void SessionServer::setAlive()
{
	QMutexLocker lock(&theCalling);
	theAlive = true;
}

void SessionServer::resetAlive()
{
	QMutexLocker lock(&theCalling);
	theAlive = false;
}

bool SessionServer::newProcessor(const QString &type, const QString &name, bool &ret)
{
	if (MESSAGES) qDebug("> newProcessor()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	if (MESSAGES) qDebug("= newProcessor(): Creating %s called %s. count = %d.", qPrintable(type), qPrintable(name), theProcessors.count());
	Processor *p = ProcessorFactory::create(type);
	if (p)
	{	if (MESSAGES) qDebug("= newProcessor(): Creaded; mapping... count = %d.", theProcessors.count());
		theProcessors[name] = p;
	}
	ret = p;
	if (MESSAGES) qDebug("< newProcessor(): Done. count = %d.", theProcessors.count());
	return true;
}

bool SessionServer::deleteProcessor(const QString &name)
{
	if (MESSAGES) qDebug("> deleteProcessor()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	if (!theProcessors.contains(name)) return false;
	Processor *p = theProcessors[name];
	if (MESSAGES) qDebug("= deleteProcessor(): Destroying. count = %d, processor = %p.", theProcessors.count(), p);
	delete p;
	if (MESSAGES) qDebug("= deleteProcessor(): Destroyed OK. count = %d.", theProcessors.count());
	theProcessors.remove(name);
	if (MESSAGES) qDebug("= deleteProcessor(): Removed entry. count = %d.", theProcessors.count());
	return true;
}

bool SessionServer::newDomProcessor(const QString &subType, const QString &name, bool &ret)
{
	if (MESSAGES) qDebug("> newDomProcessor()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	DomProcessor *p = SubProcessorFactory::createDom(subType);
	if (p) theProcessors[name] = p;
	ret = p;
	return true;
}

bool SessionServer::deleteDomProcessor(const QString &name)
{
	if (MESSAGES) qDebug("> deleteDomProcessor()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	if (!theProcessors.contains(name)) return false;
	DomProcessor *p = dynamic_cast<DomProcessor *>(theProcessors[name]);
	delete p;
	theProcessors.remove(name);
	return true;
}

bool SessionServer::domProcessorCreateAndAddLocal(const QString &name, bool &ret)
{
	if (MESSAGES) qDebug("> domProcessorCreateAndAddLocal()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	DomProcessor *p = dynamic_cast<DomProcessor *>(theProcessors[name]);
	if (!p) return false;
	ret = p->createAndAddWorker();
	return true;
}

bool SessionServer::domProcessorCreateAndAddNetwork(const QString &name, const QString &host, uint key, bool &ret)
{
	if (MESSAGES) qDebug("> domProcessorCreateAndAddNetwork()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	DomProcessor *p = dynamic_cast<DomProcessor *>(theProcessors[name]);
	if (!p) return false;
	ret = p->createAndAddWorker(host, key);
	return true;
}

bool SessionServer::processorInit(const QString &name, const Properties &properties, const QString &newName)
{
	if (MESSAGES) qDebug("> processorInit()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	theProcessors.remove(name);
	theProcessors[newName] = p;
	p->init(newName, properties);
	return true;
}

bool SessionServer::processorGo(const QString &name, bool &ret)
{
	if (MESSAGES) qDebug("> processorGo() [%s]", qPrintable(name));
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	if (MESSAGES) qDebug("< processorGo() [%s]", qPrintable(name));
	ret = p->go();
	return true;
}

bool SessionServer::processorWaitUntilGoing(const QString &name, int &errorData, int &ret)
{
	if (MESSAGES) qDebug("> processorwaitUntilGoing()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	ret = (int)p->waitUntilGoing(&errorData);
	return true;
}

bool SessionServer::processorWaitUntilDone(const QString &name)
{
	if (MESSAGES) qDebug("> processorwaitUntilDone()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	p->waitUntilDone();
	return true;
}

bool SessionServer::processorStop(const QString &name)
{
	if (MESSAGES) qDebug("> processorStop()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	p->stop();
	if (MESSAGES) qDebug("< processorStop(): Stopped!");
	return true;
}

bool SessionServer::processorReset(const QString &name)
{
	if (MESSAGES) qDebug("> processorReset()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	p->reset();
	if (MESSAGES) qDebug("< processorReset(): Stopped!");
	return true;
}

bool SessionServer::processorConnectLocal(const QString &name, uint bufferSize, uint output, const QString &destname, uint destinput, bool &ret)
{
	if (MESSAGES) qDebug("> processorConnectLocal(): name=%s", qPrintable(destname));
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	Processor *q = theProcessors[destname];
	if (!q) return false;
	ret = p->connect(output, q, destinput, bufferSize);
	return true;
}

bool SessionServer::processorConnectNetwork(const QString &name, uint bufferSize, uint output, const QString &desthost, uint destkey, const QString &destname, uint destinput, bool &ret)
{
	if (MESSAGES) qDebug("> processorConnectNetwork(): key=%d, host=%s, name=%s", destkey, qPrintable(desthost), qPrintable(destname));
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	ret = p->connect(output, desthost, destkey, destname, destinput, bufferSize);
	return true;
}

bool SessionServer::processorDisconnect(const QString &name, uint output)
{
	if (MESSAGES) qDebug("> processorDisconnect()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	p->disconnect(output);
	if (MESSAGES) qDebug("< processorDisconnect()");
	return true;
}

bool SessionServer::processorDisconnectAll(const QString &name)
{
	if (MESSAGES) qDebug("> processorDisconnectAll()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	p->disconnectAll();
	if (MESSAGES) qDebug("< processorDisconnectAll()");
	return true;
}

bool SessionServer::processorSplit(const QString &name, uint output)
{
	if (MESSAGES) qDebug("> processorSplit()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	p->split(output);
	return true;
}

bool SessionServer::processorShare(const QString &name, uint output)
{
	if (MESSAGES) qDebug("> processorShare()");
	QMutexLocker lock(&theCalling);
	theAlive = true;
	Processor *p = theProcessors[name];
	if (!p) return false;
	p->share(output);
	return true;
}

bool SessionServer::typeAvailable(const QString &id, bool &ret)
{
	QMutexLocker lock(&theCalling);
	theAlive = true;
	ret = ProcessorFactory::available(id);
	return true;
}

bool SessionServer::typeVersion(const QString &id, int &ret)
{
	QMutexLocker lock(&theCalling);
	theAlive = true;
	ret = ProcessorFactory::versionId(id);
	return true;
}

bool SessionServer::typeSubAvailable(const QString &id, bool &ret)
{
	QMutexLocker lock(&theCalling);
	theAlive = true;
	ret = SubProcessorFactory::available(id);
	return true;
}

bool SessionServer::typeSubVersion(const QString &id, int &ret)
{
	QMutexLocker lock(&theCalling);
	theAlive = true;
	ret = SubProcessorFactory::versionId(id);
	return true;
}

SessionServer *SessionServer::session(uint sessionKey)
{
	if (MESSAGES) qDebug("> session()");
	QMutexLocker lock(&mutSessionKeyMap);
	return theSessionKeyMap.count(sessionKey) ? theSessionKeyMap[sessionKey] : NULL;
}

};
