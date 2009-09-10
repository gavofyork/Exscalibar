/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _RGEDDEI_ABSTRACTPROCESSOR_H
#define _RGEDDEI_ABSTRACTPROCESSOR_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "processor.h"
#include "abstractprocessorport.h"
#else
#include <geddei/processor.h>
#include <rgeddei/abstractprocessorport.h>
#endif
using namespace Geddei;
using namespace rGeddei;

namespace Geddei
{
	class SubProcessor;
}

namespace rGeddei
{

class LocalProcessor;
class RemoteProcessor;
class LocalDomProcessor;
class RemoteDomProcessor;
class LocalSession;
class RemoteSession;
class AbstractProcessor;
class AbstractProcessorGroup;

/** @ingroup rGeddei
 * @brief Abstract interface class for handling Geddei Processor objects.
 * @author Gav Wood <gav@kde.org>
 *
 * The AbstractProcessor class is the result of the powerful abstaction
 * technique in rGeddei. It defines an interface to both Geddei Processor
 * objects that reside inside this session (i.e. LocalProcessor objects) and
 * Geddei Processor objects that reside in sessions that are non-local (i.e.
 * RemoteProcessor objects).
 *
 * Because both LocalProcessor objects and RemoteProcessor objects are derived
 * from AbstractProcessor objects they can be used whereever an
 * AbstractProcessor pointer is called for.
 *
 * You might like to think of an AbstractProcessor object as a proxy or
 * representative of a real Geddei Processor object. An AbstractProcessor has
 * no actual "guts" - it is merely an abstract container, or a location
 * transparent pointer to a real Processor object.
 */
class DLLEXPORT AbstractProcessor
{
	AbstractProcessorGroup *theGroup;

protected:
	/** @internal
	 * Actually carry out the init operation. Backend virtual void methods that
	 * is used by the front-end (non-virtual) methods.
	 *
	 * @param name The name for the Processor object.
	 * @param g The AbstractProcessorGroup (if any) it is to be a member of.
	 * @param p The Properties is it to be initialised with.
	 */
	virtual void doInit(const QString &name, AbstractProcessorGroup *g, const Properties &p) = 0;

	/** @internal
	 * Empty constructor for allowing full construction customisability to
	 * child classes.
	 */
	AbstractProcessor() : theGroup(0) {}

public:
	/**
	 * Sets the AbstractProcessorGroup that this object is a member of @a g. If
	 * this object is already a member of another group it will first be
	 * removed before joining @a g .
	 *
	 * @param g The group this AbstractProcessor is to become a member of.
	 *
	 * @note Unlike other methods, this operation concerns this object, not the
	 * Processor that it represents.
	 *
	 * @sa setNoGroup()
	 */
	void setGroup(AbstractProcessorGroup &g);

	/**
	 * Resets the group of this object. The AbstractProcessor will not be associated
	 * with any group after this call.
	 *
	 * @note Unlike other methods, this operation concerns this object, not the
	 * Processor that it represents.
	 *
	 * For internal concern: This should be made at the end of any derived
	 * types' destructors.
	 *
	 * @sa setGroup()
	 */
	void setNoGroup();

	/**
	 * Sends a message to the Processor object that this represents, telling it
	 * to start.
	 *
	 * @return true iff the Processor was primed to start OK.
	 *
	 * @sa Geddei::Processor::go()
	 */
	virtual bool go() = 0;

	/**
	 * Sends a message to the Processor object that this represents, telling it
	 * to wait until the start() operation has completed.
	 *
	 * @param errorData Pointer to an integer, which if non-zero will be filled
	 * with some data pertaining to the error (if any) that occured in starting
	 * the Processor object.
	 * @return A Geddei::Processor::ErrorType value describing the type of error (if
	 * any that occured in starting the Processor object.
	 *
	 * @sa Geddei::Processor::waitUntilGoing()
	 */
	virtual Processor::ErrorType waitUntilGoing(int *errorData = 0) = 0;

	/**
	 * Sends a message to the Processor object that this represents, telling it
	 * to wait until the processor() operation has completed.
	 *
	 * @sa Geddei::Processor::waitUntilDone()
	 */
	virtual void waitUntilDone() = 0;

	/**
	 * Sends a message to the Processor object that this represents, telling it
	 * to stop.
	 *
	 * @sa Geddei::Processor::stop()
	 */
	virtual void stop() = 0;

	/**
	 * Sends a message to the Processor object that this represents, telling it
	 * to reset.
	 *
	 * @sa Geddei::Processor::reset()
	 */
	virtual void reset() = 0;

	/**
	 * Sends a message to the Processor object that this represents, telling it
	 * to split the output port @a sourceIndex.
	 *
	 * @param sourceIndex The output port index to be split.
	 *
	 * @sa Geddei::Processor::split()
	 */
	virtual void split(uint sourceIndex) = 0;

	/**
	 * Sends a message to the Processor object that this represents, telling it
	 * to share the output port @a sourceIndex.
	 *
	 * @param sourceIndex The output port index to be share.
	 *
	 * @sa Geddei::Processor::share()
	 */
	virtual void share(uint sourceIndex) = 0;

	/**
	 * Attempts to create a connection between the Processor object this
	 * represents and another RemoteProcessor object.
	 *
	 * The connection will be formed between the output port of index
	 * @a sourceIndex of the Processor object this represents and the input
	 * port @a sinkIndex of the Processor represented by the RemoteProcessor
	 * @a sink.
	 *
	 * @param sourceIndex The index of the output port on this side of the
	 * connection.
	 * @param sink The RemoteProcessor object to whose Processor object we wish
	 * to connect.
	 * @param sinkIndex The index of the input port on the opposite side of the
	 * connection.
	 * @param bufferSize A minimum size of Buffer. Leave this at 1 unless you
	 * have a good reason not to.
	 * @return true if the connection succeeded.
	 *
	 * @sa Geddei::Processor::connect()
	 */
	virtual bool connect(uint sourceIndex, const RemoteProcessor *sink, uint sinkIndex, uint bufferSize = 1) = 0;

