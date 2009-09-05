/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _RGEDDEI_ABSTRACTDOMPROCESSOR_H
#define _RGEDDEI_ABSTRACTDOMPROCESSOR_H

#include <qstring.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "abstractprocessor.h"
#else
#include <rgeddei/abstractprocessor.h>
#endif
using namespace rGeddei;

namespace Geddei { class SubProcessor; }

namespace rGeddei
{

class LocalSession;
class RemoteSession;
class RemoteDomProcessor;
class LocalDomProcessor;

/** @ingroup rGeddei
 * @brief Abstract interface class for handling Geddei DomProcessor objects.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * @note This derives from AbstractProcessor, where the type refers to the
 * Geddei Processor-derived subclass. In this object, the type, when used
 * refers instead to the Geddei SubProcessor-derived subclass.
 */
class DLLEXPORT AbstractDomProcessor: virtual public AbstractProcessor
{
  public:
	/**
	 * Create and couple a SubProcessor hosted in session @a session to this,
	 * using it as a worker.
	 *
	 * Similar concept to Geddei::DomProcessor::createAndAddWorker, but a
	 * different name to prevent confusion, since this works with sessions
	 * rather than lower level concepts.
	 *
	 * @param session The local session under which to create the SubProcessor
	 * worker object.
	 * @return true if the worker was created successfully.
	 */
	virtual bool spawnWorker(LocalSession &session) = 0;

	/**
	 * @overload
	 * Create and couple a SubProcessor hosted in session @a session to this,
	 * using it as a worker.
	 *
	 * Similar concept to Geddei::DomProcessor::createAndAddWorker, but a
	 * different name to prevent confusion, since this works with sessions
	 * rather than lower level concepts.
	 *
	 * @param session The remote session under which to create the SubProcessor
	 * worker object.
	 * @return true if the worker was created successfully.
	 */
	virtual bool spawnWorker(RemoteSession &session) = 0;

	/**
	 * Constructs a new LocalDomProcessor object that
	 * internally creates a DomProcessor using @a primary as its primary
	 * SubProcessor object.
	 *
	 * @note This object will take ownership of @a primary . You should not
	 * attempt to delete it. Typically you will use this constructor with a
	 * "new" operator:
	 *
	 * @code
	 * AbstractDomProcessor *a = AbstractDomProcessor::create(
	 *     myLocalSession, new SubProcessorType);
	 * @endcode
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param primary The SubProcessor object to be used as the DomProcessor
	 * object's primary.
	 */
	static LocalDomProcessor *create(LocalSession &session, SubProcessor *primary);

	/** @overload
	 * Constructs a new LocalDomProcessor object that
	 * represents a DomProcessor object whose primary SubProcessor object is a
	 * @a type class.
	 *
	 * @code
	 * AbstractDomProcessor *a = AbstractDomProcessor::create(
	 *     myLocalSession, "SomePluginType");
	 * @endcode
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param type The SubProcessor derived subclass, an instantiation of is to
	 * be used as the primary to the DomProcessor object this class represents.
	 */
	static LocalDomProcessor *create(LocalSession &session, const QString &type);
	
	/**
	 * Constructs a RemoteDomProcessor object together with
	 * a DomProcessor object in the RemoteSession (i.e. on that object's host).
	 *
	 * @param session The RemoteSession under which to create the DomProcessor
	 * object.
	 * @param type The SubProcessor-derived subclass to be created as the
	 * DomProcessor object's primary. You should check the availability and
	 * version with the RemoteSession object before construction.
	 */
	static RemoteDomProcessor *create(RemoteSession &session, const QString &type);
};

}

#endif
