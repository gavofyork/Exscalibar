/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_XSCOUPLING_H
#define _GEDDEI_XSCOUPLING_H

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
class RSCoupling;	// hack around, because mixins aren't used for the remote versions.

/** @internal @ingroup Geddei
 * @brief Refinement of a Coupling for RHS attachment to a SubProcessor object.
 * @author Gav Wood <gav@kde.org>
 */
class xSCoupling : virtual public xxCoupling
{
	SubProcessor *theSubProc;

	// Reimplementations from xxCoupling
	friend class RSCoupling;
	virtual void go();
	virtual void stop();
	virtual void stoppingR();
	virtual void stoppedR();
	virtual void specifyTypes(const SignalTypeRefs &inTypes, const SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &p);
	virtual void transact(const BufferDatas &d, uint chunks);
	virtual BufferDatas deliverResults(uint *timeTaken);
	virtual void defineIO(uint inputs, uint outputs);

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
};

}

#endif
