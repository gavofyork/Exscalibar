/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <iostream>
#include <cassert>
using namespace std;

#include <qstring.h>

#include "mlconnection.h"
#include "lmconnection.h"
#include "lrconnection.h"
#include "llconnection.h"
#include "lxconnectionnull.h"
#include "processorforwarder.h"
#include "processorgroup.h"
#include "processor.h"
#include "buffer.h"
#include "splitter.h"
using namespace Geddei;

#define MESSAGES 0
#define pMESSAGES 0

namespace Geddei
{

QThreadStorage<Processor **> Processor::theOwningProcessor;

Processor::Processor(const QString &type, const MultiplicityType multi, uint flags): QThread(0), theName(""), theType(type), theFlags(flags),
	theWidth(50), theHeight(30), theGroup(0), theIOSetup(false), theStopping(false), theIsInitialised(false), theAllDone(false),
	theTypesConfirmed(false), thePaused(false), theError(NotStarted), theErrorData(0), theMulti(multi), thePlungersStarted(false), thePlungersEnded(false)
{
}

Processor::~Processor()
{
	if(MESSAGES) qDebug("Deleting Processor...");
	// Stop the threads now, as we don't want the threadCleanup() executing after
	// some of the class is deallocated!
	if(running()) stop();
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		if(theOutputs[i])
			delete theOutputs[i];
	// TODO: THIS MUST BE MUTUALLY EXCLUSIVE TO ANYTHING ACCESSING IT REMOTELY
	// (I.E. THROUGH GROUP, FOR THE ENTIRE REMOTE ACTION) --- LOCK IN GROUP?
	for(uint i = 0; i < (uint)theInputs.size(); i++)
		if(theInputs[i])
			delete theInputs[i];
	setNoGroup();
	// FINISH MUTUAL EXCLUSIVITY

	if(MESSAGES) qDebug("Deleted Processor.");
}

void Processor::startPlungers()
{
	if(pMESSAGES) qDebug("> Processor::startPlungers() [%s]: %d finished", name().latin1(), thePlungersEnded);
	QMutexLocker lock(&thePlungerSystem);
	if(!thePlungersStarted)
	{
		thePlungersStarted = true;
		thePlungersEnded = false;
		for(uint index = 0; index < numInputs(); index++)
		{	thePlungersNotified[index] = 0;
			thePlungersLeft[index] = 1;
		}
		unpause();
		for(uint i = 0; i < (uint)theOutputs.count(); i++)
			theOutputs[i]->startPlungers();
	}
	if(pMESSAGES) qDebug("< Processor::startPlungers() [%s]: %d finished", name().latin1(), thePlungersEnded);
}

void Processor::noMorePlungers()
{
	if(pMESSAGES) qDebug("> Processor::noMorePlungers() [%s]: %d finished", name().latin1(), thePlungersEnded);
	QMutexLocker lock(&thePlungerSystem);
	if(!thePlungersEnded)
	{
		thePlungersEnded = true;
		unpause();
		// Pass it on...
		for(uint i = 0; i < numOutputs(); i++)
			theOutputs[i]->noMorePlungers();
	}
	if(pMESSAGES) qDebug("< Processor::noMorePlungers() [%s]: %d finished", name().latin1(), thePlungersEnded);
}

void Processor::plungerSent(uint index)
{
	if(pMESSAGES) qDebug("> Processor::plungerSent() [%s]: %d left, %d finished", name().latin1(), thePlungersLeft[index], thePlungersEnded);
	QMutexLocker lock(&thePlungerSystem);

	thePlungersLeft[index]++;
	thePlungersNotified[index]++;
	unpause();
	//Figure out if we have already propogated notification for the thePlungersNotified[index]'th plunger.
	bool alreadyHadIt = false;
	for(uint i = 0; i < numInputs() && !alreadyHadIt; i++)
		if(thePlungersNotified[i] >= thePlungersNotified[index] && i != index)
			// Another input got here before us: Ignore the plunger.
			alreadyHadIt = true;
	if(!alreadyHadIt)
		for(uint i = 0; i < numOutputs(); i++)
			theOutputs[i]->plungerSent();
	if(pMESSAGES) qDebug("< Processor::plungerSent() [%s]: [aHI: %d] %d left, %d notified, %d finished", name().latin1(), alreadyHadIt, thePlungersLeft[index], thePlungersNotified[index], thePlungersEnded);
}

void Processor::disconnectAll()
{
	if(running())
	{	qWarning("*** WARNING: Processor::disconnect: Disconnecting input on a running processor.\n"
				 "             Stopping first.");
		stop();
	}
	for(uint i = 0; i < (uint)theOutputs.count(); i++)
		delete theOutputs[i];
}

void Processor::specifyInputSpace(Q3ValueVector<uint> &samples)
{
	for(uint i = 0; i < (uint)theInputs.size(); i++)
		samples[i] = 1;
}

void Processor::specifyOutputSpace(Q3ValueVector<uint> &samples)
{
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		samples[i] = 1;
}

void Processor::dropInput(uint index)
{
	if(running())
	{	qWarning("*** WARNING: Dropping input on a running processor. Stopping first.");
		stop();
	}
	assert(index < (uint)theInputs.size());
	if(!theInputs[index])
	{	qWarning("*** WARNING: Trying to drop a connection when there is no connection. Ignoring.");
		return;
	}

	delete theInputs[index];
}

float Processor::bufferCapacity(uint index)
{
	assert(index < (uint)theInputs.size());
	assert(theInputs[index]);
	return theInputs[index]->filled();
}

void Processor::doRegisterOut(LxConnection *me, uint port)
{
	if(MESSAGES) qDebug("Registering output link on %s.", name().latin1());
	assert(port < (uint)theOutputs.size());
	assert(theOutputs[port] == 0);
	theOutputs[port] = me;
}

void Processor::undoRegisterOut(LxConnection *me, uint port)
{
	if(MESSAGES) qDebug("Unregistering output link on %s.", name().latin1());
	assert(port < (uint)theOutputs.size());
	assert(theOutputs[port] == me);
	theOutputs[port] = 0L;
	if(MESSAGES) qDebug("Done.");
}

void Processor::doRegisterIn(xLConnection *me, uint port)
{
	if(MESSAGES) qDebug("Registering input link on %s.", name().latin1());
	assert(port < (uint)theInputs.size());
	assert(theInputs[port] == 0);
	theInputs[port] = me;
}

void Processor::undoRegisterIn(xLConnection *me, uint port)
{
	if(MESSAGES) qDebug("Unregistering input link on %s.", name().latin1());
	assert(port < (uint)theInputs.size());
	assert(theInputs[port] == me);
	theInputs[port] = 0L;
	if(MESSAGES) qDebug("Done.");
}

Processor *Processor::getCallersProcessor()
{
	if(!theOwningProcessor.localData())
		return 0;
	return *(theOwningProcessor.localData());
}

void Processor::checkExit()
{
	if(MESSAGES&&0) qDebug("Processor [%p]: Checking exit...", this);
	bool doThrow = false;
	theStop.lock();
	if(theStopping) doThrow = true;
	theStop.unlock();
	if(MESSAGES&&0) qDebug("Processor [%p]: doThrow = %d.", this, doThrow);

	if(doThrow) bail();
}

void Processor::plunged(uint index)
{
	if(MESSAGES) qDebug("> Processor::plunged() [%s:%d] thePlungedInputs=%d", name().latin1(), index, thePlungedInputs[index]);
	thePlungedInputs[index]++;
	{	QMutexLocker lock(&thePlungerSystem);
		thePlungersLeft[index]--;
	}
	if(MESSAGES) qDebug("= Processor::plunged() [%s:%d]: %d left, %d finished, %d total", name().latin1(), index, thePlungersLeft[index], thePlungersEnded, thePlungedInputs[index]);

	// If we're not the first to get this far, then exit now. We only give a plunger for the "full-speed" input.
	for(uint i = 0; i < (uint)thePlungedInputs.count(); i++)
		if(thePlungedInputs[i] >= thePlungedInputs[index] && index != i) return;
	// Otherwise this must be the first to get to this plunger: We carry on with the plunge operation.

	// We don't want to notify the user if the plunger is the one that Geddei puts at the
	// end to flush everything out.
	if(thePlungersLeft[index] || !thePlungersEnded)
		receivedPlunger();
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		theOutputs[i]->pushPlunger();
}

void Processor::resetTypes()
{
	theConfirming.lock();
	theTypesConfirmed = false;
	theConfirming.unlock();
}

void Processor::guard()
{
	thePause.lock();
	while(thePaused)
		theUnpaused.wait(&thePause);
	thePause.unlock();
	checkExit();
}

bool Processor::thereIsInputForProcessing(uint samples)
{
	guard();

	// Pull all the plungers out that we can.
	// We need to do this in order to find out if there is any input left to be
	// processed.
	// We want to block here until we can be guaranteed that we've got
	// all the plungers we can from the input. We have to do this by
	// ascertaining that the next read will read only data, not a plunger,
	// since the plunger may mean that there is no more data to be read!

	bool dataReady[numInputs()];
	for(uint i = 0; i < numInputs(); i++)
		dataReady[i] = false;

	// Keep looping here until we reach an acceptable state
	while(1)
	{
		if(pMESSAGES) qDebug("Processor [%s]: Plunging as much as possible (waiting for %d samples)...", theName.latin1(), samples);
		// Make any necessary updates...
		//TODO: this state could be cached to potentially prevent plungeSync call
		for(uint i = 0; i < (uint)theInputs.count(); i++)
			while(!dataReady[i] && thePlungersLeft[i] > 0)
				if(theInputs[i]->plungeSync(samples))
					dataReady[i] = true;

		// Could something change here?
		// thePlungersEnded could change...
		// Doesn't matter - if that changes, we note it later anyway, after lock...
		// thePlungersLeft could change... it could increment only...
		// what if it did?

		// Check, exit if necessary...
		{
			if(MESSAGES) qDebug("Processor: Locking plunger system...");
			// Lock the system for now...
			QMutexLocker lock(&thePlungerSystem);

			if(MESSAGES) qDebug("Processor: Checking status...");
			uint votesToEnd = 0;
			for(uint i = 0; i < (uint)theInputs.count(); i++)
			{
				if(pMESSAGES) qDebug("Processor [%s]: Input %d (DR: %d)...", theName.latin1(), i, dataReady[i]);
				// If there's a plunger left to be gotten and we have to process stuff before we can get it then exit.
				// FIXME: check and maybe make safe plungeSync with thePlungerSystem locked.
				if(dataReady[i]) return true;
				if(!thePlungersLeft[i] && thePlungersEnded) votesToEnd++;
			}
			if(pMESSAGES) qDebug("Processor [%s]: %d votes to end...", theName.latin1(), votesToEnd);
			if(votesToEnd == (uint)theInputs.count()) return false;

			if(MESSAGES) qDebug("Processor: Waiting for something to change...");
			// Wait until something changes...
			pause();
			thePlungerSystem.unlock();
			guard();
			thePlungerSystem.lock();
		}
	}
}

bool Processor::thereIsInputForProcessing()
{
	Q3ValueVector<uint> sready(numInputs());
	specifyInputSpace(sready);

	guard();

	// Pull all the plungers out that we can.
	// We need to do this in order to find out if there is any input left to be
	// processed.
	// We want to block here until we can be guaranteed that we've got
	// all the plungers we can from the input. We have to do this by
	// ascertaining that the next read will read only data, not a plunger,
	// since the plunger may mean that there is no more data to be read!

	bool dataReady[numInputs()];
	for(uint i = 0; i < numInputs(); i++)
		dataReady[i] = false;

	// Keep looping here until we reach an acceptable state
	while(1)
	{
		if(MESSAGES) qDebug("Processor: Plunging as much as possible...");
		// Make any necessary updates...
		//TODO: this state could be cached to potentially prevent plungeSync call
		for(uint i = 0; i < (uint)theInputs.count(); i++)
			while(!dataReady[i] && thePlungersLeft[i] > 0)
				if(theInputs[i]->plungeSync(sready[i]))
					dataReady[i] = true;

		// Could something change here?
		// thePlungersEnded could change...
		// Doesn't matter - if that changes, we note it later anyway, after lock...
		// thePlungersLeft could change... it could increment only...
		// what if it did?

		// Check, exit if necessary...
		{
			if(MESSAGES) qDebug("Processor: Locking plunger system...");
			// Lock the system for now...
			QMutexLocker lock(&thePlungerSystem);

			if(MESSAGES) qDebug("Processor: Checking status...");
			uint votesToEnd = 0;
			for(uint i = 0; i < (uint)theInputs.count(); i++)
			{
				if(MESSAGES) qDebug("Processor: Input %d...", i);
				// If there's a plunger left to be gotten and we have to process stuff before we can get it then exit.
				// FIXME: check and maybe make safe plungeSync with thePlungerSystem locked.
				if(dataReady[i]) return true;
				if(!thePlungersLeft[i] && thePlungersEnded) votesToEnd++;
			}
			if(MESSAGES) qDebug("Processor: %d votes to end...", votesToEnd);
			if(votesToEnd == (uint)theInputs.count()) return false;

			if(MESSAGES) qDebug("Processor: Waiting for something to change...");
			// Wait until something changes...
			pause();
			thePlungerSystem.unlock();
			guard();
			thePlungerSystem.lock();
		}
	}
}

void Processor::plunge()
{
	if(pMESSAGES) qDebug("= Processor::plunge() [%s]", theName.latin1());
	if(numInputs())
	{	qWarning("*** CRITICAL: Non-source Processors may not introduce plungers!");
		return;
	}
	for(uint i = 0; i < (uint)theOutputs.count(); i++)
		theOutputs[i]->plungerSent();
	for(uint i = 0; i < (uint)theOutputs.count(); i++)
		theOutputs[i]->pushPlunger();
}

void Processor::pause()
{
	QMutexLocker lock(&theStop);
	if(!theStopping)
	{	QMutexLocker lock(&thePause);
		thePaused = true;
	}
}

void Processor::unpause()
{
	thePause.lock();
	thePaused = false;
	theUnpaused.wakeAll();
	thePause.unlock();
}

void Processor::doInit(const QString &name, ProcessorGroup *g, const Properties &properties)
{
	if(MESSAGES) qDebug("Processor::init()");

	if(theIsInitialised)
	{	qWarning("*** ERROR: Processor::init(): Cannot init once already init()ed.");
		return;
	}

	if(theMulti&(In|Out) && !(theMulti&Const))
		if(!properties.keys().contains("Multiplicity"))
		{	if(MESSAGES) qDebug("Deferring...");
			theDeferredInit = true;
			theDeferredProperties = properties;
			theDeferredName = name;
			theDeferredGroup = g;
			theName = name;
			theGroup = g;
			if(theGroup) theGroup->add(this);
			return;
		}

	if(MESSAGES) qDebug("Initialising (M=%d)...", properties.keys().contains("Multiplicity") ? properties["Multiplicity"].toInt() : 0);
	theName = name;
	theGroup = g;
	if(theGroup && (!theGroup->exists(theName) || &(theGroup->get(theName)) == this)) theGroup->add(this);

	if(MESSAGES) for(uint i = 0; i < (uint)properties.keys().count(); i++) qDebug("properties[%s] = %s", properties.keys()[i].latin1(), properties[properties.keys()[i]].toString().latin1());
	Properties p = specifyProperties();
	if(MESSAGES) for(uint i = 0; i < (uint)p.keys().count(); i++) qDebug("p[%s] = %s", p.keys()[i].latin1(), p[p.keys()[i]].toString().latin1());
	p.set(properties);
	if(MESSAGES) for(uint i = 0; i < (uint)p.keys().count(); i++) qDebug("p[%s] = %s", p.keys()[i].latin1(), p[p.keys()[i]].toString().latin1());
	theGivenMultiplicity = properties.keys().contains("Multiplicity") ? p["Multiplicity"].toInt() : 0;
	initFromProperties(p);
	if(!theIOSetup)
	{	qWarning("*** ERROR: Processor::init(): initFromProperties did not setup I/O. Cannot\n"
				 "           continue. Culprit %s, named %s.", theType.latin1(), theName.latin1());
	}
	else
		theIsInitialised = true;
	theDeferredInit = false;
}

const PropertiesInfo Processor::properties() const
{
	return specifyProperties();
}

void Processor::draw(QPainter &p)
{
	paintProcessor(p);
}

void Processor::paintProcessor(QPainter &p)
{
	p.setPen(QColor(132, 132, 132));
	p.setBrush(QColor(224, 224, 224));
	p.drawRect(0, 0, width(), height());
	p.setPen(QColor(64, 64, 64));
	p.drawText(4, height() / 2 + 4, theName);
}

void Processor::setupVisual(uint width, uint height, uint redrawPeriod)
{
	theWidth = width;
	theHeight = height;
	theRedrawPeriod = redrawPeriod;
}

bool Processor::go()
{
	if(MESSAGES) qDebug("> Processor::go() (name=%s)", theName.latin1());
	if(!theIsInitialised)
	{
		if(MESSAGES) qDebug("= Processors::go() (name=%s) Not initialised!", theName.latin1());
		QMutexLocker lock(&theErrorSystem);
		theError = NotInitialised;
		theErrorWritten.wakeAll();
		if(MESSAGES) qDebug("< Processors::go() (name=%s)", theName.latin1());
		return false;
	}
	assert(!running());

	theErrorSystem.lock();
	theError = Pending;
	theErrorSystem.unlock();

	theCustomError = QString::null;
	if(!(processorStarted() && processorStarted(theCustomError)))
	{
		if(MESSAGES) qDebug("= Processors::go() (name=%s) Unable to start!", theName.latin1());
		QMutexLocker lock(&theErrorSystem);
		theError = Custom;
		theErrorWritten.wakeAll();
		if(MESSAGES) qDebug("< Processors::go() (name=%s)", theName.latin1());
		return false;
	}
	theAllDone = false;

	start(NormalPriority);
	if(MESSAGES) qDebug("< Processor::go() (name=%s)", theName.latin1());
	return true;
}

void Processor::stop()
{
	if(MESSAGES) qDebug("> Processor::stop() (name=%s)", theName.latin1());

	// This little mechanism is just in case two stop()s are called simultaneously.
	bool justExit = false;
	{	QMutexLocker lock(&theStop);
		if(!running()) return;
		if(theStopping)
			justExit = true;
		else
			theStopping = true;
	}
	if(justExit) { wait(); return; }

	// NOTE: We cannot assert(running()) here since it is possible that the process exited
	// when theStopping was set to true and doesn't require all the trapdoor stuff. It
	// doesn't make our next actions wrong, just redundant.

	thePause.lock();
	if(thePaused)
	{	thePaused = false;
		theUnpaused.wakeAll();
	}
	thePause.unlock();

	for(uint i = 0; i < (uint)theInputs.size(); i++)
		if(theInputs[i])
			theInputs[i]->sinkStopping();
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		if(theOutputs[i])
			theOutputs[i]->sourceStopping();
	if(MESSAGES) qDebug("= Processor::stop(): OK. Registering wait to stop...");
	wantToStopNow();
	if(MESSAGES) qDebug("= Processor::stop(): wait()");
	wait();
	if(MESSAGES) qDebug("= Processor::stop(): OK. Registering stopped...");
	haveStoppedNow();
	if(MESSAGES) qDebug("= Processor::stop(): Closing inputs.");
	for(uint i = 0; i < (uint)theInputs.size(); i++)
		if(theInputs[i])
			theInputs[i]->sinkStopped();
	if(MESSAGES) qDebug("= Processor::stop(): Closing outputs.");
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		if(theOutputs[i])
			theOutputs[i]->sourceStopped();

	theStop.lock();
	theStopping = false;
	theStop.unlock();

	if(MESSAGES) qDebug("< Processor::stop()");
}

bool Processor::confirmTypes()
{
	QMutexLocker lock(&theConfirming);

	if(theTypesConfirmed)
	{
		// If we're already running, best not to clear the buffer...
		{	QMutexLocker lock(&theErrorSystem);
			if(!theError) return true;
		}
		// refresh outputs in case of a reconnection
		assert(theTypesCache.count() == (uint)theOutputs.size());
		assert((uint)theSizesCache.count() == (uint)theOutputs.size());
		if(MESSAGES) qDebug("Processor::confirmTypes(): (%s) Enforcing outputs minima (from cache):", theName.latin1());
		for(uint i = 0; i != theTypesCache.count(); i++)
			if(theOutputs[i])
			{	if(MESSAGES) qDebug("Processor::confirmTypes(): (%s) Output %d : %d samples", name().latin1(), i, theSizesCache[i]);
				theOutputs[i]->setType(theTypesCache.ptrAt(i));
				theOutputs[i]->enforceMinimum(theSizesCache[i] * theTypesCache.ptrAt(i)->scope() * 2);
			}
		return true;
	}

	SignalTypeRefs inTypes(theInputs.count());
	uint ii = 0;
	for(Q3ValueVector<xLConnection *>::Iterator i = theInputs.begin(); i != theInputs.end(); i++,ii++)
	{	if(!*i)
		{	if(MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) Input not connected.", name().latin1());
			theTypesConfirmed = false;
			QMutexLocker lock(&theErrorSystem);
			theError = InputNotConnected;
			theErrorData = ii;
			theErrorWritten.wakeAll();
			return false;
		}
		const SignalType *t = (*i)->type().thePtr;
		if(!t)
		{	if(MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) Input %d is null - exiting with error.", name().latin1(), ii);
			theTypesConfirmed = false;
			QMutexLocker lock(&theErrorSystem);
			theError = InputTypeNull;
			theErrorData = ii;
			theErrorWritten.wakeAll();
			return false;
		}
		// We copy this pointer's data now, and inTypes will autodelete it on exit from function.
		inTypes.copyData(ii, t);
	}
	assert(inTypes.count() == (uint)theInputs.size());

	theTypesCache.resize(theOutputs.size());

	// Do a quick check to make sure that we're going by the multiplicity rules
	if(theMulti&In && inTypes.count())
	{		if(!inTypes.allSame())
			{	if(MESSAGES) qDebug("Processor::confirmTypes(): (%s) No input types not homogeneous.", name().latin1());
				theTypesConfirmed = false;
				QMutexLocker lock(&theErrorSystem);
				theError = InputsNotHomogeneous;
				theErrorData = ii;
				theErrorWritten.wakeAll();
				return false;
			}
	}

	theTypesConfirmed = verifyAndSpecifyTypes(inTypes, theTypesCache);
	if(!theTypesConfirmed)
	{
		if(MESSAGES) qDebug("Processor::confirmTypes(): (%s) Invalid inputs (verifyAndSpecifyTypes() returned false)", theName.latin1());
		QMutexLocker lock(&theErrorSystem);
		theError = InvalidInputs;
		theErrorWritten.wakeAll();
	}

	// NOTE: DomProcessor::verifyAndSpecifyTypes depends on this code.
	if(theMulti&Out && theTypesCache.count())
	{	if(!theTypesCache.populated(0))
		{	if(MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) No output type specified.", name().latin1());
			theTypesConfirmed = false;
			QMutexLocker lock(&theErrorSystem);
			theError = OutputsNull;
			theErrorData = ii;
			theErrorWritten.wakeAll();
			return false;
		}
		theTypesCache.setFill(theTypesCache.ptrAt(0), false);
		// TODO: Enforce the same basic class rule.
	}

	{	Q3ValueVector<uint> sizes(theInputs.count());
		specifyInputSpace(sizes);
		int ii = 0;
		if(MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) Enforcing inputs minima:", theName.latin1());
		for(Q3ValueVector<xLConnection *>::Iterator i = theInputs.begin(); i != theInputs.end(); i++, ii++)
		{	if(MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) Input %d : %d samples", name().latin1(), ii, sizes[ii]);
			// We multiply it by 2 to get the maximum of 2xoutputMin and 2xinputMin.
			// This is a cheap hack on what we really want which is outputMin+inputMin
			// For that, we will need a new API call in order to seperate the two enforceMinimum()s.
			(*i)->enforceMinimum(sizes[ii] * (*i)->type().scope());
		}
	}

	theSizesCache.resize(theOutputs.size());
	specifyOutputSpace(theSizesCache);
	if(theTypesConfirmed)
	{	assert(theTypesCache.count() == (uint)theOutputs.count());
		if(MESSAGES) qDebug("Processor::confirmInputTypes(): Enforcing outputs minima for %s:", theName.latin1());
		for(uint i = 0; i < theTypesCache.count(); i++)
			if(theOutputs[i])
			{
				if(!theTypesCache.populated(i))
					qFatal("*** FATAL: TypesCache has unpopulated entry (%d in %s). Bailing.", i, name().latin1());
				if(MESSAGES) qDebug("Processor::confirmInputTypes(): Output %d: Setting type...", i);
				theOutputs[i]->setType(theTypesCache.ptrAt(i));
				if(MESSAGES) qDebug("Processor::confirmInputTypes(): Output %d: Enforcing minimum %d", i, theSizesCache[i]);
				theOutputs[i]->enforceMinimum(theSizesCache[i] * theTypesCache.ptrAt(i)->scope() * 2);
			}
	}

	if(MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) All done (%d).", name().latin1(), theTypesConfirmed);
	return theTypesConfirmed;
}

void Processor::split(uint sourceIndex)
{
	if(running())
	{	qWarning("*** ERROR: Processor::split: %s[%d]: Cannot change connection states while running.", theName.latin1(), sourceIndex);
		return;
	}
	if(sourceIndex >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::split: %s[%d]: Invalid source index to connect from.", theName.latin1(), sourceIndex);
		return;
	}
	if(theOutputs[sourceIndex])
	{	qWarning("*** ERROR: Processor::split: %s[%d]: Cannot split an output that is already connected.", theName.latin1(), sourceIndex);
		return;
	}

	new Splitter(this, sourceIndex);
}

void Processor::share(uint sourceIndex, uint bufferSize)
{
	if(running())
	{	qWarning("*** ERROR: Processor::share: %s[%d]: Cannot change connection states while running.", theName.latin1(), sourceIndex);
		return;
	}
	if(sourceIndex >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::share: %s[%d]: Invalid source index to connect from.", theName.latin1(), sourceIndex);
		return;
	}
	if(theOutputs[sourceIndex])
	{	qWarning("*** ERROR: Processor::share: %s[%d]: Cannot share an output that is already connected.", theName.latin1(), sourceIndex);
		return;
	}

	new LMConnection(this, sourceIndex, bufferSize);
}

bool Processor::readyRegisterIn(uint sinkIndex) const
{
	if(sinkIndex >= (uint)theInputs.size())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Invalid sink index to connect to. (%p inputs: %d)", name().latin1(), sinkIndex, this, theInputs.size());
		return false;
	}
	if(theInputs[sinkIndex])
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Cannot connect to an already connected input.", theName.latin1(), sinkIndex);
		return false;
	}
	return true;
}

