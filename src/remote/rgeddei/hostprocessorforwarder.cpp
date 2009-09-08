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

#include <qstring.h>

#include "processor.h"
using namespace Geddei;

#include "hostprocessorforwarder.h"
#include "sessionserver.h"
using namespace rGeddei;

namespace rGeddei
{

Processor *HostProcessorForwarder::lookup(uint key, const QString &name)
{
	SessionServer *session = SessionServer::session(key);
	if (!session)
	{	qWarning("*** ERROR: Session key %d is invalid.", key);
		return NULL;
	}

	Processor *ret = session->getProcessor(name);
	if (!ret)
	{	qWarning("*** ERROR: Processor name %s does not exist in session %d.", qPrintable(name), key);
		return NULL;
	}

	return ret;
}

};
