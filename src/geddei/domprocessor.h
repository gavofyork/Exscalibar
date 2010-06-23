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

#include <QMutex>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "properties.h"
#include "bufferdatas.h"
#include "processor.h"
#include "qfastwaitcondition.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <geddei/bufferdatas.h>
#include <geddei/properties.h>
#include <geddei/processor.h>
#endif

namespace Geddei
{

class DSCoupling;
class DxCoupling;
class SubProcessor;

/** @ingroup Geddei
 * @brief The Processor-derived class for handling SubProcessor objects.
 * @author Gav Wood <gav@kde.org>
 * DomProcessor can be thought of as a generic Processor class which is
 * specialised not at compile time but at runtime. It is publicly derived
 * from the Processor class and thus contains all the public CoProcessor methods
 * as you might expect, however they are written so that a SubProcessor object
 * can be "plugged in" to it in order to cement the specialisation.
 *
 * SubProcessor derived objects have no method of direct control; DomProcessor
 * acts as the interface for using SubProcessor objects in this respect.
 *
 * Becuase DomProcessor objects have no inate profession, they require a
 * SubProcessor object at construction time. This may be provided by either a
 * pointer to an instance (easiest in an development scenario), or by a
 * string describing the SubProcessor's type which the SubProcessor factory can
 * create (most useful in a release scenario with plugins abound). This object
 * is forever after refered to as the /primary/ SubProcessor object.
 *
 * There are two main advantages with coding in the Sub/DomProcessor paradigm,
 * both of these are due to the constraints attached to the SubProcessor, in
 * that it is stateless. The first is the extra simplicity afforded in coding
 * it. The second is that it may be implicitly parallelised. This means, in
 * essence, that a DomProcessor may have not just one but many SubProcessor
 * objects working under it. Each of these SubProcessor objects are refered to
 * as /workers/.
 *
 * This has significant and interesting repercussions in environments where
 * multiple CPUs are available or multiple machines may be accessed.
 *
 * In the current API the SubProcessor objects cannot (and should not) be
 * accessed exclusively once under the control of a DomProcessor. So they
 * cannot be deleted explicitly. The DomProcessor object essentially owns the
 * SubProcessors and will take care of any deletions that must happen at the
 * end of its lifetime.
 *
 * Three methods are available for introducing extra SubProcessor objects to a
 * DomProcessor; addWorker(), and two forms of createAndAddWorker. addWorker()
 * allows the developer to control the creation process of the SubProcessor,
 * the other two are "easier" since they will automatically create the
 * SubProcessor for you, using the SubProcessorFactory. This, of course, only
 * works if the SubProcessor class you have constructed the DomProcessor with
 * is available as a plugin. If not you'll just have to use addWorker.
 */
class DLLEXPORT DomProcessor: public CoProcessor
{
	//* Attributes of the data processing pipeline.
	uint theSamplesIn;
	uint theSamplesStep;
	uint theSamplesOut;

	//* Attributes that define how much data we want to take in normally.
	uint theNomChunks;
	uint theMaxChunks;
	bool theAlterBuffer;
	bool theDebug;
	uint theOptimalThroughput;
	uint theWantSamples;
	uint theWantChunks;
	float theWeighting;

	//* A cache of our properties, since we may need it after init.
	Properties theProperties;

	SubProcessor *thePrimary;

	QList<DxCoupling*> theWorkers;
	BufferDatas theCurrentIns;
	BufferDatas theCurrentOuts;
	bool serviceSubs();

	virtual bool processorStarted();
	virtual int canProcess();
	virtual int process();
	virtual void wantToStopNow();
	virtual void haveStoppedNow();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual void specifyInputSpace(QVector<uint> &samples);
	virtual void specifyOutputSpace(QVector<uint> &samples);
	virtual void requireInputSpace(QVector<uint> &samples);
	virtual QColor specifyOutlineColour() const;

	/**
	 * Adds another SubProcessor object to this Processor's list of "workers".
	 *
	 * Uses a DSCoupling for the connection (an efficient local shared memory
	 * link).
	 *
	 * @param worker The SubProcessor object to be added as a worker. This must
	 * be the same class as the primary.
	 */
	void addWorker(SubProcessor *worker);

	void onMultiplicitySet(uint _m);
	void onIOSetup();

public:
	/**
	 * Note this will assume the stack has a QFastMutexLocker for theQueueLock in
	 * it, and that theQueueLock has already been manually unlocked.
	 * This means that no methods that may call checkExit() should be called
	 * unless theQueueLock is unlocked.
	 */
	virtual void checkExit() { Processor::checkExit(); } // This can be removed once SubProcessors are coop.

	/** @internal
	 * To be called from the constructor of a mixin-ed coupling. This ratifies
	 * the coupling ready for use and does any required post-initialisation on
	 * it.
	 *
	 * This really belongs in DxCoupling, but it has to be called after the
	 * left side is constructed, since it uses said methods.
	 */
	void ratify(DxCoupling *c);

	/**
	 * Creates and adds a local SubProcessor to this Processor's list of
	 * workers. Uses SubProcessorFactory for the creation, and the primary for
	 * the type.
	 *
	 * Uses a DSCoupling for the connection (an efficient local shared memory
	 * link).
	 *
	 * @return true iff a worker was added.
	 */
	bool createAndAddWorker();

	/**
	 * Creates and adds a remote SubProcessor to this Processor's list of
	 * workers. Uses a DR/RSCoupling for the connection (a remote TCP/IP link).
	 *
	 * @param host The host on which the SubProcessor should be added. This
	 * must be running the Geddei nodeserver.
	 * @param key The session key under which the SubProcessor will be added.
	 * @return true iff a worker was added.
	 */
	bool createAndAddWorker(const QString &host, uint key);

	SubProcessor* primary() const { return thePrimary; }

	/**
	 * Constructor. A valid primary SubProcessor must be passed in @a primary.
	 * This is to determine the type of DomProcessor, and provide at leat one
	 * worker.
	 *
	 * @param primary The primary SubProcessor object. This must be valid.
	 */
	DomProcessor(SubProcessor *primary);

	/**
	 * Constructor. Creates a new SubProcessor object of type given, then initialises
	 * DomProcessor as a primary of this type (to provide at least one worker).
	 *
	 * @param primaryType A valid and factory-creatable type of SubProcessor.
	 */
	DomProcessor(const QString &primaryType);

	/**
	 * Default destructor.
	 */
	virtual ~DomProcessor();
};

}
