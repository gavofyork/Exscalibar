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

#pragma once

#include <qstring.h>

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "processorforwarder.h"
#include "processorgroup.h"
#else
#include <geddei/processorforwarder.h>
#include <geddei/processorgroup.h>
#endif
using namespace Geddei;

namespace rGeddei
{

class LocalProcessor;
class RemoteProcessor;
class LocalDomProcessor;
class RemoteDomProcessor;

/** @ingroup rGeddei
 * @brief Control object for coordinating local Processor objects with remote.
 * @author Gav Wood <gav@kde.org>
 *
 * This is the local equivalent to the RemoteSession class, this is a very
 * basic class to provide a similar mechanism to creating rGeddei
 * LocalProcessor objects in the local space but have them be able to simply
 * connect to the other rGeddei objects (i.e. RemoteProcessor objects).
 *
 * The LocalSession class extends a class known as a ProcessorForwarder. This
 * allows it to open a port on the local host neccessary to forward incoming
 * data connections to the relevant processor. Each LocalSession object must
 * have its own port, though in all normal circumstances only one LocalSession
 * object is needed.
 *
 * The LocalSession object's forwarding capabilities make use of Qt's event
 * loop, and as such any application that uses this class *must* be running
 * Qt's event-driven loop. The easy way to do this is to use QSubApp.
 *
 * Tip: It is usually a good idea to declare the LocalSession object in the
 * globalscope.
 */
class DLLEXPORT LocalSession: private ProcessorForwarder, private ProcessorGroup
{
	QString theHost;
	uint thePort;

	friend class LocalProcessor;
	friend class RemoteProcessor;
	friend class LocalDomProcessor;
	friend class RemoteDomProcessor;

	virtual Processor *lookup(uint /*key*/, const QString &name)
	{
		return &(get(name));
	}

public:
	/**
	 * Basic constructor. Constructs a LocalSession object on this host, which
	 * should be able to be referred to as @a host, with the incoming
	 * data connections on port @a port.
	 *
	 * @param host The hostname that this machine should be able to be
	 * contacted at. Specify as a numeric address for safety.
	 * @param port The port that should be used for listenning for incoming
	 * Geddei connections. Default is the default Geddei port minus 1, 16660.
	 * We subtract 1 in order that we can use a nodeserver on this machine too,
	 * without a default port clash.
	 */
	LocalSession(const QString &host, uint port = GEDDEI_PORT - 1);
};

}
