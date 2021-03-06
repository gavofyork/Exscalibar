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

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "domprocessor.h"
#include "abstractprocessor.h"
#include "remoteprocessor.h"
#include "abstractdomprocessor.h"
#include "abstractprocessorgroup.h"
#else
#include <geddei/domprocessor.h>
#include <rgeddei/abstractprocessor.h>
#include <rgeddei/remoteprocessor.h>
#include <rgeddei/abstractdomprocessor.h>
#include <rgeddei/abstractprocessorgroup.h>
#endif
using namespace Geddei;
using namespace rGeddei;

namespace rGeddei
{

/** @ingroup rGeddei
 * @brief RemoteProcessor refinement and AbstractDomProcessor realisation.
 * @author Gav Wood <gav@kde.org>
 *
 * This class is used in exactly the same way as a RemoteProcessor class. It
 * accomplishes exactly the same job. However it caters for the situation where
 * a DomProcessor is to be used. It provides convenience methods for creating
 * DomProcessor representations directly from a SubProcessor type (i.e. class
 * name).
 */
class DLLEXPORT RemoteDomProcessor : virtual public RemoteProcessor, virtual public AbstractDomProcessor
{
	friend class AbstractDomProcessor;

	/**
	 * Basic constructor. Constructs a RemoteDomProcessor object together with
	 * a DomProcessor object in the RemoteSession (i.e. on that object's host).
	 *
	 * @param session The RemoteSession under which to create the DomProcessor
	 * object.
	 * @param type The SubProcessor-derived subclass to be created as the
	 * DomProcessor object's primary. You should check the availability and
	 * version with the RemoteSession object before construction.
	 */
	RemoteDomProcessor(RemoteSession &session, const QString &type);

public:
	//* Reimplementations from AbstractDomProcessor
	virtual bool spawnWorker(LocalSession &session);
	virtual bool spawnWorker(RemoteSession &session);

	/**
	 * Default destructor.
	 */
	virtual ~RemoteDomProcessor();
};

}
