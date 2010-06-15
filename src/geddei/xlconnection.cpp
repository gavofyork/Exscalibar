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

#include "processor.h"
#include "xlconnection.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

xLConnection::xLConnection(Sink *newSink, uint newSinkIndex) : Connection()
{
	theSink = newSink;
	theSinkIndex = newSinkIndex;
	theSink->doRegisterIn(this, theSinkIndex);
	theSink->resetTypes();
}

xLConnection::~xLConnection()
{
	// If theSink has already been set to zero, it means that we have already reset the
	// processor's port, and that there's no need to again.
	if (theSink)
	{
//		if (MESSAGES) qDebug("Deleting input link on %s...", theSink->name().toLatin1());
		theSink->undoRegisterIn(this, theSinkIndex);
	}
}

}

#undef MESSAGES
