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

#include "domprocessor.h"
#include "subprocessor.h"
#include "subprocessorfactory.h"
using namespace Geddei;

#include "localsession.h"
#include "remotesession.h"
#include "localprocessor.h"
#include "remoteprocessor.h"
#include "localdomprocessor.h"
using namespace rGeddei;

#define MESSAGES 0

namespace rGeddei
{

LocalDomProcessor::LocalDomProcessor(LocalSession &session, const QString &type) :
	LocalProcessor(session, SubProcessorFactory::createDom(type))
{
}

LocalDomProcessor::LocalDomProcessor(LocalSession &session, SubProcessor *primary) :
	LocalProcessor(session, new DomProcessor(primary))
{
}

LocalDomProcessor::~LocalDomProcessor()
{
	theAdopted = true;
}

const bool LocalDomProcessor::spawnWorker(LocalSession &session)
{
	if(MESSAGES) qDebug("LocalDomProcessor::spawnWorker(Local?)");
	if(theSession != &session)
		return dynamic_cast<DomProcessor *>(theProcessor)->createAndAddWorker(session.theHost, session.thePort);
	else
		return dynamic_cast<DomProcessor *>(theProcessor)->createAndAddWorker();
}

const bool LocalDomProcessor::spawnWorker(RemoteSession &session)
{
	if(MESSAGES) qDebug("LocalDomProcessor::spawnWorker(Node)");
	return dynamic_cast<DomProcessor *>(theProcessor)->createAndAddWorker(session.theHost, session.theKey);
}

};
