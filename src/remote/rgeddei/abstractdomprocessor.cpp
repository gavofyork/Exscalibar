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

#include "subprocessor.h"
#include "abstractdomprocessor.h"
#include "remotedomprocessor.h"
#include "localdomprocessor.h"
#include "localsession.h"
#include "remotesession.h"
using namespace Geddei;
using namespace rGeddei;

namespace rGeddei
{

LocalDomProcessor *AbstractDomProcessor::create(LocalSession &session, SubProcessor *primary)
{
	return new LocalDomProcessor(session, primary);
}

LocalDomProcessor *AbstractDomProcessor::create(LocalSession &session, const QString &type)
{
	return new LocalDomProcessor(session, type);
}

RemoteDomProcessor *AbstractDomProcessor::create(RemoteSession &session, const QString &type)
{
	return new RemoteDomProcessor(session, type);
}

}
