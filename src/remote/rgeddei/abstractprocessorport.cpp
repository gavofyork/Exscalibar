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

#include "remoteprocessor.h"
#include "localprocessor.h"
#include "abstractprocessor.h"
using namespace rGeddei;

namespace rGeddei
{

bool AbstractProcessorPort::split()
{
	theParent->split(thePort);
	return true;
}

bool AbstractProcessorPort::share()
{
	theParent->share(thePort);
	return true;
}

bool AbstractProcessorPort::connect(const AbstractProcessorPort &input)
{
	if (dynamic_cast<RemoteProcessor *>(input.theParent))
		return theParent->connect(thePort, dynamic_cast<RemoteProcessor *>(input.theParent), input.thePort, input.theBufferSize);
	else if (dynamic_cast<LocalProcessor *>(input.theParent))
		return theParent->connect(thePort, dynamic_cast<LocalProcessor *>(input.theParent), input.thePort, input.theBufferSize);
	else
		return false;
}

bool AbstractProcessorPort::disconnect()
{
	theParent->disconnect(thePort);
	return true;
}

}
