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

#include <cmath>

#include <qdatetime.h>
//Added by qt3to4:
#include <Q3PtrList>

#include "dxcoupling.h"
#include "dscoupling.h"
#include "subprocessor.h"
#include "domprocessor.h"
#include "subprocessorfactory.h"
#include "processorforwarder.h"

#define MESSAGES 0
#define hMESSAGES 0
#define lMESSAGES 0

namespace Geddei
{

DomProcessor::DomProcessor(SubProcessor *primary) : Processor("DomProcessor", primary->theMulti), theQueuePos(theWorkers.begin()), thePrimary(primary), theEaterThread(this)
{
	primary->thePrimaryOf = this;
	thePrimaryCoupling = new DSCoupling(this, primary);
}

DomProcessor::DomProcessor(const QString &primaryType) : Processor("DomProcessor", (thePrimary = SubProcessorFactory::create(primaryType))->theMulti), theQueuePos(theWorkers.begin()), theEaterThread(this)
{
	thePrimary->thePrimaryOf = this;
	thePrimaryCoupling = new DSCoupling(this, thePrimary);
}

DomProcessor::~DomProcessor()
{
	delete thePrimaryCoupling;
	// ~DSCoupling removes the ptr from the list, probably as theWorkers.clear()
	// traverses it, so we make a proxy copy.
	Q3PtrList<DxCoupling> copyWorkers = theWorkers;
	copyWorkers.setAutoDelete(true);
	copyWorkers.clear();
	assert(theWorkers.isEmpty());
}

void DomProcessor::paintProcessor(QPainter &p)
{
	thePrimary->paintProcessor(p);
}

bool DomProcessor::createAndAddWorker()
{
	SubProcessor *sub = SubProcessorFactory::create(thePrimary->theType);
	if(!sub) return false;
	addWorker(sub);
	return true;
}

bool DomProcessor::createAndAddWorker(const QString &host, uint key)
{
	return ProcessorForwarder::createCoupling(this, host, key, thePrimary->theType) != 0;
}

void DomProcessor::wantToStopNow()
{
	if(MESSAGES) qDebug("DomProcessor[%s]: Initiating stop..", theName.latin1());

	if(MESSAGES) qDebug("DomProcessor[%s]: Unlocking queue...", theName.latin1());
	{	QMutexLocker lock(&theQueueLock);
		theQueueChanged.wakeAll();
	}

	if(MESSAGES) qDebug("DomProcessor[%s]: %d workers.", theName.latin1(), theWorkers.count());

	if(MESSAGES) qDebug("DomProcessor[%s]: Informing couplings...", theName.latin1());
	for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
		(*w)->stopping();
	if(MESSAGES) qDebug("DomProcessor[%s]: OK.", theName.latin1());
}

void DomProcessor::haveStoppedNow()
{
	if(MESSAGES) qDebug("DomProcessor[%s]: Stopped.", theName.latin1());

	if(MESSAGES) qDebug("DomProcessor[%s]: Waiting on eater...", theName.latin1());
	theEaterThread.wait();

	if(MESSAGES) qDebug("DomProcessor[%s]: Closing trapdoors...", theName.latin1());
	for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
		(*w)->stopped();

	if(MESSAGES) qDebug("DomProcessor[%s]: Stopping workers...", theName.latin1());
	for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
		(*w)->stop();

	if(MESSAGES) qDebug("DomProcessor[%s]: Deleting our readers and resurecting...", theName.latin1());
	for(uint i = 0; i < numInputs(); i++)
		if(theInputs[i])
		{	for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
				delete (*w)->theReaders[i];
			theInputs[i]->resurectReader();
		}
}

bool DomProcessor::processorStarted()
{
	if(MESSAGES) qDebug("DomProcessor[%s]: Starting...", theName.latin1());

	// Sort out all the buffer readers.
	for(uint i = 0; i < numInputs(); i++)
		if(theInputs[i])
			theInputs[i]->killReader();
			// In fact otherwise this would be a fatal error, but now it not the time to catch that.
			// It will be caught in confirmTypes instead.
	for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
	{	(*w)->theReaders.resize(numInputs());
		for(uint i = 0; i < numInputs(); i++)
			if(theInputs[i])
				(*w)->theReaders[i] = theInputs[i]->newReader();
	}
	if(MESSAGES) qDebug("DomProcessor[%s]: Startup done (workers: %d)", theName.latin1(), theWorkers.count());
	return true;
}

void DomProcessor::processor()
{
	if(MESSAGES && theDebug) qDebug("DomProcessor[%s]::processor(): Starting main loop...", theName.latin1());

	// Figure out theNomChunks from the buffer capacities.
	uint minInCap = Undefined;
	for(uint i = 0; i < numInputs(); i++)
		if(minInCap > (*theWorkers.begin())->theReaders[i]->size() / (*theWorkers.begin())->theReaders[i]->type()->scope())
			minInCap = (*theWorkers.begin())->theReaders[i]->size() / (*theWorkers.begin())->theReaders[i]->type()->scope();
	theNomChunks = ((minInCap - theSamplesIn) / theSamplesStep + 1) / theWorkers.count();
	uint minOutCap = Undefined;
	for(uint i = 0; i < numOutputs(); i++)
		if(minOutCap > output(i).maximumScratchSamplesEver())
			minOutCap = output(i).maximumScratchSamplesEver();
	if(minOutCap != Undefined)
		theNomChunks = min(theNomChunks, minOutCap / theSamplesOut / theWorkers.count());

	// We set nom to halve of the maximum possible in order to behave normally when the output
	// processor hits its worst case of having a minimum pushing chunk of size n/2+1 where n is
	// the size of the buffer. In this case it is only able to push one chunk at once so the
	// buffer could never be more than n/2+1 full at any one time; if we therefore only ever wait
	// for halve the maximum possible chunks then we should be ok.
	theNomChunks = (theNomChunks + 1 - theSamplesIn / theSamplesStep) / 2;

	if(lMESSAGES && theDebug) qDebug("DomProcessor[%s]: tNC=%d, mIC=%d, mOC=%d)...", theName.latin1(), theNomChunks, minInCap, minOutCap);

	// Start all sub-processors
	for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
	{	(*w)->theLoad = theNomChunks;
		(*w)->go();
	}

	QMutexLocker qlock(&theQueueLock);
	theQueuePos = theWorkers.begin();
	theQueueLen = 0;
	theLimbo = false;

	if(MESSAGES && theDebug) qDebug("DomProcessor[%s]: Starting eater thread...", theName.latin1());
	theStopped = false;
	theEaterThread.start();

	bool wasPlunger = false;

	Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin();
	while(1)
	{

		// Wait until there's room in the queue for another job, or the last iteration just pushed a plunger
		// (which means the queue may look full, but the current worker is, in fact, not busy, since it only
		// pushed a plunger).
		while(theQueueLen == theWorkers.count() && !wasPlunger)
		{	checkExitDontLock();
			theQueueChanged.wait(&theQueueLock);
		}

		// Now there's room, and since no other process will ever fill up the queue, we can relinquish our lock.
		theQueueLock.unlock();

		// Calculate how much data we're going to try to process this iteration.
		uint wouldReadSamples = theSamplesStep * (max(1, (*w)->theLoad) - 1) + 3;

		// This is some data to describe what we actually are doing this iteration.
		uint willReadSamples, willReadChunks, discardFromOthers = 0;

		// Wait for the data to be ready...
		if(MESSAGES && theDebug) qDebug("DomProcessor[%s]: Waiting for a nominal (%d) or at least minimal (%d) amount of data...", theName.latin1(), wouldReadSamples, theSamplesIn);
		(*w)->theReaders[0]->waitForElements(wouldReadSamples * (*w)->theReaders[0]->type()->scope());

		// Make sure that we don't have to exit now.
		if(MESSAGES && theDebug) qDebug("DomProcessor[%s]: Checking exit status...", theName.latin1());
		checkExit();

		// Find out how much we will be reading
		uint availableSamples = min(wouldReadSamples, (*w)->theReaders[0]->type()->samples((*w)->theReaders[0]->elementsReady()));
		if(MESSAGES && theDebug) qDebug("DomProcessor[%s]: Found %d samples ready for reading...", theName.latin1(), availableSamples);

		// If we don't get to read a useful amount (i.e. less than one single chunk)
		if(availableSamples < theSamplesIn)
		{
#ifdef EDEBUG
			if((*w)->theReaders[0]->elementsReady() % (*w)->theReaders[0]->type()->scope())
				qWarning("*** CRITICAL: A plunger is not on a sample boundary! The stream is corrupt.\n");
#endif
			// We don't actually read anything
			willReadChunks = 0;
			willReadSamples = 0;

			// But we set it up so that later we discard all the samples from the others.
			discardFromOthers = availableSamples;

			// We must do us first, since another reader may be already at the plunger
			//  and thus cannot read past it. Then must then send the plunger immediately,
			//  since other readers may be ahead of us and unable to read past it.
			if(MESSAGES && theDebug) qDebug("DomProcessor[%s]: Plunger after %d samples. Skipping samples and plunging on %p...", theName.latin1(), availableSamples, *w);
			(*w)->skipPlungeAndSend(availableSamples);
			checkExit();
		}
		else
		{	willReadChunks = ((availableSamples - theSamplesIn) / theSamplesStep) + 1;
			willReadSamples = theSamplesStep * (willReadChunks - 1) + theSamplesIn;
			discardFromOthers = willReadChunks * theSamplesStep;

			if(MESSAGES && theDebug) qDebug("DomProcessor[%s]: Found %d samples (%d chunks) available. Reading & sending to %p...", theName.latin1(), willReadSamples, willReadChunks, *w);
			(*w)->peekAndSend(willReadSamples, willReadChunks);
			(*w)->skip(discardFromOthers);
			checkExit();
		}

		// If this is the first time we're pushing something to this SubProc, then we want the
		// queue len to increase. It would be the first time if the last time was data.
		if(MESSAGES && theDebug) qDebug("DomProcessor: Checking wasPlunger (%d).", wasPlunger);
		theQueueLock.lock();
		if(!wasPlunger)
		{
			if(MESSAGES && theDebug) qDebug("DomProcessor: Incrementing queue length.");
			theQueueLen++;
			theQueueChanged.wakeAll();
		}
		theLimbo = false;
		theQueueChanged.wakeAll();
		theQueueLock.unlock();

		if(hMESSAGES && theDebug) (*w)->theReaders[0]->debug();
		if(MESSAGES && theDebug) qDebug("DomProcessor: Skipping %d samples on others...", discardFromOthers);
		for(Q3PtrList<DxCoupling>::Iterator x = theWorkers.begin(); x != theWorkers.end(); x++)
			if(*x != *w) (*x)->skip(discardFromOthers);

		if(hMESSAGES && theDebug) (*w)->theReaders[0]->debug();

		// If we processed real data, then we want to go on.
		// If we only processed a plunger, we want to stay still.
		if(willReadChunks)
		{	w++;
			if(w == theWorkers.end()) w = theWorkers.begin();
			if(MESSAGES && theDebug)
			{	qDebug("\n---- NEXT WORKER ----");
				char c = 'A';
				for(Q3PtrList<DxCoupling>::Iterator x = theWorkers.begin(); x != theWorkers.end() && x != w; x++, c++) {}
				qDebug("--------- %c ---------", c);
			}
		}

		// We set it here so that if we were a plunger (!willReadChunks) then next time
		// we dont move the queue on (to keep in sync).
		wasPlunger = (!willReadChunks);

		theQueueLock.lock();

		// If we just pushed a plunger, wait until it has been processed (i.e. limbo has been acknowledged)
		if(wasPlunger)
		{	if(MESSAGES && theDebug) qDebug("DomProcessor: Waiting for limbo to be acked (or to be stopped)...");
			while(!theLimbo)
			{	checkExitDontLock();
				theQueueChanged.wait(&theQueueLock);
			}
			if(MESSAGES && theDebug) qDebug("DomProcessor: Limbo acked!");
		}
	}
}

void DomProcessor::eater()
{
	theOwningProcessor.setLocalData(new Processor *(this));

	uint lastPri = 0;

	if(MESSAGES && theDebug) qDebug("Eater: Starting...");
	QTime clock;
	clock.start();
	bool balanceLoad = theBalanceLoad && theWorkers.count() > 1;
	uint tc = theNomChunks * theWorkers.count();
	float speeds[theWorkers.count()];
	for(uint i = 0; i < theWorkers.count(); i++) speeds[i] = 0.;
	QMutexLocker lock(&theQueueLock);
	while(1)
	{
		if(MESSAGES && theDebug) qDebug("Eater: Waiting for a new producer...");
		while(!theQueueLen)
		{	//if(MESSAGES && theDebug) qDebug("Eater: None in queue yet...");
			checkExitDontLock();
			theQueueChanged.wait(&theQueueLock);
		}
		DxCoupling *w = *theQueuePos;
		theQueueLock.unlock();
		if(MESSAGES && theDebug) qDebug("Eater: Waiting for producer %p to return results...", w);
		bool justPlunger;
		{	BufferDatas d = w->returnResults();

			if(w == thePrimaryCoupling && lastPri && balanceLoad)
			{
				int i = 0;
				for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++,i++)
					speeds[i] = speeds[i] * .95 + float((*w)->theLoad) / float((*w)->theLastTimeTaken+1.) * .05;
				if(uint(clock.elapsed()) > theBalanceInterval)
				{
					// Sort out load balancing.
					float ts = 0.;
					int i = 0;
					for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++,i++)
						ts += speeds[i] * (dynamic_cast<DSCoupling *>(*w) ? theLocalFudge : 1.);
					if(lMESSAGES && theDebug)
					{
						QString s;
						qDebug("Total chunks: %d. Total speeds (after fudge): %f", tc, ts);
						i = 0;
						for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++,i++)
							s += "   [" + QString::number((*w)->theLoad) + "/" + QString::number((*w)->theLastTimeTaken+1.) + "=" + QString::number(speeds[i]) + "]";
						qDebug("Time/load:%s", s.latin1());
						clock.start();
					}
					i = 0;
					for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++,i++)
					{	float ratio = speeds[i] * (dynamic_cast<DSCoupling *>(*w) ? theLocalFudge : 1.) / ts;
						(*w)->theLoad = uint(round(  /*round(ratio * 16.f) / 16.f*/ratio * float(tc)  ));
						if(lMESSAGES && theDebug) qDebug("Adjusting load... speed = %f, tc = %d, ratio = %f, ratio16 = %f, ts = %f, res = %d", speeds[i], tc, ratio, round(ratio * 16.f) / 16.f, ts, (*w)->theLoad);
					}
					if(theDebug)
					{
						QString s;
						for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
							s += "   " + QString::number((*w)->theLoad);
						qDebug("New loads:%s", s.latin1());
					}

					//for(int i = 0; i < theWorkers.count(); i++) speeds[i] = 0.;
					clock.start();
				}
			}
			lastPri = w->theLastTimeTaken;

