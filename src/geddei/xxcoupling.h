/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_XXCOUPLING_H
#define _GEDDEI_XXCOUPLING_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "bufferdatas.h"
#else
#include <geddei/bufferdatas.h>
#endif

namespace Geddei
{

class SignalTypeRefs;
class Properties;

/** @internal @ingroup Geddei
 * @brief Abstract class to define the Sub/DomProcessor coupling mechanism.
 * @author Gav Wood <gav@kde.org>
 */
class xxCoupling
{
public:
	/**
	 * Virtual destructor.
	 */
	virtual ~xxCoupling() {}
};

};

#endif
