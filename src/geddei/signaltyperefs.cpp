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

#include <cassert>
using namespace std;

#include "signaltyperefs.h"
#include "signaltyperef.h"
#include "signaltype.h"

namespace Geddei
{

SignalTypeRefs::SignalTypeRefs(const SignalTypeRefs &src): theImplicitCopying(true)
{

	theCount = src.theCount;
	theData = new const TransmissionType *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = src.theData[i]->copy();
}

SignalTypeRefs::SignalTypeRefs(QList<TransmissionType*> &src): theImplicitCopying(true)
{
	theCount = src.count();
	theData = new const TransmissionType *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = src.at(i)->copy();
}

SignalTypeRefs::SignalTypeRefs(uint count, bool implicitCopying) : theCount(count), theImplicitCopying(implicitCopying)
{
	theData = new const TransmissionType *[count];
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
	theData = new const TransmissionType *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = theImplicitCopying ? src.theData[i]->copy() : src.theData[i];
	return *this;
}

SignalTypeRefs &SignalTypeRefs::operator=(const TransmissionType &src)
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
		else if (!theData[i]->isEqualTo(theData[0])) return false;
	return true;
}

void SignalTypeRefs::resize(uint count)
{
	for (uint i = 0; i < theCount; i++)
		delete theData[i];
	delete [] theData;
	theCount = count;
	theData = new const TransmissionType *[theCount];
	for (uint i = 0; i < theCount; i++)
		theData[i] = 0;
}

void SignalTypeRefs::setData(uint i, const TransmissionType *d)
{
	delete theData[i];
	theData[i] = d;
}

void SignalTypeRefs::setFill(const TransmissionType *d, bool replaceExisting)
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

void SignalTypeRefs::copyData(uint i, const TransmissionType *d)
{
	delete theData[i];
	theData[i] = d->copy();
}

void SignalTypeRefs::copyInto(QList<TransmissionType*> &dest)
{
	dest.clear();
	for (uint i = 0; i < theCount; i++)
		dest.append(theData[i]->copy());
}

}
