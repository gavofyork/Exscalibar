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

}
