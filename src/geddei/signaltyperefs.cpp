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

#include <cassert>
using namespace std;

#include <q3ptrlist.h>

#include "signaltyperefs.h"
#include "signaltyperef.h"
#include "signaltype.h"

namespace Geddei
{

SignalTypeRefs::SignalTypeRefs(const SignalTypeRefs &src): theImplicitCopying(true)
{

	theCount = src.theCount;
	theData = new const SignalType *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = src.theData[i]->copy();
}

SignalTypeRefs::SignalTypeRefs(Q3PtrList<SignalType> &src): theImplicitCopying(true)
{
	theCount = src.count();
	theData = new const SignalType *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = src.at(i)->copy();
}

SignalTypeRefs::SignalTypeRefs(uint count, bool implicitCopying) : theCount(count), theImplicitCopying(implicitCopying)
{
	theData = new const SignalType *[count];
	for (uint i = 0; i < theCount; i++)
		theData[i] = 0;
}

SignalTypeRefs::~SignalTypeRefs()
{
	for (uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
}

SignalTypeRefs &SignalTypeRefs::operator=(const SignalTypeRefs &src)
{
	for (uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
	theCount = src.theCount;
	theData = new const SignalType *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = theImplicitCopying ? src.theData[i]->copy() : src.theData[i];
	return *this;
}

SignalTypeRefs &SignalTypeRefs::operator=(const SignalType &src)
{
	for (uint i = 0; i < theCount; i++)
		if (theData[i] != &src)
		{	delete theData[i];
			theData[i] = src.copy();
		}
	return *this;
}

SignalTypeRefs &SignalTypeRefs::operator=(const SignalTypeRef &src)
{
	for (uint i = 0; i < theCount; i++)
		if (theData[i] != src.thePtr)
		{	delete theData[i];
			theData[i] = src.thePtr->copy();
		}
	return *this;
}

bool SignalTypeRefs::allSame() const
{
	if (!theData[0]) return false;
	for (uint i = 1; i < theCount; i++)
		if (!theData[i]) return false;
		else if (!theData[i]->sameAs(theData[0])) return false;
	return true;
}

void SignalTypeRefs::resize(uint count)
{
	for (uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
	theCount = count;
	theData = new const SignalType *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = 0;
}

void SignalTypeRefs::setData(uint i, const SignalType *d)
{
	delete theData[i];
	theData[i] = d;
}

void SignalTypeRefs::setFill(const SignalType *d, bool replaceExisting)
{
	bool okToDel = true;
	for (uint i = 0; i < theCount; i++)
	{	// Annoying exception - we need to explicitly allow for the scenario that
		// d is a member of the old array. Thus we cant delete the old array member
		// if, in fact, it is d.
		if (replaceExisting)
		{	if (theData[i] != d) delete theData[i];
			theData[i] = i ? d->copy() : d;
		}
		else
			if (!theData[i])
				theData[i] = d->copy();
			else if (theData[i] == d)
				okToDel = false;
	}
	if (!theCount || okToDel) delete d;
}

void SignalTypeRefs::copyData(uint i, const SignalType *d)
{
	delete theData[i];
	theData[i] = d->copy();
}

void SignalTypeRefs::copyInto(Q3PtrList<SignalType> &dest)
{
	dest.clear();
	for (uint i = 0; i < theCount; i++)
		dest.append(theData[i]->copy());
}

}
