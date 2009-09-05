/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_MULTIPROCESSOR_H
#define _GEDDEI_MULTIPROCESSOR_H

#include <qstring.h>
#include <q3valuevector.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "multisource.h"
#include "multisink.h"
#include "properties.h"
#else
#include <geddei/multisource.h>
#include <geddei/multisink.h>
#include <geddei/properties.h>
#endif
using namespace Geddei;

namespace Geddei
{

class Processor;
class ProcessorGroup;
class MultiProcessorCreator;

/** @ingroup Geddei
 * @brief Class capable of representing multiple homegenous Processor objects.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * The MultiProcessor class is a late adjunct to the Geddei core. It enables
 * the use of multiple homegenous Processor objects to appear as a single
 * entity. This system, as a whole, is called multiplicity and is very
 * powerful. Suppose you have the situation where you wish to split a signal
 * into several discrete sub portions and process each portion's data in an
 * independent but homogenous way before recombining them accordingly.
 *
 * Normally, you would set this up for a specific number of portions, lay out
 * the network properly and begin the experiment. If the number of portions
 * changed, you would have to create a whole new network.
 *
 * With Multiplicity it is possible to declare a single path that each
 * "portion" must take, and declare a point at which a Processor's output
 * should be split (demultiplexed) to go down those paths and another point at
 * which the several paths must come together to reconstitute a single discrete
 * signal again (remultiplexing).
 *
 * We call any Processing element (i.e. a Processor and a MultiProcessor) that
 * is able to source and/or sink a perhaps configurable number of multiple
 * connections a Multiplicative object or Multiplicity-aware.
 *
 * Processor objects (and by extension SubProcessor objects) may declare them-
 * selves Multiplicative. The MultiProcessor class is automatically
 * Multiplicative, since it represents the parts of the processing path where
 * the portions are each being processed.
 *
 * It is important not to confuse the differing meanings of "parallel
 * processing" here. MultiProcessor is, technically, a method of parallel
 * processing. However, unlike DomProcessor/SubProcessor, it adds no extra
 * parallelism from the standard Geddei tools. It is meant merely as a
 * shortcut to be used in particular situations.
 */
class DLLEXPORT MultiProcessor: public MultiSource, public MultiSink
{
	friend class Processor;
	friend class ProcessorPort;

	MultiProcessorCreator *theCreator;
	Processor *theSource;

	bool theIsInitialised;
	Q3ValueVector<Processor *> theProcessors;

	/**
	 * @return true iff the quantity of multiplicity is known.
	 */
	bool knowMultiplicity() const { return theIsInitialised; }

	/**
	 * Get the quantity of multiplicity.
	 *
	 * @return The quantity of multiplicity. This value is only valid if
	 * knowMultiplicity() returns true.
	 */
	uint multiplicity() const { return theProcessors.count(); }

	/**
	 * @return true iff init() has been called on this object.
	 */
	bool initGiven() const { return theIsInitialised || theDeferredInit; }

	//* Reimplementation from Multiplicative
	virtual void doInit(const QString &name, ProcessorGroup *g, const Properties &properties);

	//* Reimplementation from MultiSource
	virtual ProcessorPort sourcePort(uint i) { return (*theProcessors[i])[0]; }

	//* Reimplementation from MultiSink
	virtual ProcessorPort sinkPort(uint i) { return (*theProcessors[i])[0]; }

public:
	/**
	 * Performs basic object construction that cannot otherwise be done in the
	 * constructor due to shared library limitations. Initialises the object
	 * fully ready for connect()s and then go().
	 *
	 * @param name This Processor instance's name, and should be unique across
	 * any group or network the processor is in.
	 * @param group A reference to the ProcessorGroup (if any) that you may
	 * use to control a set of objects.
	 * @param properties The properties with which to initialise this object.
	 * This defaults to an empty properties set. If a needed property is left
	 * undefined, the default value is taken from the properties() method.
	 */
	void init(const QString &name, ProcessorGroup &group, const Properties &properties = Properties()) { doInit(name, &group, properties); }

	/** @overload
	 * Performs basic object construction that cannot otherwise be done in the
	 * constructor due to shared library limitations. Initialises the object
	 * fully ready for connect()s and then go().
	 *
	 * @note The processor will not be initialised as a member of any group.
	 *
	 * @param name This Processor instance's name, and should be unique across
	 * any group or network the processor is in. This defaults to the empty
	 * name, which can be used iff the object is not contained in a
	 * ProcessorGroup.
	 * @param properties The properties with which to initialise this object.
	 * This defaults to an empty properties set. If a needed property is left
	 * undefined, the default value is taken from the properties() method.
	 */
	void init(const QString &name = "", const Properties &properties = Properties()) { doInit(name, 0, properties); }

	/**
	 * Basic constructor. Creates a new MultiProcessor object whose Processor
	 * constituents are created by the Creator @a c. See MultiProcessorCreator
	 * and its derivative classes for more information about specifying a
	 * suitable creator.
	 *
	 * @param c The MultiProcessorCreator to be used for creating Processor
	 * objects in this object.
	 */
	MultiProcessor(MultiProcessorCreator *c): theCreator(c), theSource(0), theIsInitialised(false) {}

	/**
	 * Default destructor.
	 */
	~MultiProcessor();
};

};

#endif