	/** @overload
	 * Attempts to create a connection between the Processor object this
	 * represents and another LocalProcessor object.
	 *
	 * The connection will be formed between the output port of index
	 * @a sourceIndex of the Processor object this represents and the input
	 * port @a sinkIndex of the Processor represented by the LocalProcessor
	 * @a sink.
	 *
	 * @param sourceIndex The index of the output port on this side of the
	 * connection.
	 * @param sink The LocalProcessor object to whose Processor object we wish
	 * to connect.
	 * @param sinkIndex The index of the input port on the opposite side of the
	 * connection.
	 * @param bufferSize A minimum size of Buffer. Leave this at 1 unless you
	 * have a good reason not to.
	 * @return true if the connection succeeded.
	 *
	 * @sa Geddei::Processor::connect()
	 */
	virtual bool connect(uint sourceIndex, const LocalProcessor *sink, uint sinkIndex, uint bufferSize = 1) = 0;

	/**
	 * Disguard all connections going from a given port (generally only one,
	 * unless the port has a split() or share() operation carried out on it).
	 *
	 * @param sourceIndex The output port from whom all connection are to be
	 * destroyed.
	 *
	 * @sa Geddei::Processor::disconnect()
	 */
	virtual void disconnect(uint sourceIndex) = 0;

	/**
	 * Disguard all connections going from this object.
	 *
	 * @sa Geddei::Processor::disconnectAll()
	 */
	virtual void disconnectAll() = 0;

	/**
	 * Retreive the unique identifier of this object.
	 *
	 * @return The name of the Processor object this represents, and by
	 * extension, this object.
	 *
	 * @sa Geddei::Processor::name()
	 */
	virtual const QString name() const = 0;

	/**
	 * Initialises the Processor object this represents.
	 *
	 * @param name is the unique name (application-wide) you may refer to this
	 * object by with the AbstractProcessorGroup.
	 * @param g is the AbstractProcessorGroup it will belong to.
	 * @param p defines the Properties the object this represents is
	 * initialised with.
	 */
	void init(const QString &name, AbstractProcessorGroup &g, const Properties &p = Properties()) { doInit(name, &g, p); }

	/** @overload
	 * Initialises the Processor object this represents.
	 *
	 * This Processor object will not initially belong to any group.
	 *
	 * @param name is the unique name the Processor object this represents is
	 * given. If an empty string(default) then a unique name will be created.
	 * Never specifiy a non-unique (session-wide) name!
	 * @param p defines the Properties the object this represents is
	 * initialised with.
	 *
	 * The processor will not initially be a member of any group.
	 */
	void init(const QString &name = "", const Properties &p = Properties()) { doInit(name, 0, p); }

	/**
	 * Returns convenience object that represents one of the Processor object's
	 * input/outputs. It can be used for creating connections in a simple
	 * manner.
	 *
	 * @param port The index of the port that you wish to reference.
	 * @return A ProcessorPort object that refers to the given i/o port. Due to the
	 * linearity of the parameter, input and output cannot be differentiated here
	 * and is instead left to later on in its usage for clarification.
	 */
	AbstractProcessorPort operator[](uint port) { return AbstractProcessorPort(this, port); }

	/** @overload
	 * Constructs a LocalProcessor object that uses, but
	 * does not adopt the given Processor object.
	 *
	 * @note This object will not take ownership of @a processor . You must
	 * delete it when finished with.
	 *
	 * However, you should not attempt to control any of the base Geddei
	 * functionality directly. Use this class for that control instead.
	 *
	 * Example:
	 *
	 * @code
	 * MyCustomProcessor p;
	 * AbstractProcessor *a = AbstractProcessor::create(mySession, p);
	 *
	 * // Correct:
	 * l.init();
	 *
	 * // Wrong:
	 * //p.init();
	 *
	 * delete a;
	 * @endcode
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param processor A reference to the processor which this LocalProcessor
	 * object will...
	 */
	static LocalProcessor *create(LocalSession &session, Processor &processor);

	/**
	 * Constructs a LocalProcessor object that adopts the
	 * given Processor object.
	 *
	 * @note This object will take ownership of @a processor . You should not
	 * attempt to delete it or other control any of the base Geddei
	 * functionality. Use this class for that control instead. Typically you
	 * will use this constructor with a "new" operator:
	 *
	 * @code
	 * AbstractProcessor *l = AbstractProcessor::create(
	 *     mySession, new MyCustomProcessorSubClass);
	 * @endcode
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param processor A pointer to the processor which this LocalProcessor
	 * object will take ownership.
	 */
	static LocalProcessor *create(LocalSession &session, Processor *processor);

	/** @overload
	 * Constructs a LocalProcessor object together with its
	 * associated (and owned) Processor object.
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param type The Processor-derived subclass to be created. The resident
	 * ProcessorFactory singleton will be used for creation, so the Processor
	 * derived subclass must be available as a plugin.
	 */
	static LocalProcessor *create(LocalSession &session, const QString &type);

	/**
	 * Constructs a RemoteProcessor object together with a
	 * Processor object in the RemoteSession (i.e. on that object's host).
	 *
	 * @param session The RemoteSession under which to create the Processor
	 * object.
	 * @param type The Processor-derived subclass to be created. You should
	 * check the availability and version with the RemoteSession object before
	 * construction.
	 */
	static RemoteProcessor *create(RemoteSession &session, const QString &type);

	virtual ~AbstractProcessor() {}
};

};

#endif
