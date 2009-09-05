/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_MULTIPLICATIVE_H
#define _GEDDEI_MULTIPLICATIVE_H

#include <qstring.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "properties.h"
#else
#include <geddei/properties.h>
#endif
using namespace Geddei;

namespace Geddei
{

class ProcessorGroup;

/** @internal @ingroup Geddei
 * @brief Abstract class for defining interface to a fundamentally multiplicitive object.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 */
class Multiplicative
{
protected:
	//* Deferred init data
	bool theDeferredInit;
	Properties theDeferredProperties;
	QString theDeferredName;
	ProcessorGroup *theDeferredGroup;

	virtual void setSourceMultiplicity(uint multiplicity) = 0;
	virtual void setSinkMultiplicity(uint multiplicity) = 0;

public:
	virtual bool initGiven() const = 0;
	virtual bool knowMultiplicity() const = 0;
	virtual uint multiplicity() const = 0;
	virtual void doInit(const QString &name, ProcessorGroup *g = 0, const Properties &properties = Properties()) = 0;

	void setMultiplicity(uint multiplicity);

	Multiplicative() : theDeferredInit(false) {}
	virtual ~Multiplicative() {}
};

}

#endif
