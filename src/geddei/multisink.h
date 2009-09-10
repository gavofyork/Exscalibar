/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_MULTISINK_H
#define _GEDDEI_MULTISINK_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "multiplicative.h"
#include "processorport.h"
#else
#include <geddei/multiplicative.h>
#include <geddei/processorport.h>
#endif
using namespace Geddei;

namespace Geddei
{

class MultiSource;

/** @internal @ingroup Geddei
 * @brief Abstract class for defining interface to multiplicitive sink object.
 * @author Gav Wood <gav@kde.org>
 *
 */
class DLLEXPORT MultiSink: virtual public Multiplicative
{
	//* (Reverse) connection deferral mechanism
	QList<MultiSource *> theDeferrals;

	//* Reimplementations from Multiplicative
	virtual void setSinkMultiplicity(uint multiplicity);

	//* Neccessary so MultiSource can use append-/remove-Deferral
	friend class MultiSource;

	/**
	 * Appends the given MultiSource to our list of deferrals.
	 * We can then go through our list of deferrals when we get our multiplicity set,
	 * setting their multiplicities.
	 */
	void appendDeferral(MultiSource *o) { theDeferrals.append(o); }

	/**
	 * Removes the given MultiSource to our list of deferrals.
	 * This is done by the MultiSource in question once a deferred connection has been
	 * made.
	 */
	void removeDeferral(MultiSource *o) { theDeferrals.removeOne(o); }

public:
	/**
	 * Override to provide the routine for providing a sink port.
	 */
	virtual ProcessorPort sinkPort(uint i) = 0;
};

};

#endif
