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

#include "source.h"
#include "bufferdata.h"
#include "lxconnectionreal.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

void LxConnectionReal::push(const BufferData &data)
{
	if (MESSAGES) qDebug("LxConnectionReal::push(): (size=%d)", data.elements());

	if (theLastElementsFree > data.elements())
	{	theSource->checkExit();
		transport(data);
		theLastElementsFree -= data.elements();
	}
	else
		for (uint i = 0; i < data.elements();)
		{
			theSource->checkExit();
			bufferWaitForFree();
			int elementsFree = bufferElementsFree();
			int samplesToSend = theType->samples(elementsFree);
			// TODO: will grind. fix: waitForFreeSample()
			if (!samplesToSend) continue;
			uint elementsToSend = theType->elementsFromSamples(samplesToSend);
			if (elementsToSend > data.elements() - i) elementsToSend = data.elements() - i;

			transport(data.mid(i, elementsToSend));
			i += elementsToSend;
			theLastElementsFree = elementsFree - elementsToSend;
		}
	if (MESSAGES) qDebug("LxConnectionReal::push(): Pushed (size=%d).", data.elements());
}

}

#undef MESSAGES