/*			else if(lastPri && w->theLastTimeTaken)
			{	if(w->theLastTimeTaken > lastPri * 4 / 3 && w)
				{	// TODO: use relative increments, not just 1
					if(
					w->theLoad = min(theMaxChunks, (uint)max(1, w->theLoad - 1));
					if(MESSAGES && theDebug) qDebug("Eater: SubProc %p has been decreased to %d due to %d ms lateness (Primary=%d)", w, w->theLoad, w->theLastTimeTaken, lastPri);
				}
				else if(w->theLastTimeTaken < lastPri * 3 / 4)
				{	w->theLoad = min(theMaxChunks, (uint)max(1, w->theLoad + 1));
					if(MESSAGES && theDebug) qDebug("Eater: SubProc %p has been increased to %d due to %d ms speed (Primary=%d)", w, w->theLoad, w->theLastTimeTaken, lastPri);
				}
			}
*/
			checkExit();
			// for some reason, unless we print off the value of plunged, plunged seems to
			// be unreliable in valgrind. we can, however, assume (for now) that
			// plunged == (d.size() == 0), since we only ever send a plunger with no data.
			justPlunger = (d.size() == 0) && numOutputs();
			//if(MESSAGES) qDebug("Eater: Got results (plunger: %d)...", plu);

			if(!justPlunger)
				for(uint i = 0; i < numOutputs(); i++)
				{
					for(uint e = 0; e < d[i].elements(); e++)
					{
						if(isnan(d[i][e]) || isinf(d[i][e]))
						{
							qDebug("%s: Contains non-finite value on output %d, element %d", name().latin1(), i, e);
						}
					}
				}

			if(MESSAGES && theDebug)
			{	if(justPlunger)
					qDebug("Eater: Pushing plunger...");
				else
					qDebug("Eater: Pushing data (samples=%d)", numOutputs() ? d[0].samples() : 0);
			}
			for(uint i = 0; i < numOutputs(); i++)
				if(justPlunger)
					output(i).pushPlunger();
				else
					output(i).push(d[i]);
			if(MESSAGES && theDebug) qDebug("Eater: Results returned.");
		}
		theQueueLock.lock();

		if(!justPlunger)
		{
			if(MESSAGES && theDebug) qDebug("Eater: Removing producer %p...", w);
			theQueueLen--;
			theQueuePos++;
			if(theQueuePos == theWorkers.end())
				theQueuePos = theWorkers.begin();
			theQueueChanged.wakeAll();
		}
		else
		{	if(MESSAGES && theDebug) qDebug("Eater: Acking limbo...");
			theLimbo = true;
			theQueueChanged.wakeAll();
			if(MESSAGES && theDebug) qDebug("Eater: Waiting for limbo to be reset, so I can continue (or stopping)...");
			while(theLimbo)
			{	checkExitDontLock();
				theQueueChanged.wait(&theQueueLock);
			}
			if(MESSAGES && theDebug) qDebug("Eater: Continuing.");
		}
	}
}

