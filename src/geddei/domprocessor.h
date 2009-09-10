/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_DOMPROCESSOR_H
#define _GEDDEI_DOMPROCESSOR_H

#include <QMutex>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "properties.h"
#include "processor.h"
#include "qfastwaitcondition.h"
#else
#include <qtextra/qfastwaitcondition.h>
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
 * from the Processor class and thus contains all the public Processor methods
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
class DLLEXPORT DomProcessor: public Processor
{
	//* Attributes of the data processing pipeline.
	uint theSamplesIn, theSamplesStep, theSamplesOut;

	//* Attributes that define how much data we want to take in normally.
	uint theNomChunks, theMaxChunks;
	double theWeighting;
	bool theAlterBuffer;
	uint theOptimalThroughput, theWantSize;

	//* Queue management stuff.
	QList<DxCoupling*> theWorkers;
	QList<DxCoupling*>::Iterator theQueuePos;
	uint theQueueLen;
	QMutex theQueueLock;
	QFastWaitCondition theQueueChanged;

	//* A flag to tell us if we've been stopped.
	bool theStopped;

	//* A flag to tell us if we should debug our action.
	bool theDebug;

	//* Settings for the load sharing stuff.
	bool theBalanceLoad;
	uint theBalanceInterval;
	float theLocalFudge;

	//* A cache of our properties, since we may need it after init.
	Properties theProperties;

	/**
	 * A flag tell us if we're in the situation where we have a queue that has
	 * been plunged but with no data after it yet.
	 */
	bool theLimbo;

	//* Primary processor references.
	friend class DxCoupling;
	SubProcessor *thePrimary;
	DSCoupling *thePrimaryCoupling;

	//* A quick QThread object for running eater().
	class EaterThread: public QThread
	{
		DomProcessor *theDomProcessor;
		virtual void run();
	public:
		EaterThread(DomProcessor *d) : theDomProcessor(d) {}
		virtual ~EaterThread() {}
	} theEaterThread;

	/**
	 * The basic eater routine.
	 * Gets called by EaterThread when it is start()ed.
	 */
	void eater();

	/**
	 * Special checkExit() variant that will *not* relock theQueueLock.
	 * This should be used instead of checkExit() when theQueueLock mutex is
	 * already in it's correct state before full stack unwinding (i.e.
	 * QMutexLockers get resolved).
	 */
	void checkExitDontLock();

	/**
	 * Does exactly as in Processor::setupVisual(). Needed here as a front-end
	 * so SubProcessor can access it.
	 *
	 * Call this from initFromProperties to initialise the visual properties of
	 * the class.
	 *
	 * If this is not called, the size will default to 50x30 and no redraw.
	 *
	 * @param width The width of the drawing canvas. Should be a multiple of
	 * 10.
	 * @param height The height of the drawing canvas. Should be a multiple of
	 * 10.
	 * @param redrawPeriod The rate for which the processor's visual should
	 * be redrawn in milliseconds. A value of zero means no explicit redraw.
	 *
	 * @sa Processor
	 */
	void setupVisual(uint width = 32, uint height = 32, uint redrawPeriod = 0) { Processor::setupVisual(width, height, redrawPeriod); }
	friend class SubProcessor;

	//* Reimplementations from Processor.
	virtual bool processorStarted();
	virtual void processor();
	virtual void wantToStopNow();
	virtual void haveStoppedNow();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void specifyInputSpace(QVector<uint> &samples);
	virtual void specifyOutputSpace(QVector<uint> &samples);
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(240, 96, 160); }

	/**
	 * Note this will assume the stack has a QMutexLocker for theQueueLock in
	 * it, and that theQueueLock has already been manually unlocked.
	 * This means that no methods that may call checkExit() should be called
	 * unless theQueueLock is unlocked.
	 */
	virtual void checkExit();

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

public:
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

};

#endif
