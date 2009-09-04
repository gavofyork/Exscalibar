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

#include "multiprocessorcreator.h"
#include "processor.h"
#include "multiprocessor.h"

#define MESSAGES 0

namespace Geddei
{

MultiProcessor::~MultiProcessor()
{
	// delete all processors
	for(uint i = 0; i < theProcessors.count(); i++)
		delete theProcessors[i];
	delete theCreator;
}

void MultiProcessor::doInit(const QString &name, ProcessorGroup *g, const Properties &properties)
{
	if(MESSAGES) qDebug("MultiProcessor::init()");
	assert(!theIsInitialised);

	if(!properties.keys().contains("Multiplicity"))
	{	if(MESSAGES) qDebug("Deferring...");
		theDeferredInit = true;
		theDeferredProperties = properties;
		// FIXME: need to name and group it now somehow, in order to use the group to connect it later.
		theDeferredName = name;
		theDeferredGroup = g;
		return;
	}

	if(MESSAGES) qDebug("Initialising (M=%d)...", properties["Multiplicity"].toInt());
	theProcessors.resize(properties["Multiplicity"].toInt());
	//qDebug("Multiplicity %d.", theProcessors.count());
	for(uint i = 0; i < theProcessors.count(); i++)
	{	theProcessors[i] = theCreator->newProcessor();
		//qDebug("Processor %d created as %p", i, theProcessors[i]);
		theProcessors[i]->doInit(name + QString::number(i), g, properties);
	}
	theIsInitialised = true;
	theDeferredInit = false;
}

};
