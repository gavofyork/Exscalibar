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

#include "xxcoupling.h"
#include "bufferdatas.h"
#else
#include <geddei/xxcoupling.h>
#include <geddei/bufferdatas.h>
#endif

namespace Geddei
{

class SubProcessor;
class Properties;
class SignalTypeRefs;

/** @internal @ingroup Geddei
 * @brief Refinement of a Coupling for RHS attachment to a SubProcessor object.
 * @author Gav Wood <gav@kde.org>
 */
class xSCoupling: virtual public xxCoupling
{
protected:
	/**
	 * Basic constructor.
	 *
	 * @param subProc TheSubProcessor to which this Coupling object shall be
	 * tied.
	 */
	xSCoupling(SubProcessor *subProc);

	/**
	 * Default destructor.
	 */
	virtual ~xSCoupling();

	SubProcessor *theSubProc;
};

}
