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

#include <QString>
#include <QTextCodec>
#include <QTextStream>

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

QFastMutex *ProcessorForwarder::theReaper;
QList<RLConnection*> ProcessorForwarder::theGraveyard;

ProcessorForwarder::ProcessorForwarder(uint port)
{
	listen(QHostAddress::LocalHost, port ? port : GEDDEI_PORT);
	if (MESSAGES) qDebug("Starting server on port: %d.", port ? port : GEDDEI_PORT);
}

ProcessorForwarder::~ProcessorForwarder()
{
	while (theGraveyard.size())
		delete theGraveyard.takeLast();
}

QFastMutex *ProcessorForwarder::reaper()
{
	if (!theReaper)
		theReaper = new QFastMutex;
	return theReaper;
}

void ProcessorForwarder::deleteMeLater(RLConnection *me)
{
	QFastMutexLocker lock(reaper());
	theGraveyard.append(me);
}

void ProcessorForwarder::incomingConnection(int socket)
{
	if (MESSAGES) qDebug("> newConnection()");
	clearGraveyard();
	if (MESSAGES) qDebug("= newConnection(): Graveyard cleared. Creating socket device");

	// SocketDevice must be a pointer since we need to pass it to RLC and it will
	// get closed on destruction, which would be a problem if stiored on the stack.
	// We give ownership of the link to RLConnection (though we, in fact, sort of
	// keep ownership of the RLC anyways. RLC will look after its deletion and
	// we sort of look after the deletion of the RLC.
	QTcpSocket *link = new QTcpSocket;
	link->setSocketDescriptor(socket);
	if (MESSAGES) qDebug("= newConnection(): Created. Creating stream and encoding.");

	{
		QTextStream header(link);
		header.setCodec("UTF-8");

		if (MESSAGES) qDebug("= newConnection(): Done. Reading key...");
		uint key = header.readLine().toUInt();
		if (MESSAGES) qDebug("Received key: %d.", key);
		QString command = header.readLine();
		if (command == "connect")
		{
			QString procName = header.readLine();
			if (MESSAGES) qDebug("Received proc name: %s.", qPrintable(procName));
			Processor *processor = lookup(key, procName);
			if (MESSAGES) qDebug("Processor is %p", processor);
			int input = header.readLine().toInt();
			uint bufferSize = header.readLine().toUInt();

			// TODO: check if already connected - do something if it is
			if (!processor)
			{	qWarning("*** ERROR: Invalid connection header---Processor reference or key invalid.\n"
						 "           (processor=%p, key=%d)", processor, key);
				return;
			}
			new RLConnection(link, processor, input, bufferSize);
			// return here to make sure that link isn't deleted.
			return;
		}
		else if (command == "disconnect")
		{
			QString procName = header.readLine();
			if (MESSAGES) qDebug("Received proc name: %s.", qPrintable(procName));
			// Need QFastMutexLocker for the group here.
			Processor *processor = lookup(key, procName);
			if (MESSAGES) qDebug("Processor is %p", processor);
			int input = header.readLine().toInt();
			if (!processor)
			{	qWarning("*** ERROR: Invalid connection header---Processor reference or key invalid.\n"
						 "           (processor=%p, key=%d)", processor, key);
				return;
			}
			processor->dropInput(input);
			// can end here
			header << "OK" << endl;
		}
		else if (command == "couple")
		{
			if (MESSAGES) qDebug("Got COUPLE command:");
			// Create a subProc, then create a RSCoupling. Associate them.
			QString type = header.readLine();
			if (MESSAGES) qDebug("Received proc type: %s", qPrintable(type));
			SubProcessor *sub = SubProcessorFactory::create(type);
			if (MESSAGES) qDebug("Created SubProcessor at %p", sub);
			// Return the subProc's pointer for decoupling later...
			header << ((long uint)sub) << endl;
			new RSCoupling(link, sub);
			// return here to make sure that link isn't deleted.
			return;
		}
		else if (command == "decouple")
		{
			if (MESSAGES) qDebug("Got DECOUPLE command:");
			// grab the subProc pointer.
			SubProcessor *sub = (SubProcessor *)(header.readLine().toUInt());
			RSCoupling *coupling = dynamic_cast<RSCoupling *>(sub->coupling());
			if (MESSAGES) qDebug("Deleting SubProcessor at %p...", sub);
			delete sub;
			if (MESSAGES) qDebug("Deleting RSCoupling at %p...", coupling);
			delete coupling;
			header << "OK" << endl;
		}
	}
	link->close();
	delete link;
}

