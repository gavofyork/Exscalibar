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

bool LocalDomProcessor::spawnWorker(LocalSession &session)
{
	if (MESSAGES) qDebug("LocalDomProcessor::spawnWorker(Local?)");
	if (theSession != &session)
		return dynamic_cast<DomProcessor *>(theProcessor)->createAndAddWorker(session.theHost, session.thePort);
	else
		return dynamic_cast<DomProcessor *>(theProcessor)->createAndAddWorker();
}

bool LocalDomProcessor::spawnWorker(RemoteSession &session)
{
	if (MESSAGES) qDebug("LocalDomProcessor::spawnWorker(Node)");
	return dynamic_cast<DomProcessor *>(theProcessor)->createAndAddWorker(session.theHost, session.theKey);
}

}