const Connection *Processor::connect(uint sourceIndex, Sink *sink, uint sinkIndex, uint bufferSize)
{
	if(running())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Cannot change connection states while running.", theName.latin1(), sourceIndex);
		return 0;
	}
	if(sourceIndex >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Invalid source index to connect from.", theName.latin1(), sourceIndex);
		return 0;
	}

	if(!sink->readyRegisterIn(sinkIndex)) return 0;

	if(theOutputs[sourceIndex] == 0)
		return new LLConnection(this, sourceIndex, sink, sinkIndex, bufferSize);
	else if(dynamic_cast<LMConnection *>(theOutputs[sourceIndex]))
	{
		LMConnection *c = dynamic_cast<LMConnection *>(theOutputs[sourceIndex]);
		return new MLConnection(sink, sinkIndex, c);
	}
	else if(dynamic_cast<Splitter *>(theOutputs[sourceIndex]))
	{
		Splitter *s = dynamic_cast<Splitter *>(theOutputs[sourceIndex]);
		return new LLConnection(s, 0, sink, sinkIndex, bufferSize);
	}
	else
	{	qWarning("*** ERROR: Processor::connect: Output %s[%d] already connected and is neither split\n"
			   "           nor share()d.", theName.latin1(), sourceIndex);
		return 0;
	}
}

