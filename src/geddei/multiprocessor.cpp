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

#include "multiprocessorcreator.h"
#include "processor.h"
#include "multiprocessor.h"

#define MESSAGES 0

namespace Geddei
{

MultiProcessor::~MultiProcessor()
{
	// delete all processors
	for (uint i = 0; i < (uint)theProcessors.count(); i++)
		delete theProcessors[i];
	delete theCreator;
}

void MultiProcessor::doInit(const QString &name, ProcessorGroup *g, const Properties &properties)
{
	if (MESSAGES) qDebug("MultiProcessor::init()");
	assert(!theIsInitialised);

	if (!properties.keys().contains("Multiplicity"))
	{	if (MESSAGES) qDebug("Deferring...");
		theDeferredInit = true;
		theDeferredProperties = properties;
		// FIXME: need to name and group it now somehow, in order to use the group to connect it later.
		theDeferredName = name;
		theDeferredGroup = g;
		return;
	}

	if (MESSAGES) qDebug("Initialising (M=%d)...", properties["Multiplicity"].toInt());
	theProcessors.resize(properties["Multiplicity"].toInt());
	//qDebug("Multiplicity %d.", theProcessors.count());
	for (uint i = 0; i < (uint)theProcessors.count(); i++)
	{	theProcessors[i] = theCreator->newProcessor();
		//qDebug("Processor %d created as %p", i, theProcessors[i]);
		theProcessors[i]->doInit(name + QString::number(i), g, properties);
	}
	theIsInitialised = true;
	theDeferredInit = false;
}

}

#undef MESSAGES
