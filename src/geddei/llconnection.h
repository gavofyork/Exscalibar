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
 * @author Gav Wood <gav@kde.org>
 *
 * A realisation of the Connection flow-control class framework.
 * This shunts data from a processor object, to another processor object, and provides
 * an abstraction for automatic type synchronisation.
 */
class DLLEXPORT LLConnection: public LxConnectionReal, public xLConnectionReal
{
	//* Reimplementations from LxConnection
	virtual bool waitUntilReady();
	virtual Tristate isReadyYet();
	virtual const Type& type() const { return xLConnectionReal::type(); }
	virtual void setType(Type const& _type);
	virtual void resetType();
	virtual void sourceStopping();
	virtual void sourceStopped();
	virtual void reset() {}
	virtual BufferData makeScratchElements(uint elements, bool autoPush);
	virtual void pushPlunger();
	virtual void startPlungers();
	virtual void plungerSent();
	virtual void noMorePlungers();
	virtual uint freeInDestinationBuffer(uint minimum) { while (bufferElementsFree() < minimum) bufferWaitForFree(); return bufferElementsFree(); }
	virtual uint freeInDestinationBufferEver() { return theBuffer.size(); }

	//* Reimplementation from xLConnection.
	virtual bool pullType();

	//* Reimplementation from LxConnectionReal.
	virtual void bufferWaitForFree();
	virtual uint bufferElementsFree();
	virtual void transport(const BufferData &data);

	friend class Processor;
	LLConnection(Source *newSource, uint newSourceIndex, Sink *newSink, uint newSinkIndex, uint bufferSize);
};

}
