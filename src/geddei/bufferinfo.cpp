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