void DomProcessor::addWorker(SubProcessor *worker)
{
	new DSCoupling(this, worker);
}

void DomProcessor::ratify(DxCoupling *c)
{
	if(theIsInitialised)
	{
		c->initFromProperties(theProperties);
		c->defineIO(numInputs(), numOutputs());
	}
}

void DomProcessor::EaterThread::run()
{
	try
	{
		theDomProcessor->eater();
	}
	catch(const BailException &) {}
	if(MESSAGES) qDebug("Eater: Exiting...");
}

void DomProcessor::checkExitDontLock()
{
	if(MESSAGES&&0) qDebug("DomProcessor [%p]: Checking exit (not locking)...", this);
	bool doThrow = false;
	theStop.lock();
	if(theStopping) doThrow = true;
	theStop.unlock();
	if(MESSAGES&&0) qDebug("DomProcessor [%p]: doThrow = %d.", this, doThrow);

	if(doThrow) bail();
}

void DomProcessor::checkExit()
{
	if(MESSAGES&&0) qDebug("DomProcessor [%p]: Checking exit (will lock)...", this);
	bool doThrow = false;
	theStop.lock();
	if(theStopping) doThrow = true;
	theStop.unlock();
	if(MESSAGES&&0) qDebug("DomProcessor [%p]: doThrow = %d.", this, doThrow);

	if(doThrow)
	{	theQueueLock.lock();
		bail();
	}
}

