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

#include <qstring.h>
#include <q3socketdevice.h>
#include <qtextcodec.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3PtrList>

#include "subprocessorfactory.h"
#include "drcoupling.h"
#include "rscoupling.h"
#include "lrconnection.h"
#include "qsocketsession.h"
using namespace Geddei;

#include "processorforwarder.h"

#define MESSAGES 0

namespace Geddei
{

QMutex *ProcessorForwarder::theReaper;
Q3PtrList<RLConnection> ProcessorForwarder::theGraveyard;

ProcessorForwarder::ProcessorForwarder(const uint port) : Q3ServerSocket(port ? port : GEDDEI_PORT )
{
	if(MESSAGES) qDebug("Starting server on port: %d.", port ? port : GEDDEI_PORT);
}

QMutex *ProcessorForwarder::reaper()
{
	if(!theReaper)
	{	theReaper = new QMutex;
		theGraveyard.setAutoDelete(true);
	}
	return theReaper;
}

void ProcessorForwarder::deleteMeLater(RLConnection *me)
{
	QMutexLocker lock(reaper());
	theGraveyard.append(me);
}

void ProcessorForwarder::newConnection(int socket)
{
	if(MESSAGES) qDebug("> newConnection()");
	clearGraveyard();
	if(MESSAGES) qDebug("= newConnection(): Graveyard cleared. Creating socket device");

	// SocketDevice must be a pointer since we need to pass it to RLC and it will
	// get closed on destruction, which would be a problem if stiored on the stack.
	// We give ownership of the link to RLConnection (though we, in fact, sort of
	// keep ownership of the RLC anyways. RLC will look after its deletion and
	// we sort of look after the deletion of the RLC.
	Q3SocketDevice *link = new Q3SocketDevice(socket, Q3SocketDevice::Stream);
	if(MESSAGES) qDebug("= newConnection(): Created. Creating stream and encoding.");

	{
		Q3TextStream header(link);
		header.setEncoding(Q3TextStream::Latin1);

		if(MESSAGES) qDebug("= newConnection(): Done. Reading key...");
		uint key = header.readLine().toUInt();
		if(MESSAGES) qDebug("Received key: %d.", key);
		QString command = header.readLine();
		if(command == "connect")
		{
			QString procName = header.readLine();
			if(MESSAGES) qDebug("Received proc name: %s.", procName.latin1());
			Processor *processor = lookup(key, procName);
			if(MESSAGES) qDebug("Processor is %p", processor);
			int input = header.readLine().toInt();
			uint bufferSize = header.readLine().toUInt();

			// TODO: check if already connected - do something if it is
			if(!processor)
			{	qWarning("*** ERROR: Invalid connection header---Processor reference or key invalid.\n"
						 "           (processor=%p, key=%d)", processor, key);
				return;
			}
			new RLConnection(link, processor, input, bufferSize);
			// return here to make sure that link isn't deleted.
			return;
		}
		else if(command == "disconnect")
		{
			QString procName = header.readLine();
			if(MESSAGES) qDebug("Received proc name: %s.", procName.latin1());
			// Need QMutexLocker for the group here.
			Processor *processor = lookup(key, procName);
			if(MESSAGES) qDebug("Processor is %p", processor);
			int input = header.readLine().toInt();
			if(!processor)
			{	qWarning("*** ERROR: Invalid connection header---Processor reference or key invalid.\n"
						 "           (processor=%p, key=%d)", processor, key);
				return;
			}
			processor->dropInput(input);
			// can end here
			header << "OK" << endl;
		}
		else if(command == "couple")
		{
			if(MESSAGES) qDebug("Got COUPLE command:");
			// Create a subProc, then create a RSCoupling. Associate them.
			QString type = header.readLine();
			if(MESSAGES) qDebug("Received proc type: %s", type.latin1());
			SubProcessor *sub = SubProcessorFactory::create(type);
			if(MESSAGES) qDebug("Created SubProcessor at %p", sub);
			// Return the subProc's pointer for decoupling later...
			header << ((uint)sub) << endl;
			new RSCoupling(link, sub);
			// return here to make sure that link isn't deleted.
			return;
		}
		else if(command == "decouple")
		{
			if(MESSAGES) qDebug("Got DECOUPLE command:");
			// grab the subProc pointer.
			SubProcessor *sub = (SubProcessor *)(header.readLine().toUInt());
			RSCoupling *coupling = dynamic_cast<RSCoupling *>(sub->coupling());
			if(MESSAGES) qDebug("Deleting SubProcessor at %p...", sub);
			delete sub;
			if(MESSAGES) qDebug("Deleting RSCoupling at %p...", coupling);
			delete coupling;
			header << "OK" << endl;
		}
	}
	link->close();
	delete link;
}

LRConnection *ProcessorForwarder::createConnection(Source *source, const uint sourceIndex, const uint bufferSize, const QString &sinkHost, const uint sinkKey, const QString &sinkProcessorName, const uint sinkIndex)
{
	LRConnection *ret;
	Q3SocketDevice *link = new Q3SocketDevice;
	if(MESSAGES) qDebug("> ProcessorForwarder::createConnection() : sinkHost = %s", sinkHost.latin1());
	if(!link->connect(QHostAddress(sinkHost), sinkKey < 65536 ? sinkKey : GEDDEI_PORT))
	{	qWarning("*** ERROR: Couldn't connect to sink host (%s). Code %d.", sinkHost.latin1(), (int)link->error());
		return 0;
	}
	else
	{	Q3TextStream header(link);
		if(MESSAGES) qDebug("Setting codec...");
		header.setEncoding(Q3TextStream::Latin1);
		if(MESSAGES) qDebug("Sending credentials (key=%d, name=%s)", sinkKey, sinkProcessorName.latin1());
		header << sinkKey << endl << "connect" << endl << sinkProcessorName << endl << sinkIndex << endl << bufferSize << endl;
		if(MESSAGES) qDebug("Sent. Creating LRC...");
		ret = new LRConnection(source, sourceIndex, link);
		ret->setCredentials(sinkHost, sinkKey, sinkProcessorName, sinkIndex);
		if(MESSAGES) qDebug("Done. Exiting.");
	}
	return ret;
}

const bool ProcessorForwarder::deleteConnection(const QString &sinkHost, const uint sinkKey, const QString &sinkProcessorName, const uint sinkIndex)
{
	Q3SocketDevice link;
	if(MESSAGES) qDebug("> ProcessorForwarder::deleteConnection() : sinkHost = %s", sinkHost.latin1());
	if(!link.connect(QHostAddress(sinkHost), sinkKey < 65536 ? sinkKey : GEDDEI_PORT))
	{	qWarning("*** ERROR: Couldn't connect to sink host (%s). Code %d.", sinkHost.latin1(), (int)link.error());
		return false;
	}
	Q3TextStream header(&link);
	if(MESSAGES) qDebug("Setting codec...");
	header.setEncoding(Q3TextStream::Latin1);
	if(MESSAGES) qDebug("Sending credentials (key=%d, name=%s)", sinkKey, sinkProcessorName.latin1());
	header << sinkKey << endl << "disconnect" << endl << sinkProcessorName << endl << sinkIndex << endl;
	if(MESSAGES) qDebug("Done. Verifying...");
	return header.readLine() == "OK";
}

Q3SocketDevice *ProcessorForwarder::login(const QString &host, const uint key)
{
	Q3SocketDevice *link = new Q3SocketDevice;
	if(MESSAGES) qDebug("> ProcessorForwarder::login() : host = %s, key = %d", host.latin1(), key);
	if(!link->connect(QHostAddress(host), key < 65536 ? key : GEDDEI_PORT))
	{	qWarning("*** ERROR: Couldn't connect to sink host (%s). Code %d.", host.latin1(), (int)link->error());
		return 0;
	}
	if(MESSAGES) qDebug("< ProcessorForwarder::login() : Logged in OK");
	return link;
}

DRCoupling *ProcessorForwarder::createCoupling(DomProcessor *dom, const QString &host, const uint key, const QString &type)
{
	Q3SocketDevice *link = login(host, key);
	if(!link) return 0;
	DRCoupling *ret;
	Q3TextStream header(link);
	if(MESSAGES) qDebug("Setting codec...");
	header.setEncoding(Q3TextStream::Latin1);
	if(MESSAGES) qDebug("Sending credentials (key=%d, type=%s)", key, type.latin1());
	header << key << endl << "couple" << endl << type << endl;
	if(MESSAGES) qDebug("Sent. Reading subProcKey...");
	const uint sPK = header.readLine().toUInt();
	if(MESSAGES) qDebug("Got %d. Creating DRC...", sPK);
	ret = new DRCoupling(dom, link);
	ret->setCredentials(host, key, sPK);
	if(MESSAGES) qDebug("Done. Exiting.");
	return ret;
}

const bool ProcessorForwarder::deleteCoupling(const QString &host, const uint key, const uint sPK)
{
	Q3SocketDevice *link = login(host, key);
	if(!link) return false;
	Q3TextStream header(link);
	if(MESSAGES) qDebug("Setting codec...");
	header.setEncoding(Q3TextStream::Latin1);
	if(MESSAGES) qDebug("Sending credentials (key=%d, subProcKey=%d)", key, sPK);
	header << key << endl << "decouple" << endl << sPK << endl;
	if(MESSAGES) qDebug("Done. Verifying...");
	bool ret = header.readLine() == "OK";
	delete link;
	return ret;
}

void ProcessorForwarder::clearGraveyard()
{
	QMutexLocker lock(reaper());
	theGraveyard.clear();
}

};