LRConnection *ProcessorForwarder::createConnection(Source *source, uint sourceIndex, uint bufferSize, const QString &sinkHost, uint sinkKey, const QString &sinkProcessorName, uint sinkIndex)
{
	LRConnection *ret;
	QTcpSocket *link = new QTcpSocket;
	if (MESSAGES) qDebug("> ProcessorForwarder::createConnection() : sinkHost = %s", qPrintable(sinkHost));
	link->connectToHost(QHostAddress(sinkHost), sinkKey < 65536 ? sinkKey : GEDDEI_PORT);
	if (!link->waitForConnected())
	{	qWarning("*** ERROR: Couldn't connect to sink host (%s). Code %d.", qPrintable(sinkHost), (int)link->error());
		return 0;
	}
	else
	{	QTextStream header(link);
		if (MESSAGES) qDebug("Setting codec...");
		header.setCodec("UTF-8");
		if (MESSAGES) qDebug("Sending credentials (key=%d, name=%s)", sinkKey, qPrintable(sinkProcessorName));
		header << sinkKey << endl << "connect" << endl << sinkProcessorName << endl << sinkIndex << endl << bufferSize << endl;
		if (MESSAGES) qDebug("Sent. Creating LRC...");
		ret = new LRConnection(source, sourceIndex, link);
		ret->setCredentials(sinkHost, sinkKey, sinkProcessorName, sinkIndex);
		if (MESSAGES) qDebug("Done. Exiting.");
	}
	return ret;
}

bool ProcessorForwarder::deleteConnection(const QString &sinkHost, uint sinkKey, const QString &sinkProcessorName, uint sinkIndex)
{
	QTcpSocket link;
	if (MESSAGES) qDebug("> ProcessorForwarder::deleteConnection() : sinkHost = %s", qPrintable(sinkHost));
	link.connectToHost(QHostAddress(sinkHost), sinkKey < 65536 ? sinkKey : GEDDEI_PORT);
	if (!link.waitForConnected())
	{	qWarning("*** ERROR: Couldn't connect to sink host (%s). Code %d.", qPrintable(sinkHost), (int)link.error());
		return false;
	}
	QTextStream header(&link);
	if (MESSAGES) qDebug("Setting codec...");
	header.setCodec("UTF-8");
	if (MESSAGES) qDebug("Sending credentials (key=%d, name=%s)", sinkKey, qPrintable(sinkProcessorName));
	header << sinkKey << endl << "disconnect" << endl << sinkProcessorName << endl << sinkIndex << endl;
	if (MESSAGES) qDebug("Done. Verifying...");
	return header.readLine() == "OK";
}

QTcpSocket *ProcessorForwarder::login(const QString &host, uint key)
{
	QTcpSocket *link = new QTcpSocket;
	if (MESSAGES) qDebug("> ProcessorForwarder::login() : host = %s, key = %d", qPrintable(host), key);
	link->connectToHost(QHostAddress(host), key < 65536 ? key : GEDDEI_PORT);
	if (!link->waitForConnected())
	{	qWarning("*** ERROR: Couldn't connect to sink host (%s). Code %d.", qPrintable(host), (int)link->error());
		return 0;
	}
	if (MESSAGES) qDebug("< ProcessorForwarder::login() : Logged in OK");
	return link;
}

DRCoupling *ProcessorForwarder::createCoupling(DomProcessor *dom, const QString &host, uint key, const QString &type)
{
	QTcpSocket *link = login(host, key);
	if (!link) return 0;
	DRCoupling *ret;
	QTextStream header(link);
	if (MESSAGES) qDebug("Setting codec...");
	header.setCodec("UTF-8");
	if (MESSAGES) qDebug("Sending credentials (key=%d, type=%s)", key, qPrintable(type));
	header << key << endl << "couple" << endl << type << endl;
	if (MESSAGES) qDebug("Sent. Reading subProcKey...");
	uint sPK = header.readLine().toUInt();
	if (MESSAGES) qDebug("Got %d. Creating DRC...", sPK);
	ret = new DRCoupling(dom, link);
	ret->setCredentials(host, key, sPK);
	if (MESSAGES) qDebug("Done. Exiting.");
	return ret;
}

bool ProcessorForwarder::deleteCoupling(const QString &host, uint key, uint sPK)
{
	QTcpSocket *link = login(host, key);
	if (!link) return false;
	QTextStream header(link);
	if (MESSAGES) qDebug("Setting codec...");
	header.setCodec("UTF-8");
	if (MESSAGES) qDebug("Sending credentials (key=%d, subProcKey=%d)", key, sPK);
	header << key << endl << "decouple" << endl << sPK << endl;
	if (MESSAGES) qDebug("Done. Verifying...");
	bool ret = header.readLine() == "OK";
	delete link;
	return ret;
}

void ProcessorForwarder::clearGraveyard()
{
	QFastMutexLocker lock(reaper());
	theGraveyard.clear();
}

}

#undef MESSAGES
