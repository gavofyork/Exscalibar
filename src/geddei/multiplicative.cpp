/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include "multiplicative.h"

#undef MESSAGES
#define MESSAGES 0

namespace Geddei
{

void Multiplicative::setMultiplicity(uint multiplicity)
{
	if (MESSAGES) qDebug("Multiplicative::setMultiplicity(%d)", multiplicity);
	if (theDeferredInit)
	{	if (MESSAGES) qDebug("Deferred init - reinitialising...");
		theDeferredProperties["Multiplicity"] = multiplicity;
		doInit(theDeferredName, theDeferredGroup, theDeferredProperties);
	}
	setSourceMultiplicity(multiplicity);
	setSinkMultiplicity(multiplicity);
}

};
