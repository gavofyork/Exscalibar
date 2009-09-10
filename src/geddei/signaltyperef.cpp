/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <cassert>
using namespace std;

#include "signaltype.h"
#include "signaltyperef.h"
using namespace Geddei;

namespace Geddei
{

bool SignalTypeRef::operator==(const SignalType &p)
{
	return thePtr->sameAs(&p);
}

bool SignalTypeRef::operator==(const SignalTypeRef &p)
{
	return thePtr->sameAs(p.thePtr);
}

SignalTypeRef &SignalTypeRef::operator=(const SignalType &p)
{
	delete thePtr;
	thePtr = p.copy();
	return *this;
}

SignalTypeRef &SignalTypeRef::operator=(const SignalTypeRef &p)
{
	delete thePtr;
	thePtr = p.thePtr->copy();
	return *this;
}

uint SignalTypeRef::scope() const
{
	return thePtr->scope();
}

float SignalTypeRef::frequency() const
{
	return thePtr->frequency();
}

}
