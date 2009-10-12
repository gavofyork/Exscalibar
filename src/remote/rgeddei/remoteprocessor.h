/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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

#include <qstring.h>

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "remotesession.h"
#include "processor.h"
#include "processorfactory.h"
#include "abstractprocessorgroup.h"
#include "abstractprocessor.h"
#else
#include <rgeddei/remotesession.h>
#include <rgeddei/abstractprocessorgroup.h>
#include <rgeddei/abstractprocessor.h>
#include <geddei/processor.h>
#include <geddei/processorfactory.h>
#endif

using namespace Geddei;
using namespace rGeddei;

namespace rGeddei
{

class LocalProcessor;
class LocalDomProcessor;
class RemoteDomProcessor;

/** @ingroup rGeddei
 * @brief AbstractProcessor realisation representing a remote Processor object.
 * @author Gav Wood <gav@kde.org>
 *
 * This class is used when a Geddei Processor object is to be administered
 * through a RemoteSession. It allows the user to remotely and trasnparently
 * create, destroy, start, stop, connect and otherwise control a Geddei
 * Processor object that resides in a different process space, potentially on a
 * different machine.
 *
 * Typically, it is used very similarly to a Geddei Processor object.
 */
class DLLEXPORT RemoteProcessor: virtual public AbstractProcessor
{
	friend class RemoteSession;
	friend class LocalProcessor;
	friend class LocalDomProcessor;
	friend class RemoteDomProcessor;
	friend class AbstractProcessor;

	RemoteSession *theSession;
	QString theHandle, theType, theName;
	AbstractProcessorGroup *theGroup;

	/**
	 * Check that this RemoteProcessor object does actually represent a real
	 * Processor object at the other end.
	 *
	 * @return true if this is a valid Processor object representation.
	 */
	inline bool isValid() const { return theType != ""; }

	/**
	 * Basic constructor. Constructs a RemoteProcessor object together with a
	 * Processor object in the RemoteSession (i.e. on that object's host).
	 *
	 * @param session The RemoteSession under which to create the Processor
	 * object.
	 * @param type The Processor-derived subclass to be created. You should
	 * check the availability and version with the RemoteSession object before
	 * construction.
	 */
	RemoteProcessor(RemoteSession &session, const QString &type);

protected:
	/**
	 * Advanced constructor for RemoteDomProcessor to allow custom object
	 * construction.
	 *
	 * @param session The RemoteSession ovedr which to create this Processor
	 * object.
	 */
	RemoteProcessor(RemoteSession &session);

	//* Reimplementations from AbstractProcessor.
private:
	virtual void doInit(const QString &name, AbstractProcessorGroup *g, const Properties &p);
public:
	virtual bool go() { return theSession->processorGo(theHandle); }
	virtual Processor::ErrorType waitUntilGoing(int *errorData = 0) { int ed; return (Processor::ErrorType)theSession->processorWaitUntilGoing(theHandle, errorData ? *errorData : ed); }
	virtual void waitUntilDone() { theSession->processorWaitUntilDone(theHandle); }
	virtual void stop() { theSession->processorStop(theHandle); }
	virtual void reset() { theSession->processorReset(theHandle); }
	virtual void split(uint sourceIndex) { theSession->processorSplit(theHandle, sourceIndex); }
	virtual void share(uint sourceIndex) { theSession->processorShare(theHandle, sourceIndex); }
	virtual bool connect(uint sourceIndex, const RemoteProcessor *sink, uint sinkIndex, uint bufferSize = 1);
	virtual bool connect(uint sourceIndex, const LocalProcessor *sink, uint sinkIndex, uint bufferSize = 1);
	virtual void disconnect(uint sourceIndex) { theSession->processorDisconnect(theHandle, sourceIndex); }
	virtual void disconnectAll() { theSession->processorDisconnectAll(theHandle); }
	virtual const QString name() const { return theName; }

	/**
	 * Default destructor.
	 */
	virtual ~RemoteProcessor();
};

}
