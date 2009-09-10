/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _RGEDDEI_LOCALDOMPROCESSOR_H
#define _RGEDDEI_LOCALDOMPROCESSOR_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "domprocessor.h"
#include "abstractprocessor.h"
#include "localprocessor.h"
#include "abstractdomprocessor.h"
#include "abstractprocessorgroup.h"
#else
#include <geddei/domprocessor.h>
#include <rgeddei/abstractprocessor.h>
#include <rgeddei/localprocessor.h>
#include <rgeddei/abstractdomprocessor.h>
#include <rgeddei/abstractprocessorgroup.h>
#endif
using namespace Geddei;
using namespace rGeddei;

namespace Geddei { class SubProcessor; }

namespace rGeddei
{

class LocalSession;
class RemoteSession;

/** @ingroup rGeddei
 * @brief LocalProcessor refinement and AbstractDomProcessor realisation.
 * @author Gav Wood <gav@kde.org>
 *
 * This class is used in exactly the same way as a LocalProcessor class. It
 * accomplishes exactly the same job. However it caters for the situation where
 * a DomProcessor is to be used. It provides convenience methods for creating
 * DomProcessor representations from a SubProcessor or existing DomProcessor
 * objects.
 */
class DLLEXPORT LocalDomProcessor : virtual public LocalProcessor, virtual public AbstractDomProcessor
{
	friend class AbstractDomProcessor;

	/**
	 * Simple constructor. Constructs a new LocalDomProcessor object that
	 * internally creates a DomProcessor using @a primary as its primary
	 * SubProcessor object.
	 *
	 * @note This object will take ownership of @a primary . You should not
	 * attempt to delete it. Typically you will use this constructor with a
	 * "new" operator:
	 *
	 * @code
	 * LocalDomProcessor l(mySession, new MySubProcessorSubClass);
	 * @endcode
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param primary The SubProcessor object to be used as the DomProcessor
	 * object's primary.
	 */
	LocalDomProcessor(LocalSession &session, SubProcessor *primary);

	/** @overload
	 * Simple constructor. Constructs a new LocalDomProcessor object that
	 * represents a DomProcessor object whose primary SubProcessor object is a
	 * @a type class.
	 *
	 * @code
	 * LocalDomProcessor l(mySession, "SomePluginType");
	 * @endcode
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param type The SubProcessor derived subclass, an instantiation of is to
	 * be used as the primary to the DomProcessor object this class represents.
	 */
	LocalDomProcessor(LocalSession &session, const QString &type);

public:
	//* Reimplementations from AbstractDomProcessor
	virtual bool spawnWorker(LocalSession &session);
	virtual bool spawnWorker(RemoteSession &session);

	/**
	 * Default destructor.
	 */
	virtual ~LocalDomProcessor();
};

};

#endif