const Connection *Processor::connect(uint sourceIndex, const QString &sinkHost, uint sinkKey, const QString &sinkProcessorName, uint sinkIndex, uint bufferSize)
{
	if(running())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Cannot change connection states while running.", theName.latin1(), sourceIndex);
		return 0;
	}
	if(sourceIndex >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Invalid source index to connect from.", theName.latin1(), sourceIndex);
		return 0;
	}

	// TODO: Need remote version of readyRegisterIn

	if(theOutputs[sourceIndex] == 0)
		return ProcessorForwarder::createConnection(this, sourceIndex, bufferSize, sinkHost, sinkKey, sinkProcessorName, sinkIndex);
	else
	{	Splitter *s = dynamic_cast<Splitter *>(theOutputs[sourceIndex]);

		if(!s)
		{	qWarning("*** ERROR: Processor::connect: %s[%d]: This output is already connected and not declared\n"
				   "           split.", theName.latin1(), sourceIndex);
			return 0;
		}

		return ProcessorForwarder::createConnection(s, 0, bufferSize, sinkHost, sinkKey, sinkProcessorName, sinkIndex);
	}
}

void Processor::disconnect(uint index)
{
	if(running())
	{	qWarning("*** WARNING: Processor::disconnect: %s[%d]: Disconnecting input on a running processor.\n"
				 "             Stopping first.", theName.latin1(), index);
		stop();
	}
	if(index >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::disconnect: %s[%d]: Invalid output index to connect from.", theName.latin1(), index);
		return;
	}
	if(!theOutputs[index])
	{	qWarning("*** ERROR: Processor::disconnect: %s[%d]: Output is not connected.", theName.latin1(), index);
		return;
	}

	delete theOutputs[index];
}

