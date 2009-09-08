/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_LMCONNECTION_H
#define _GEDDEI_LMCONNECTION_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "buffer.h"
#include "lxconnectionreal.h"
#else
#include <geddei/buffer.h>
#include <geddei/lxconnectionreal.h>
#endif
using namespace Geddei;

namespace Geddei
{

class Processor;
class MLConnection;

/** @internal @ingroup Geddei
 * @brief Embodiment of Connection between local Source and multiple outputs.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * A refinement of a flow control class for outbound communication.
 * This class implements a single-in, multiple-out connection paradigm.
 */
class DLLEXPORT LMConnection: public LxConnectionReal
{
	friend class MLConnection;
	friend class Processor;

	//* Reimplementations from Connection
	virtual const SignalTypeRef type();

	//* Reimplementations from LxConnection
	virtual bool waitUntilReady();
	virtual void setType(const SignalType *type);
	virtual void resetType();
	virtual void sourceStopping();
	virtual void sourceStopped();
	virtual void reset();
	virtual BufferData makeScratchElements(uint elements, bool autoPush = false);
	virtual void pushPlunger();
	virtual void startPlungers();
	virtual void plungerSent();
	virtual void noMorePlungers();
	virtual uint maximumScratchElements(uint minimum = 1);
	virtual uint maximumScratchElementsEver();
	virtual void enforceMinimum(uint elements);

	//* Reimplementations from LxConnectionReal
	virtual void bufferWaitForFree();
	virtual uint bufferElementsFree();
	virtual void transport(const BufferData &data);

	/**
	 * Relays an openTrapdoor call to the Buffer object.
	 *
	 * This is used by the MLConnection class as it is more robust than relying
	 * on the BufferReader object to be existant (which, in the case of being
	 * connected to a DomProcessor object it would not be).
	 *
	 * @param sink The Processor object that the trapdoor must be opened for.
	 */
	void openBufferTrapdoor(Processor *sink) { theBuffer.openTrapdoor(sink); }

	/**
	 * Relays an closeTrapdoor call to the Buffer object.
	 *
	 * This is used by the MLConnection class as it is more robust than relying
	 * on the BufferReader object to be existant (which, in the case of being
	 * connected to a DomProcessor object it would not be).
	 *
	 * @param sink The Processor object that the trapdoor must be opened for.
	 */
	void closeBufferTrapdoor(Processor *sink) { theBuffer.closeTrapdoor(sink); }

	/**
	 * Simple constructor, used from Processor object.
	 *
	 * @param source The connection's source.
	 * @param sourceIndex The port index of the source.
	 * @param bufferSize The minimum size of buffer to be used for the
	 * new connection.
	 */
	LMConnection(Source *source, uint sourceIndex, uint bufferSize);

	/**
	 * Simple destructor.
	 */
	~LMConnection();

protected:
	QList<MLConnection*> theConnections;
	Buffer theBuffer;
};

};

#endif
