/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_SPLITTER_H
#define _GEDDEI_SPLITTER_H

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
class Splitter: public LxConnection, public Source
{
	//* Reimplementations from Source
	virtual void checkExit();
	virtual bool confirmTypes();
	virtual void doRegisterOut(LxConnection *me, uint port);
	virtual void undoRegisterOut(LxConnection *me, uint port);
	virtual const Processor *processor() const { return dynamic_cast<Processor *>(theSource); }

	//* Reimplementations from Connection
	virtual const SignalTypeRef type();

	//* Reimplementations from ScratchOwner, LxConnection
	virtual void pushScratch(const BufferData &data);
	virtual void forgetScratch(const BufferData &data);

	//* Reimplementations from LxConnection
	virtual bool waitUntilReady();
	virtual Tristate isReadyYet();
	virtual void setType(const SignalType *type);
	virtual void resetType();
	virtual void sourceStopping();
	virtual void sourceStopped();
	virtual void reset();
	virtual BufferData makeScratchElements(uint elements, bool autoPush = false);
	virtual void push(const BufferData &data);
	virtual void pushPlunger();
	virtual void startPlungers();
	virtual void plungerSent();
	virtual void noMorePlungers();
	virtual uint maximumScratchElements(uint minimum = 1);
	virtual uint maximumScratchElementsEver();
	virtual void enforceMinimum(uint elements);
	virtual uint bufferElementsFree();

	QList<LxConnection*> theConnections;

public:
	Splitter(Processor *source, uint sourceIndex);
	virtual ~Splitter();
};

};

#endif
