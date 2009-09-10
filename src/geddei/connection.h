/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_CONNECTION_H
#define _GEDDEI_CONNECTION_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "signaltype.h"
#include "signaltyperef.h"
#else
#include <geddei/signaltype.h>
#include <geddei/signaltyperef.h>
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
class Connection
{
protected:
	SignalType *theType;

public:
	/**
	 * Retrieves the type of signal for the connection.
	 * 
	 * Should ask source processor if the type is unknown.
	 */
	virtual const SignalTypeRef type() = 0;

	/**
	 * Makes the writing/reading buffer at least @a elements big.
	 * 
	 * Source-writers/sink-readers may make access transactions of elements
	 * without blocking indefinately.
	 * 
	 * Makes sure that maximumScratchElementsEver() will return at least
	 * elements.
	 * 
	 * Makes sure that readElements(@a elements) will never block indefinately.
	 */
	virtual void enforceMinimum(uint elements) = 0;

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

#endif
