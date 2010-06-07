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
	virtual const SignalTypeRef type() const { return SignalTypeRef(const_cast<LxConnectionNull*>(this)->theType); }
	virtual void pushBE(const BufferData &);
	virtual void pushPlunger() {}
	virtual void startPlungers() {}
	virtual void plungerSent() {}
	virtual void noMorePlungers() {}
	virtual uint maximumScratchElements(const uint) { return Undefined; }
	virtual uint maximumScratchElementsEver() { return Undefined; }
	virtual BufferData makeScratchElements(const uint, bool = false) { return BufferData::fake(); }
	virtual void setType(const TransmissionType *type) { theType = type->copy(); }
	virtual void resetType() { delete theType; theType = 0L; }
	virtual bool waitUntilReady() { return 1; }
	virtual Tristate isReadyYet() { return Succeeded; }
	virtual void sourceStopping() {}
	virtual void sourceStopped() {}
	virtual void reset() {}
	virtual void enforceMinimum(const uint) {}
	virtual uint bufferElementsFree() { return (uint)-1; }

public:
	LxConnectionNull(Source *source, uint sourceIndex) : LxConnection(source, sourceIndex) {}
};

}