void Processor::waitUntilDone()
{
	if(!(theFlags & Guarded))
		qWarning("*** WARNING: Processor::waitUntilDone(): I'll never exit, since I'm not a\n"
				 "             Guarded Processor-derived object (name=%s).", name().latin1());
	QMutexLocker lock(&theStop);
	while(!theAllDone)
		theAllDoneChanged.wait(&theStop);
}

Processor::ErrorType Processor::waitUntilGoing(int *errorData)
{
	QMutexLocker lock(&theErrorSystem);
	while(theError == Pending || theError == NotStarted)
		theErrorWritten.wait(&theErrorSystem);
	if(errorData) *errorData = theErrorData;
	return theError;
}

const QString Processor::error() const
{
	switch(theError)
	{
	case NoError:
		return "No error has occured";
	case Pending:
		return "Operation still in progress";
	case Custom:
		return QString("processorStarted() failed (%1)").arg(theCustomError);
	case NotInitialised:
		return "Processor was never initialised";
	case InputTypeNull:
		return QString("Input port %1 has a NULL signal type").arg(theErrorData);
	case InputNotConnected:
		return QString("Input port %1 has not been connected").arg(theErrorData);
	case InputsNotHomogeneous:
		return "This is a Multiplicive Input processor and inputs are not the same signal type";
	case OutputsNull:
		return QString("No type was specified on output port %1").arg(theErrorData);
	case InvalidInputs:
		return QString("Processor connected to invalid input signal types");
	case RecursiveFailure:
		return QString("Another processor connected to output %1 has already failed").arg(theErrorData);
	case NotStarted:
		return QString("Operation not yet started");
	default:
		return QString("Unknown error (code: %1, data %2)").arg(theError).arg(theErrorData);
	}
}

