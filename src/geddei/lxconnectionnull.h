/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_LXCONNECTIONNULL_H
#define _GEDDEI_LXCONNECTIONNULL_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "signaltyperef.h"
#include "lxconnection.h"
#else
#include <geddei/signaltyperef.h>
#include <geddei/lxconnection.h>
#endif
using namespace Geddei;

namespace Geddei
{

/** @internal @ingroup Geddei
 * @brief Embodiment of a Connection from a local Source that discards any data.
 * @author Gav Wood <gav@kde.org>
 *
 * A refinement of a flow control class for outbound communication.
 * This class is a nullified class that only pretends to "work". No data is transferred
 * at all with this class.
 */
class DLLEXPORT LxConnectionNull: public LxConnection
{
	virtual const SignalTypeRef type() { return SignalTypeRef(theType); }
	virtual void push(const BufferData &) {}
	virtual void pushPlunger() {}
	virtual void startPlungers() {}
	virtual void plungerSent() {}
	virtual void noMorePlungers() {}
	virtual uint maximumScratchElements(const uint) { return Undefined; }
	virtual uint maximumScratchElementsEver() { return Undefined; }
	virtual BufferData makeScratchElements(const uint, bool = false) { return BufferData::fake(); }
	virtual void setType(const SignalType *type) { theType = type->copy(); }
	virtual void resetType() { delete theType; theType = 0L; }
	virtual bool waitUntilReady() { return true; }
	virtual void sourceStopping() {}
	virtual void sourceStopped() {}
	virtual void reset() {}
	virtual void enforceMinimum(const uint) {}

public:
	LxConnectionNull(Source *source, uint sourceIndex) : LxConnection(source, sourceIndex) {}
};

};

#endif
