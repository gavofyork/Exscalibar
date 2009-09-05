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

#include <cstdlib>
#include <iostream>
using namespace std;

#include <qapplication.h>
#include <q3socketdevice.h>
#include <q3socket.h>
#include <qstring.h>

#ifdef Q_WS_X11
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#include "qcleaner.h"
#include "qsocketsession.h"

#include "commandcodes.h"
#include "properties.h"
using namespace Geddei;

#include "sessionserver.h"
#include "hostprocessorforwarder.h"
#include "commcodes.h"
using namespace rGeddei;

#define MESSAGES 0

class NodeServer;

class NodeServerSession: public QThread
{
	static QCleaner<NodeServerSession> theCleaner;
	
	QSocketSession theSource;
	SessionServer *theSession;
	NodeServer *theServer;
	
	virtual void run();
	
public:
	NodeServerSession(int socket, NodeServer *server, SessionServer *session): theSource(new Q3SocketDevice(socket, Q3SocketDevice::Stream)), theSession(session), theServer(server)
	{
		if (MESSAGES) qDebug("Creating session %p.", this);
		theSource.handshake(false);
		start();
	}
	
	~NodeServerSession()
	{
		if (MESSAGES) qDebug("Destroying session %p.", this);
	}
};

QCleaner<NodeServerSession> NodeServerSession::theCleaner(1);

class NodeServer : public Q3ServerSocket
{
	//* Reimplementation from QServerSocket.
	virtual void newConnection(int socket);

	friend class NodeServerSession;

public:
	NodeServer(uint port = RGEDDEI_PORT) : Q3ServerSocket(port) {}
};

void NodeServerSession::run()
{
	if (MESSAGES) qDebug("> NSS::run(): isOpen() = %d", theSource.isOpen());
	
	if (MESSAGES) qDebug("Sending key %d...", theSession->sessionKey());
	// Send the session id
	theSource.safeSendWord(theSession->sessionKey());
	
	if (MESSAGES) qDebug("Started session.");
	while (theSource.isOpen())
	{
		if (MESSAGES) qDebug("= NSS::run(): Receiving...");
		uchar command = theSource.receiveByte();
	
		if (MESSAGES) qDebug("= NSS::run(): Command attained. isOpen() = %d", theSource.isOpen());
		if (!theSource.isOpen()) break;
	
		if (MESSAGES) qDebug("= NSS::run(): command = %d", (int)command);
		switch (command)
		{
		case Nop:
			theSession->keepAlive();
			theSource.ack();
			break;
		case NewProcessor:
		{
			QString type = theSource.receiveString();
			QString name = theSource.receiveString();
			if (MESSAGES) qDebug("Creating Processor %s of type %s", name.latin1(), type.latin1());
			bool ret;
			bool ack = theSession->newProcessor(type, name, ret);
			theSource.ack(ret);
			theSource.ack(ack);
			break;
		}
		case DeleteProcessor:
		{
			QString name = theSource.receiveString();
			if (MESSAGES) qDebug("Deleting Processor %s", name.latin1());
			theSource.ack(theSession->deleteProcessor(name));
			break;
		}
		case NewDomProcessor:
		{
			QString type = theSource.receiveString();
			QString name = theSource.receiveString();
			if (MESSAGES) qDebug("Creating DomProcessor %s of SubProcessor type %s", name.latin1(), type.latin1());
			bool ret;
			bool ack = theSession->newDomProcessor(type, name, ret);
			theSource.ack(ret);
			theSource.ack(ack);
			break;
		}
		case DeleteDomProcessor:
		{
			QString name = theSource.receiveString();
			if (MESSAGES) qDebug("Destroying DomProcessor %s", name.latin1());
			theSource.ack(theSession->deleteDomProcessor(name));
			break;
		}
		case ProcessorInit:
		{
			QString name = theSource.receiveString();
			QByteArray a(theSource.safeReceiveWord<int>());
			theSource.receiveChunk((uchar *)a.data(), a.size());
			QString newName = theSource.receiveString();
			theSource.ack(theSession->processorInit(name, Properties(a), newName));
			break;
		}
		case ProcessorGo:
		{
			QString name = theSource.receiveString();
			if (MESSAGES) qDebug("Starting Processor %s", name.latin1());
			bool ret;
			bool ack = theSession->processorGo(name, ret);
			theSource.ack(ret);
			theSource.ack(ack);
			break;
		}
		case ProcessorWaitUntilGoing:
		{
			QString name = theSource.receiveString();
			int errorData, ret;
			int ack = theSession->processorWaitUntilGoing(name, errorData, ret);
			theSource.safeSendWord(errorData);
			theSource.safeSendWord(ret);
			theSource.safeSendWord(ack);
			break;
		}
		case ProcessorWaitUntilDone:
		{
			QString name = theSource.receiveString();
			theSource.ack(theSession->processorWaitUntilDone(name));
			break;
		}
		case ProcessorStop:
		{
				QString name = theSource.receiveString();
				theSource.ack(theSession->processorStop(name));
				break;
		}
		case ProcessorReset:
		{
				QString name = theSource.receiveString();
				theSource.ack(theSession->processorReset(name));
				break;
		}
		case ProcessorConnectL:
		{
			QString name = theSource.receiveString();
			if (MESSAGES) qDebug("Local Connecting Processor %s", name.latin1());
			int bufferSize = theSource.safeReceiveWord<int>();
			int output = theSource.safeReceiveWord<int>();
			QString destName = theSource.receiveString();
			int destInput = theSource.safeReceiveWord<int>();
			if (MESSAGES) qDebug("Connecting from %s:%d to %s:%d (size:%d)", name.latin1(), output, destName.latin1(), destInput, bufferSize);
			bool ret;
			bool ack = theSession->processorConnectLocal(name, bufferSize, output, destName, destInput, ret);
			theSource.ack(ret);
			theSource.ack(ack);
			break;
		}
		case ProcessorConnectR:
		{
			QString name = theSource.receiveString();
			int bufferSize = theSource.safeReceiveWord<int>();
			int output = theSource.safeReceiveWord<int>();
			QString destHost = theSource.receiveString();
			int destKey = theSource.safeReceiveWord<int>();
			QString destName = theSource.receiveString();
			int destInput = theSource.safeReceiveWord<int>();
			bool ret;
			bool ack = theSession->processorConnectNetwork(name, bufferSize, output, destHost, destKey, destName, destInput, ret);
			theSource.ack(ret);
			theSource.ack(ack);
			break;
		}
		case ProcessorDisconnect:
		{
			QString name = theSource.receiveString();
			int output = theSource.safeReceiveWord<int>();
			theSource.ack(theSession->processorDisconnect(name, output));
			break;
		}
		case ProcessorDisconnectAll:
		{
			QString name = theSource.receiveString();
			theSource.ack(theSession->processorDisconnectAll(name));
			break;
		}
		case ProcessorSplit:
		{
			QString name = theSource.receiveString();
			int output = theSource.safeReceiveWord<int>();
			theSource.ack(theSession->processorSplit(name, output));
			break;
		}
		case ProcessorShare:
		{
			QString name = theSource.receiveString();
			int output = theSource.safeReceiveWord<int>();
			theSource.ack(theSession->processorShare(name, output));
			break;
		}
		case DomProcessorCreateAndAddL:
		{
			QString name = theSource.receiveString();
			bool ret;
			bool ack = theSession->domProcessorCreateAndAddLocal(name, ret);
			theSource.ack(ret);
			theSource.ack(ack);
			break;
		}
		case DomProcessorCreateAndAddR:
		{
			QString name = theSource.receiveString();
			QString destHost = theSource.receiveString();
			int destKey = theSource.safeReceiveWord<int>();
			bool ret;
			bool ack = theSession->domProcessorCreateAndAddNetwork(name, destHost, destKey, ret);
			theSource.ack(ret);
			theSource.ack(ack);
			break;
		}
		case TypeAvailable:
		{
			QString type = theSource.receiveString();
			bool ret;
			bool ack = theSession->typeAvailable(type, ret);
			theSource.ack(ret);
			theSource.ack(ack);
			break;
		}
		case TypeVersion:
		{
			QString type = theSource.receiveString();
			int ret;
			bool ack = theSession->typeVersion(type, ret);
			theSource.safeSendWord(ret);
			theSource.ack(ack);
			break;
		}
		case TypeSubAvailable:
		{
			QString type = theSource.receiveString();
			bool ret;
			bool ack = theSession->typeSubAvailable(type, ret);
			theSource.ack(ret);
			theSource.ack(ack);
			break;
		}
		case TypeSubVersion:
		{
			QString type = theSource.receiveString();
			int ret;
			bool ack = theSession->typeSubVersion(type, ret);
			theSource.safeSendWord(ret);
			theSource.ack(ack);
			break;
		}
		case EndSession:
			theSource.ack();
			theSource.close();
			break;
		}
	}
	if (MESSAGES) qDebug("Session closed. Exiting...");
	SessionServer::safeDelete(theSession->sessionKey());
	theCleaner.deleteObject(this);
}

