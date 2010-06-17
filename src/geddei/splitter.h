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
#include "lxconnection.h"
#include "lxconnectionreal.h"
#else
#include <geddei/lxconnection.h>
#include <geddei/lxconnectionreal.h>
#endif
using namespace Geddei;

namespace Geddei
{

class Processor;

/** @internal @ingroup Geddei
 * @brief Copies data from a Connection to multiple other Connections.
 * @author Gav Wood <gav@kde.org>
 *
 */
class DLLEXPORT Splitter: public LxConnection, public Source
{
	//* Reimplementations from Source
	virtual void checkExit();
	virtual bool confirmTypes();
	virtual void doRegisterOut(LxConnection *me, uint port);
	virtual void undoRegisterOut(LxConnection *me, uint port);
	virtual const Processor *processor() const { return dynamic_cast<Processor *>(theSource); }

	//* Reimplementations from Connection
	virtual Type const& type() const;

	//* Reimplementations from ScratchOwner, LxConnection
	virtual void pushScratch(const BufferData &data);
	virtual void forgetScratch(const BufferData &data);

	//* Reimplementations from LxConnection
	virtual bool waitUntilReady();
	virtual Tristate isReadyYet();
	virtual void setType(Type const& _type);
	virtual void resetType();
	virtual void sourceStopping();
	virtual void sourceStopped();
	virtual void reset();
	virtual BufferData makeScratchElements(uint elements, bool autoPush = false);
	virtual void pushBE(const BufferData &data);
	virtual void pushPlunger();
	virtual void startPlungers();
	virtual void plungerSent();
	virtual void noMorePlungers();
	virtual uint maximumScratchElements(uint minimum = 1);
	virtual uint maximumScratchElementsEver();
	virtual void enforceMinimumWrite(uint _elements);
	virtual uint bufferElementsFree();
	virtual double secondsPassed() const { return theSource->secondsPassed(); }
	virtual double secondsPassed(float _s, uint _i = 0) const { return theSource->secondsPassed(_s, _i); }

	QList<LxConnection*> theConnections;

public:
	Splitter(Processor *source, uint sourceIndex);
	virtual ~Splitter();
};

}
