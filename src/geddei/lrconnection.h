/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_LRCONNECTION_H
#define _GEDDEI_LRCONNECTION_H

#include <qmutex.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "qsocketsession.h"
#include "lxconnectionreal.h"
#else
#include <qtextra/qsocketsession.h>
#include <geddei/lxconnectionreal.h>
#endif
using namespace Geddei;

class QTcpSocket;

namespace Geddei
{

class ProcessorForwarder;

/** @internal @ingroup Geddei
 * @brief Embodiment of Connection between a Source and a remote socket.
 * @author Gav Wood <gav@kde.org>
 *
 * A realisation of the Connection flow-control class framework.
 * This shunts data from a processor object, to a network socket, along with any control
 * signals neccessary (such as for type identification/synchronisation).
 */
class DLLEXPORT LRConnection: public LxConnectionReal
{
	//* Reimplementations from Connection
	virtual const SignalTypeRef type();

	//* Reimplementations from LxConnection
	virtual bool waitUntilReady();
	virtual Tristate isReadyYet();
	virtual void setType(const SignalType *type);
	virtual void resetType();
	virtual void sourceStopping();
	virtual void sourceStopped();
	virtual void reset() {}
	virtual void pushPlunger();
	virtual void startPlungers();
	virtual void plungerSent();
	virtual void noMorePlungers();
	virtual uint maximumScratchElements(const uint) { return Undefined; }
	virtual uint maximumScratchElementsEver() { return Undefined; }
	virtual void enforceMinimum(uint elements);

	//* Reimplementations from LxConnectionReal
	virtual void transport(const BufferData &data);
	virtual void bufferWaitForFree();
	virtual uint bufferElementsFree();

	QString theRemoteHost, theRemoteProcessorName;
	uint theRemoteKey, theRemoteIndex;

	QSocketSession theSink;
	QFastMutex theTrapdoor;
	void openTrapdoor() { theTrapdoor.lock(); }
	void closeTrapdoor() { theTrapdoor.unlock(); }
	bool trapdoor() { bool ret = theTrapdoor.tryLock(); if (ret) theTrapdoor.unlock(); return !ret; }

	/**
	 * Simple constructor.
	 * Used from ProcessorForwarder object.
	 */
	friend class ProcessorForwarder;
	LRConnection(Source *newSource, uint newSourceIndex, QTcpSocket *newSink);

	/**
	 * Simple destructor.
	 */
	~LRConnection();

public:
	void setCredentials(const QString &remoteHost, uint remoteKey, const QString &remoteProcessorName, uint remoteIndex);
};

};

#endif