void Processor::setGroup(ProcessorGroup &g)
{
	if(theGroup == &g) return;
	if(theGroup) theGroup->remove(this);
	theGroup = &g;
	if(theGroup) theGroup->add(this);
}

void Processor::setNoGroup()
{
	if(!theGroup) return;
	ProcessorGroup *d = theGroup;
	theGroup = 0L;
	if(d) d->remove(this);
}

bool Processor::waitUntilReady()
{
	return waitUntilGoing() == NoError;
}

void Processor::run()
{
	theOwningProcessor.setLocalData(new Processor *(this));

	// Fill up any output slots left
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		if(!theOutputs[i])
			theOutputs[i] = new LxConnectionNull(this, i);

	if(MESSAGES) qDebug("Processor::run(): (%s) Confirming types...", theName.latin1());
	// TODO: Look into reasons why this shouldn't go back in.
	// currently theTypesConfirmed *never* seems to be falsified after being initially
	// set to true.
	/*{	QMutexLocker lock(&theConfirming);
		theTypesConfirmed = false;
		theTypesCache.clear();
	}*/
	if(!confirmTypes()) return;

	// Wait for them to confirm their own types before we start our processing/pushing data.
	if(MESSAGES) qDebug("Processor::run(): (%s) Waiting for outputs...", theName.latin1());
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		if(theOutputs[i])
		{	if(MESSAGES) qDebug("Processor::run(): (%s) Waiting on output %d...", theName.latin1(), i);
			if(!theOutputs[i]->waitUntilReady())
			{	if(MESSAGES) qDebug("Processor::run(): (%s) Output %d had some error starting. Recursive failure imminent.", theName.latin1(), i);
				QMutexLocker lock(&theErrorSystem);
				theError = RecursiveFailure;
				theErrorData = i;
				theErrorWritten.wakeAll();
				if(MESSAGES) qDebug("Processor::run(): (%s) Error recorded. Bailing...", theName.latin1());
				return;
			}
		}

	if(MESSAGES) qDebug("Processor::run(): (%s) All tests completed. Releasing lock and starting.", theName.latin1());
	theErrorSystem.lock();
	theError = NoError;
	theErrorWritten.wakeAll();
	theErrorSystem.unlock();
	for(uint i = 0; i < (uint)thePlungedInputs.count(); i++)
		thePlungedInputs[i] = 0L;

	// Execute processor with exception handler to bail it if it throws an int
	try
	{
		if(!theInputs.count())
			for(uint i = 0; i < (uint)theOutputs.count(); i++)
				theOutputs[i]->startPlungers();

		if(MESSAGES) qDebug("Processor[%s]: Plungers primed; starting task...", name().latin1());

		processor();

		if(MESSAGES) qDebug("Processor[%s]: Task done.", name().latin1());
		{	QMutexLocker lock(&theStop);
			theAllDone = true;
			theAllDoneChanged.wakeAll();
		}

		if(MESSAGES) qDebug("Processor[%s]: Informing of no more plungers...", name().latin1());
		for(uint i = 0; i < (uint)theOutputs.count(); i++)
			theOutputs[i]->noMorePlungers();
		if(MESSAGES) qDebug("Processor[%s]: Dispatching last plunger...", name().latin1());
		// We must remember that we started expecting a plunger that we never sent, so...
		// Send plunger without a corresponding plungerSent(), in order to make it symmetrical
		for(uint i = 0; i < (uint)theOutputs.count(); i++)
			theOutputs[i]->pushPlunger();

		if(MESSAGES) qDebug("Processor[%s]: Finished. Holding until stop()ed...", theName.latin1());
		while(1)
		{	pause();
			thereIsInputForProcessing();
		}
	}
	catch(BailException &) {}
	catch(int e) {}

	if(MESSAGES) qDebug("Processor stopping (name=%s).", theName.latin1());

	processorStopped();

	if(MESSAGES) qDebug("Stopped.");
}

