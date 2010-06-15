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

}
