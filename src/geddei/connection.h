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

#include "transmissiontype.h"
#include "type.h"
#else
#include <geddei/transmissiontype.h>
#include <geddei/type.h>
#endif
using namespace Geddei;

namespace Geddei
{

/** @internal @ingroup Geddei
 * @brief Abstract class defining link between two Processor objects.
 * @author Gav Wood <gav@kde.org>
 *
 * An abstract flow control class to put between two Processors on potentially
 * different machines. Handles data transfer between them and some basic control
 * protocol for communicating data types.
 */
class DLLEXPORT Connection
{
public:
	/**
	 * The various error types that a Processor may trip up on.
	 */
	enum Tristate
	{	Succeeded = 0, ///< Indicates no error occured.
		Pending, ///< Indicates the operation has yet to finish.
		Failed
	};

protected:
	/**
	 * Retrieves the type of signal for the connection.
	 *
	 * Should ask source processor if the type is unknown.
	 */
	virtual Type const& type() const = 0;

	virtual void enforceMinimumRead(uint) {}
	virtual void enforceMinimumWrite(uint) {}

	mutable Type theType;

public:
	/**
	 * Makes the writing/reading buffer at least @a elements big.
	 *
	 * Source-writers/sink-readers may make access transactions of elements
	 * without blocking indefinately.
	 *
	 * Makes sure that freeInDestinationBufferEver() will return at least
	 * elements.
	 *
	 * Makes sure that readElements(@a elements) will never block indefinately.
	 */
	virtual void setMinimumRead(uint _s) { enforceMinimumRead(_s * theType.size()); }
	virtual void setMinimumWrite(uint _s) { enforceMinimumWrite(_s * theType.size()); }

	/**
	 * Simple constructor.
	 */
	Connection();

	/**
	 * Simple virtual destructor.
	 */
	virtual ~Connection();
};

}