void Processor::bail()
{
	throw BailException();
}

void Processor::reset()
{
	if(MESSAGES) qDebug("> Processor::reset() [%s]", theName.latin1());

	thePlungersStarted = false;
	thePlungersEnded = false;
	theError = NotStarted;

	// unconfirm types --- or the next time our consumer will assume they're already confirmed (or !
	if(MESSAGES) qDebug("= Processor::reset(): Unconfirming types...");
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		if(theOutputs[i])
			theOutputs[i]->resetType();

	// undo our meddling with output slots
	if(MESSAGES) qDebug("= Processor::reset(): Deleting null outputs...");
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		if(dynamic_cast<LxConnectionNull *>(theOutputs[i]))
		{	delete theOutputs[i];
			theOutputs[i] = 0L;
		}

	if(MESSAGES) qDebug("< Processor::reset()");
}

void Processor::setupIO(uint inputs, uint outputs)
{
	assert(!running());

	for(uint i = 0; i < (uint)theInputs.size(); i++)
		if(theInputs[i]) delete theInputs[i];
	for(uint i = 0; i < (uint)theOutputs.size(); i++)
		if(theOutputs[i]) delete theOutputs[i];

	theInputs.resize(0);
	theOutputs.resize(0);

	uint rinputs = inputs, routputs = outputs;

	if(theMulti&In && !(theMulti&Const))
	{	if(rinputs != Undefined)
			qWarning("*** Processor::setupIO(): You have specified %d inputs in setupIO, but the"
					 "    processor has non-fixed multiple inputs. Overriding to multiplicity %d.", rinputs, theGivenMultiplicity);
		rinputs = theGivenMultiplicity;
	}
	else if(theMulti&In && theMulti&Const && !rinputs)
	{
		qWarning("*** Processor::setupIO() [%s]: You have specified 0 inputs in setupIO,"
				 "    but the processor has fixed multiple inputs. SetupIO aborted.", theName.latin1());
		return;
	}
	else if(!(theMulti&In && !(theMulti&Const)) && rinputs == Undefined)
		qFatal("*** Processor::setupIO(): Undefined inputs, when not unfixed Multi.");

	if(theMulti&Out && !(theMulti&Const))
	{	if(routputs != Undefined)
			qWarning("*** Processor::setupIO(): You have specified %d outputs in setupIO, but the"
					 "    processor has non-fixed multiple outputs. Overriding to multiplicity %d.", routputs, theGivenMultiplicity);
		routputs = theGivenMultiplicity;
	}
	else if(theMulti&Out && theMulti&Const && !routputs)
	{
		qWarning("*** Processor::setupIO() [%s]: You have specified 0 outputs in setupIO,"
				 "    but the processor has fixed multiple outputs. SetupIO aborted.", theName.latin1());
		return;
	}
	else if(!(theMulti&Out && !(theMulti&Const)) && routputs == Undefined)
		qFatal("*** Processor::setupIO(): Undefined outputs, when not unfixed Multi.");

	if(theMulti == InOutConst)
	{	if(rinputs != routputs)
		{	qWarning("*** Processor::setupIO(): You have specified %d inputs in setupIO, but %d"
					 "    outputs. InOutConst processors must have the same multiplicity. Overriding"
					 "    to minimum of the two (%d).", rinputs, routputs, min(rinputs, routputs));
			rinputs = routputs = min(rinputs, routputs);
		}
		if(rinputs == Undefined)
			qFatal("*** Processor::setupIO(): Undefined outputs, when fixed Multi.");
	}

	theInputs.resize(rinputs);
	for(uint i = 0; i < rinputs; i++)
		theInputs[i] = 0L;

	theOutputs.resize(routputs);
	for(uint i = 0; i < routputs; i++)
		theOutputs[i] = 0L;

	// We need to size up the plunger stuff.
	thePlungersLeft.resize(rinputs);
	thePlungersNotified.resize(rinputs);
	thePlungedInputs.resize(rinputs);

	theIOSetup = true;
}

}
