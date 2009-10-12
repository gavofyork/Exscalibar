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

#include "bufferinfo.h"
#include "buffer.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

void BufferInfo::jettison()
{
	// TODO: actually do what should be done at the end of its life.
	// Do this by making a new method in BufferInfo - kill
	assert(theLife == Foreign);
	assert(theResident = true);
	theEndType = Ignore;
	theValid = false;
	theResident = false;
}

void BufferInfo::reference()
{
	theCounter++;
}

void BufferInfo::unreference(BufferData &client)
{
	if (--theCounter == 0)
		destruct(client);
}

void BufferInfo::retire(BufferData &client)
{
	if (theAux && theValid)
	{	if (theEndType == Forget)
		{	if (MESSAGES) qDebug("BufferInfo[%p]: Autoforgetting with %p.", this, theAux);
			theAux->forget(client);
		}
		else if (theEndType == Activate)
			theAux->activate(client);
	}
}

void BufferInfo::destruct(BufferData &client)
{
	retire(client);

	if (theLife == Managed)
		delete [] theData;

	if (!theResident)
		delete this;
}

}

#undef MESSAGES
