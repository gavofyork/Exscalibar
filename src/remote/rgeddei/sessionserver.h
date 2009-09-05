/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _RGEDDEI_HOSTSESSION_H
#define _RGEDDEI_HOSTSESSION_H

#include <qmutex.h>
#include <qmap.h>
#include <qstring.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "processorgroup.h"
#include "processor.h"
#include "properties.h"
#include "buffer.h"
#else
#include <geddei/processorgroup.h>
#include <geddei/processor.h>
#include <geddei/properties.h>
#include <geddei/buffer.h>
#endif
using namespace Geddei;

namespace rGeddei
{

/** @internal @ingroup rGeddei
 * @brief Remote session server.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * This class is responsible for hosting a session on the server-side of the
 * rGeddei connection. It essentially functions as a control actuator, taking
 * commands through a well-defined interface (currently CORBA, but this is
 * under review) and carrying them out with its cache of *Processor objects.
 *
 * It is a singleton object, thus once used anywhere in a program will exist
 * throughout. It can host a number of individual sessions, each accessible
 * only by a session key, transmitted to the client at session initialisation.
 * It thus provides a certain degree of security.
 *
 * This class is unlikely to be used by anything other than the nodeserver
 * application, however it is kept in the rGeddei module in order to keep it
 * close to its logical partner, the client part of the link, RemoteSession.
 */
class DLLEXPORT SessionServer
{
	static QMap<uint, SessionServer *> theSessionKeyMap;
	static QMutex mutSessionKeyMap;
	class SessionServerReaper: public QThread { virtual void run(); };
	// Created and started on (first) call to SessionServer::SessionServer
	static SessionServerReaper *theReaper;
	static void reap();

	uint theSessionKey;
	QMap<QString, Processor *> theProcessors;
	bool theAlive, theReaping;
	mutable QMutex theCalling;

	void setAlive();
	void resetAlive();
	bool alive() const;

public:
	// implementations of rpcs
	bool newProcessor(const QString &type, const QString &name, bool &ret);
	bool deleteProcessor(const QString &name);
	bool newDomProcessor(const QString &subType, const QString &name, bool &ret);
	bool deleteDomProcessor(const QString &name);
	bool processorInit(const QString &name, const Properties &p, const QString &newName);
	bool processorGo(const QString &name, bool &ret);
	bool processorWaitUntilGoing(const QString &name, int &errorData, int &ret);
	bool processorWaitUntilDone(const QString &name);
	bool processorStop(const QString &name);
	bool processorReset(const QString &name);
	bool processorConnectNetwork(const QString &name, uint bufferSize, uint output, const QString &desthost, uint destkey, const QString &destname, uint destinput, bool &ret);
	// to be finalised - need some way of identifying sockets from keys
	bool processorConnectSocket(const QString &name, uint bufferSize, uint output, uint destkey, const QString &destname, uint destinput, bool &ret);
	bool processorConnectLocal(const QString &name, uint bufferSize, uint output, const QString &destname, uint destinput, bool &ret);
	bool processorDisconnect(const QString &name, uint output);
	bool processorDisconnectAll(const QString &name);
	bool processorSplit(const QString &name, uint output);
	bool processorShare(const QString &name, uint output);
	bool domProcessorCreateAndAddLocal(const QString &name, bool &ret);
	bool domProcessorCreateAndAddNetwork(const QString &name, const QString &host, uint key, bool &ret);
	bool typeAvailable(const QString &id, bool &ret);
	bool typeVersion(const QString &id, int &ret);
	bool typeSubAvailable(const QString &id, bool &ret);
	bool typeSubVersion(const QString &id, int &ret);
	void keepAlive() { setAlive(); }

	inline Processor *getProcessor(const QString &name) { return theProcessors.contains(name) ? theProcessors[name] : 0; }
	inline uint sessionKey() { return theSessionKey; }
	static SessionServer *session(uint sessionKey);

	/**
	 * Even if this is called by two thread concurrently, the given session's object will
	 * only be deleted once.
	 * This should always be used instead of a "delete session(key);" statement, since
	 * otherwise there are concurrency issues.
	 */
	static void safeDelete(uint sessionKey);

	SessionServer();
	~SessionServer();
};

};

#endif