void NodeServer::newConnection(int socket)
{
	if (MESSAGES) qDebug("Got new connection on socket %d - Creating server", socket);
	SessionServer *s = new SessionServer;
	if (MESSAGES) qDebug("OK. Putting it into object");
	new NodeServerSession(socket, this, s);
	if (MESSAGES) qDebug("All sorted.");
}

int main(int argc, char **argv)
{
#ifdef Q_WS_X11
	struct rlimit s;
	getrlimit(RLIMIT_NOFILE, &s);
	if (s.rlim_cur < 1024)
		qWarning("*** WARNING: Your system resource limit for open files is low. This"
				 "             means that class Geddei will only be able to have roughly %d network"
				 "             connections in total (for all users). If more are needed, then"
				 "             ask your sysadmin to increase the hard limit imposed by the OS.", (int)s.rlim_cur);

#endif
	uint rGPort = RGEDDEI_PORT, GPort = GEDDEI_PORT;
	
	for (int i = 1; i < argc; i++)
		if ((QString(argv[i]) == "--geddei-port" || QString(argv[i]) == "-p") && (i + 1) < argc)
			GPort = QString(argv[++i]).toInt();
		else if ((QString(argv[i]) == "--rgeddei-port" || QString(argv[i]) == "-P") && (i + 1) < argc)
			rGPort = QString(argv[++i]).toInt();
		else
		{	std::cout << "Usage: nodeserver [(-p|--geddei-port) <port>] [(-P|--rgeddei-port) <port>]" << std::endl;
			exit(0);
		}
	
	QApplication a(argc, argv, false);
	HostProcessorForwarder hPF(GPort);
	NodeServer server(rGPort);
	a.exec();
	return 0;
}