bool DomProcessor::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	// We can use just verifyAndSpecifyTypes here, since the outTypes will be recorded
	// for our primary in the for loop later anyway (assuming they're valid).
	bool ret = thePrimary->verifyAndSpecifyTypes(inTypes, outTypes);

	theSamplesIn = thePrimary->theIn;
	theSamplesStep = thePrimary->theStep;
	theSamplesOut = thePrimary->theOut;

	// Now we need a quick hack here since if we're a MultiOut, all outTypes==outTypes[0]:
	if(theMulti&Out && outTypes.count() && outTypes.populated(0))
		outTypes.setFill(outTypes.ptrAt(0), false);

	if(ret)
		for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
			(*w)->specifyTypes(inTypes, outTypes);
	return ret;
}

PropertiesInfo DomProcessor::specifyProperties() const
{
	return PropertiesInfo(thePrimary->specifyProperties())
						 ("Latency/Throughput", 0.5, "Throughput to latency optimisation weighting. Towards 0 for low latency at the cost of CPU usage and throughput, towards 1 for high throughput at the cost of memory and latency. { Value >= 0; Value <= 1 }")
						 ("Alter Buffer", true, "Change buffer size according to optimal configuration.")
						 ("Optimal Throughput", 262144, "Optimal size of buffer for maximum throughput in elements.")
						 ("Balance Load", true, "Attempt to balance load between different workers (if there is more than 1).")
						 ("Balance Interval", 10000, "Minimum time between balancing intervals in milliseconds. A larger value results in more accurate estimation but is less quick to adapt.")
						 ("Local Fudge", 0.01, "Coefficient of load for a SubProcessor being local. Above 1.0 results in the localhost being given more than estimated, below 1.0 results in it being given less then the estimate.")
						 ("Debug", false, "Debug this DomProcessor.");
}

