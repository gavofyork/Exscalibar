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

#include "source.h"
#include "bufferdata.h"
#include "lxconnectionreal.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

void LxConnectionReal::pushBE(const BufferData &data)
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
