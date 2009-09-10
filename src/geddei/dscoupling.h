/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_DSCOUPLING_H
#define _GEDDEI_DSCOUPLING_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "dxcoupling.h"
#include "xscoupling.h"
#else
#include <geddei/dxcoupling.h>
#include <geddei/xscoupling.h>
#endif

namespace Geddei
{

class DomProcessor;

/** @internal @ingroup Geddei
 * @brief Embodiment of a direct coupling between DomProcessor and SubProcessor.
 * @author Gav Wood <gav@kde.org>
 */
class DSCoupling: public DxCoupling, public xSCoupling
{
	friend class DomProcessor;
	DSCoupling(DomProcessor *dom, SubProcessor *subProc);
};


};

#endif