void DomProcessor::initFromProperties(const Properties &properties)
{
	theWeighting = max(0., min(1., properties["Latency/Throughput"].toDouble()));
	theAlterBuffer = properties["Alter Buffer"].toBool();
	theOptimalThroughput = properties["Optimal Throughput"].toInt();
	theDebug = properties["Debug"].toBool();
	theBalanceLoad = properties["Balance Load"].toBool();
	theBalanceInterval = properties["Balance Interval"].toInt();
	theLocalFudge = properties["Local Fudge"].toDouble();
	for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
		(*w)->initFromProperties(properties);
	theProperties = properties;
	setupIO(thePrimary->theNumInputs, thePrimary->theNumOutputs);
	for(Q3PtrList<DxCoupling>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
		(*w)->defineIO(numInputs(), numOutputs());
}

void DomProcessor::specifyInputSpace(Q3ValueVector<uint> &samples)
{
	uint minimumSize = (theSamplesIn + theSamplesStep * (theWorkers.count() - 1));
	uint optimalSize = Undefined;
	for(uint i = 0; i < (uint)samples.count(); i++)
		if(optimalSize > max(theOptimalThroughput / input(i).type().scope(), minimumSize))
			optimalSize = max(theOptimalThroughput / input(i).type().scope(), minimumSize);
	theWantSize = uint(ceil(exp((log(double(optimalSize)) - log(double(minimumSize))) * theWeighting + log(double(minimumSize)))));
	for(uint i = 0; i < (uint)samples.count(); i++)
		samples[i] = theAlterBuffer ? theWantSize : minimumSize;

	if(theDebug && lMESSAGES) qDebug("sIS (%s): minimum=%d, workers=%d, optimal=%d, want=%d, alter=%d", name().latin1(), minimumSize, theWorkers.count(), optimalSize, theWantSize, theAlterBuffer);
}

void DomProcessor::specifyOutputSpace(Q3ValueVector<uint> &samples)
{
	for(uint i = 0; i < (uint)samples.count(); i++)
		samples[i] = theAlterBuffer ? ((theWantSize - theSamplesIn) / theSamplesStep + 1) * theSamplesOut : theSamplesOut * theWorkers.count();
}

};
