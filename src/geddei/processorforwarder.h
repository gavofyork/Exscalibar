/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _RGEDDEI_PROCESSORFORWARDER_H
#define _RGEDDEI_PROCESSORFORWARDER_H

#include <q3ptrlist.h>
#include <qmutex.h>
#include <qstring.h>
#include <q3serversocket.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "processor.h"
#else
#include <geddei/processor.h>
#endif
using namespace Geddei;

#define GEDDEI_PORT 16661

namespace Geddei
{

class DRCoupling;

/** @internal @ingroup Geddei
 * @brief A class for serving and routing incoming Processor connections.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * @note This class can ONLY ever be used when a QApplication object is running
 * in the main() thread. It's completely useless otherwise.
 */
class DLLEXPORT ProcessorForwarder: public Q3ServerSocket
{
	// Orderly (out of thread) remote connection deletion subsystem.
	// Needed because a connection cannot delete itself from its own thread.
	static QMutex *theReaper;
	static Q3PtrList<RLConnection> theGraveyard;
	static QMutex *reaper();
	void clearGraveyard();

	friend class ProcessorForwarderLink;

	//* Reimplementation from QServerSocket.
	virtual void newConnection(int socket);

	/**
	 * Subclass this method to derive processor from key and name.
	 * Will be different on basic UI than on node servers.
	 */
	virtual Processor *lookup(uint key, const QString &name) = 0;

	/**
	 * Initiates a connection with a remote ProcessorForwarder.
	 * Returns a new QSocketDevice * allocated on the heap. This is not owned
	 * by this method, and must be deleted by the caller.
	 * This point is moot though, since the returned QSD is adopted by a connection
	 * or coupling.
	 */
	static Q3SocketDevice *login(const QString &host, uint key);

public:
	/**
	 * Initiate a new connection.
	 * Code only vaguely belongs in this class, put here mainly to keep both ends on connection
	 * code close to each other (code originally from Processor).
	 * Method is static to show lack of home for code.
	 */
	static LRConnection *createConnection(Source *source, uint sourceIndex, uint bufferSize, const QString &sinkHost, uint sinkKey, const QString &sinkProcessorName, uint sinkIndex);

	/**
	 * Delete an existing connection.
	 * Simply calls disconnect on the given processor/input.
	 * Used for LRCs to kill their RLC ends.
	 */
	static bool deleteConnection(const QString &sinkHost, uint sinkKey, const QString &sinkProcessorName, uint sinkIndex);

	/**
	 * Delets a "child" RLConnection.
	 * Used by RLConnection objects to delete themselves safely from their own thread.
	 */
	static void deleteMeLater(RLConnection *me);

	/**
	 * Initiates creation of a DRC/RSC pair, returning the DRC and associating the RSC with the
	 * remote subprocessor described by @a host, @a key and @a subProcessorKey.
	 * The DomProcessor is given by @a dom.
	 */
	static DRCoupling *createCoupling(DomProcessor *dom, const QString &host, uint key, const QString &type);

	/**
	 * Initiates a remote deletion request on @a host with session @a key.
	 * The RSCoupling object resident with the SubProcessor of @a subProcessorKey will
	 * be deleted.
	 * This should be called *before* the accompanying DRCoupling object is fully deleted
	 * (and is can/should be indirectly called by the DRC destructor).
	 */
	static bool deleteCoupling(const QString &host, uint key, uint subProcessorKey);

	/**
	 * Simple constructor.
	 */
	ProcessorForwarder(uint port = 0);
};

};

#endif
