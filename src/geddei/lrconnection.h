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

#pragma once

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
	virtual Type const& type() const;

	//* Reimplementations from LxConnection
	virtual bool waitUntilReady();
	virtual Tristate isReadyYet();
	virtual void setType(Type const& _type);
	virtual void resetType();
	virtual void sourceStopping();
	virtual void sourceStopped();
	virtual void reset() {}
	virtual void pushPlunger();
	virtual void startPlungers();
	virtual void plungerSent();
	virtual void noMorePlungers();
	virtual uint freeInDestinationBuffer(uint) { return Undefined; }
	virtual uint freeInDestinationBufferEver() { return Undefined; }
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

}
