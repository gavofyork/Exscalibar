/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

#include "processorgroup.h"
#include "groupable.h"
using namespace Geddei;

namespace Geddei
{

Groupable::Groupable(): theGroup(0)
{
}

Groupable::~Groupable()
{
	// TODO: THIS MUST BE MUTUALLY EXCLUSIVE TO ANYTHING ACCESSING IT REMOTELY
	// (I.E. THROUGH GROUP, FOR THE ENTIRE REMOTE ACTION) --- LOCK IN GROUP?
	setNoGroup();
	// FINISH MUTUAL EXCLUSIVITY
}

void Groupable::setGroup(ProcessorGroup &g)
{
	if (theGroup == &g) return;
	if (theGroup) theGroup->remove(this);
	theGroup = &g;
	if (theGroup) theGroup->add(this);
}

void Groupable::setNoGroup()
{
	if (!theGroup) return;
	ProcessorGroup *d = theGroup;
	theGroup = 0L;
	if (d) d->remove(this);
}

}
