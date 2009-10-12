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

}
