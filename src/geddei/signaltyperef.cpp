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

#include "signaltype.h"
#include "signaltyperef.h"
using namespace Geddei;

namespace Geddei
{

bool SignalTypeRef::operator==(const SignalType &p)
{
	assert(thePtr);
	return thePtr->sameAs(&p);
}

bool SignalTypeRef::operator==(const SignalTypeRef &p)
{
	assert(thePtr);
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
	assert(thePtr);
	return thePtr->scope();
}

float SignalTypeRef::frequency() const
{
	assert(thePtr);
	return thePtr->frequency();
}

}
