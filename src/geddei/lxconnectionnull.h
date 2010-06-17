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
#include "type.h"
#include "lxconnection.h"
#else
#include <geddei/type.h>
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
	virtual Type const& type() const { return theType; }
	virtual void pushBE(const BufferData &);
	virtual void pushPlunger() {}
	virtual void startPlungers() {}
	virtual void plungerSent() {}
	virtual void noMorePlungers() {}
	virtual uint maximumScratchElements(uint) { return Undefined; }
	virtual uint maximumScratchElementsEver() { return Undefined; }
	virtual BufferData makeScratchElements(uint, bool = false) { return BufferData::fake(); }
	virtual void setType(Type const& _type) { theType = _type; }
	virtual void resetType() { theType.nullify(); }
	virtual bool waitUntilReady() { return 1; }
	virtual Tristate isReadyYet() { return Succeeded; }
	virtual void sourceStopping() {}
	virtual void sourceStopped() {}
	virtual void reset() {}
	virtual void enforceMinimum(uint) {}
	virtual void enforceMinimumRead(uint) {}
	virtual void enforceMinimumWrite(uint) {}
	virtual uint bufferElementsFree() { return (uint)-1; }

public:
	LxConnectionNull(Source *source, uint sourceIndex) : LxConnection(source, sourceIndex) {}
};

}
