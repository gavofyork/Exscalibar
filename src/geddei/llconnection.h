/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_LLCONNECTION_H
#define _GEDDEI_LLCONNECTION_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "xlconnectionreal.h"
#include "lxconnectionreal.h"
#else
#include <geddei/xlconnectionreal.h>
#include <geddei/lxconnectionreal.h>
#endif
using namespace Geddei;

namespace Geddei
{

class Processor;
class Source;

/** @internal @ingroup Geddei
 * @brief Embodiment of Connection between a local Source object and a local Sink object.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * A realisation of the Connection flow-control class framework.
 * This shunts data from a processor object, to another processor object, and provides
 * an abstraction for automatic type synchronisation.
 */
class DLLEXPORT LLConnection: public LxConnectionReal, public xLConnectionReal
{
	//* Reimplementations from LxConnection
	virtual bool waitUntilReady();
	virtual const SignalTypeRef type() { return xLConnectionReal::type(); }
	virtual void setType(const SignalType *type);
	virtual void resetType();
	virtual void sourceStopping();
	virtual void sourceStopped();
	virtual void reset() {}
	virtual BufferData makeScratchElements(uint elements, bool autoPush);
	virtual void pushPlunger();
	virtual void startPlungers();
	virtual void plungerSent();
	virtual void noMorePlungers();
	virtual uint maximumScratchElements(uint minimum) { while(bufferElementsFree() < minimum) bufferWaitForFree(); return bufferElementsFree(); }
	virtual uint maximumScratchElementsEver() { return theBuffer.size(); }

	//* Reimplementation from xLConnection.
	virtual bool pullType();

	//* Reimplementation from LxConnectionReal.
	virtual void bufferWaitForFree();
	virtual uint bufferElementsFree();
	virtual void transport(const BufferData &data);

	friend class Processor;
	LLConnection(Source *newSource, uint newSourceIndex, Sink *newSink, uint newSinkIndex, uint bufferSize);
};

};

#endif
