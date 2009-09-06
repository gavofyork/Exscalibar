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

#include "multisource.h"
#include "multisink.h"
//Added by qt3to4:
#include <Q3ValueList>

namespace Geddei
{

void MultiSink::setSinkMultiplicity(uint multiplicity)
{
	Q3ValueList<MultiSource *> t = theDeferrals;
	for (Q3ValueList<MultiSource *>::Iterator i = t.begin(); i != t.end(); i++)
		(*i)->setMultiplicity(multiplicity);
}

}
