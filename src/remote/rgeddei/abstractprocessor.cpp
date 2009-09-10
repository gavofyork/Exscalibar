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

#include "subprocessor.h"
using namespace Geddei;

#include "localprocessor.h"
#include "remoteprocessor.h"
#include "localdomprocessor.h"
#include "remotedomprocessor.h"
#include "abstractprocessorgroup.h"
#include "abstractprocessor.h"
using namespace rGeddei;

namespace rGeddei
{

LocalProcessor *AbstractProcessor::create(LocalSession &session, Processor &processor)
{
	return new LocalProcessor(session, processor);
}

LocalProcessor *AbstractProcessor::create(LocalSession &session, Processor *processor)
{
	return new LocalProcessor(session, processor);
}

LocalProcessor *AbstractProcessor::create(LocalSession &session, const QString &type)
{
	return new LocalProcessor(session, type);
}

RemoteProcessor *AbstractProcessor::create(RemoteSession &session, const QString &type)
{
	return new RemoteProcessor(session, type);
}

void AbstractProcessor::setGroup(AbstractProcessorGroup &g)
{
	if (theGroup == &g) return;
	if (theGroup) theGroup->remove(this);
	theGroup = &g;
	if (theGroup) theGroup->add(this);
}

void AbstractProcessor::setNoGroup()
{
	if (!theGroup) return;
	AbstractProcessorGroup *d = theGroup;
	theGroup = 0;
	if (d) d->remove(this);
}

};
