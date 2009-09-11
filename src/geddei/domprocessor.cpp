/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <cmath>

#include <QDateTime>
#include <QList>

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

DomProcessor::DomProcessor(SubProcessor *primary):
	Processor("DomProcessor", primary->theMulti, Cooperative),
	thePrimary(primary),
	theCurrentIns(0),
	theCurrentOuts(0)
{
	primary->thePrimaryOf = this;
}

DomProcessor::DomProcessor(const QString &primaryType):
	Processor("DomProcessor", (thePrimary = SubProcessorFactory::create(primaryType))->theMulti, Cooperative),
	theCurrentIns(0),
	theCurrentOuts(0)
{
	thePrimary->thePrimaryOf = this;
}

DomProcessor::~DomProcessor()
{
	while (theWorkers.size())
		delete theWorkers.takeLast();
	assert(theWorkers.isEmpty());
}

bool DomProcessor::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	_p.fillRect(QRectF(QPointF(0, 0), _s), QBrush(Qt::black, Qt::Dense4Pattern));
	return true;
//	return thePrimary->paintProcessor(_p);
}

void DomProcessor::addWorker(SubProcessor *worker)
{
	theWorkers.append(new DSCoupling(this, worker));
}

void DomProcessor::ratify(DxCoupling *c)
{
	if (theIsInitialised)
	{
		c->initFromProperties(theProperties);
		c->defineIO(numInputs(), numOutputs());
	}
}

bool DomProcessor::createAndAddWorker()
{
	SubProcessor *sub = SubProcessorFactory::create(thePrimary->type());
	if (!sub) return false;
	addWorker(sub);
	return true;
}

bool DomProcessor::createAndAddWorker(const QString &host, uint key)
{
	return ProcessorForwarder::createCoupling(this, host, key, thePrimary->type()) != 0;
}

void DomProcessor::wantToStopNow()
{
	if (MESSAGES) qDebug("DomProcessor[%s]: Initiating stop..", qPrintable(theName));

	if (MESSAGES) qDebug("DomProcessor[%s]: %d workers.", qPrintable(theName), theWorkers.count());

	if (MESSAGES) qDebug("DomProcessor[%s]: Informing couplings...", qPrintable(theName));
	foreach (DxCoupling* w, theWorkers)
		w->stopping();
	if (MESSAGES) qDebug("DomProcessor[%s]: OK.", qPrintable(theName));
}

void DomProcessor::haveStoppedNow()
{
	if (MESSAGES) qDebug("DomProcessor[%s]: Stopped.", qPrintable(theName));

	if (MESSAGES) qDebug("DomProcessor[%s]: Closing trapdoors...", qPrintable(theName));
	foreach (DxCoupling* w, theWorkers)
		w->stopped();

	if (MESSAGES) qDebug("DomProcessor[%s]: Stopping workers...", qPrintable(theName));
	foreach (DxCoupling* w, theWorkers)
		w->stop();

	if (MESSAGES) qDebug("DomProcessor[%s]: Deleting worker readers...", qPrintable(theName));
	for (uint i = 0; i < numInputs(); i++)
		if (theInputs[i])
			foreach (DxCoupling* w, theWorkers)
				delete w->theReaders[i];
}

void DomProcessor::specifyInputSpace(QVector<uint> &samples)
{
	theWantChunks = theWorkers.count() + 1;

	if (theAlterBuffer)
	{
		uint minimumSamples = theWorkers.count() * theSamplesStep + theSamplesIn;
		uint optimalSamples = Undefined;
		for (uint i = 0; i < (uint)samples.count(); i++)
			optimalSamples = min(optimalSamples, max(minimumSamples, theOptimalThroughput / input(i).type().scope()));
		uint optimalChunks = (optimalSamples - theSamplesIn) / theSamplesStep;

		// Formulate in terms of whole chunks, make sure it's divisible by the readers, recalculate how many samples.
		theWantChunks = uint(ceil(exp((log(double(optimalChunks)) - log(double(theWantChunks))) * theWeighting + log(double(theWantChunks))))) / (theWorkers.count() + 1) * (theWorkers.count() + 1);
	}

	theWantSamples = (theWantChunks - 1) * theSamplesStep + theSamplesIn;

	for (uint i = 0; i < (uint)samples.count(); i++)
		samples[i] = theWantSamples;

	if (theDebug && lMESSAGES) qDebug("sIS (%s): workers=%d, samples=%d, chunks=%d, alter=%d", qPrintable(name()), theWorkers.count(), theWantSamples, theWantChunks, theAlterBuffer);
}

void DomProcessor::requireInputSpace(QVector<uint> &samples)
{
	uint minimumSize = theSamplesIn + theSamplesStep * theWorkers.count();
	for (uint i = 0; i < (uint)samples.count(); i++)
		samples[i] = minimumSize;
}

void DomProcessor::specifyOutputSpace(QVector<uint> &samples)
{
	for (uint i = 0; i < (uint)samples.count(); i++)
		samples[i] = theWantChunks * theSamplesOut;
}

bool DomProcessor::processorStarted()
{
	if (MESSAGES) qDebug("DomProcessor[%s]: Starting...", qPrintable(theName));

	theCurrentIns.resize(numInputs());
	theCurrentOuts.resize(numOutputs());

	// Start all sub-processors
	foreach (DxCoupling* w, theWorkers)
	{
		w->theLoad = theWantChunks / (theWorkers.count() + 1);
		w->go();
	}

	return true;
}

bool DomProcessor::serviceSubs()
{
	if (!theCurrentIns.isNull() || !theCurrentOuts.isNull())
	{
		foreach (DxCoupling* w, theWorkers)
			if (!w->isReady())
				return false;
		for (uint i = 0; i < theCurrentOuts.count(); i++)
			output(i) << theCurrentOuts[i];
		theCurrentIns.nullify();
		theCurrentOuts.nullify();
		for (uint i = 0; i < numInputs(); i++)
			input(i).readSamples(theWantChunks * theSamplesStep);
	}
	return true;
}

int DomProcessor::canProcess()
{
	if (!serviceSubs())
		return NoWork;
	return Processor::canProcess();
}

int DomProcessor::process()
{
	uint samples = Undefined;
	for (uint i = 0; i < numInputs(); i++)
		samples = min(samples, input(i).samplesReady());

	if (samples != theWantSamples)
	{
		// stream discontinuity.
		uint chunks = (samples - theSamplesIn) / theSamplesStep + 1;
		for (uint i = 0; i < theCurrentIns.count(); i++)
			theCurrentIns.copyData(i, input(i).peekSamples(samples));	// normally would be theWantSamples
		for (uint i = 0; i < theCurrentOuts.count(); i++)
			theCurrentOuts.copyData(i, output(i).makeScratchSamples(chunks * theSamplesOut));	// normally would be theWantChunks * theSamplesOut
		thePrimary->processChunks(theCurrentIns, theCurrentOuts, chunks);
		for (uint i = 0; i < theCurrentOuts.count(); i++)
			output(i) << theCurrentOuts[i];
		theCurrentIns.nullify();
		theCurrentOuts.nullify();
		for (uint i = 0; i < theCurrentIns.count(); i++)
			input(i).readSamples(samples);	// normally would be theWantSamples - theSamplesIn + theSamplesStep
		return DidWork;
	}
	else
	{
		uint chunksEach = theWantChunks / (theWorkers.count() + 1);
		for (uint i = 0; i < theCurrentIns.count(); i++)
			theCurrentIns.copyData(i, input(i).peekSamples(theWantSamples));
		for (uint i = 0; i < theCurrentOuts.count(); i++)
			theCurrentOuts.copyData(i, output(i).makeScratchSamples(theWantChunks * theSamplesOut));
		for (int i = 0; i <= theWorkers.count(); i++)
		{
			BufferDatas ins = theCurrentIns.samples(i * chunksEach * theSamplesStep, chunksEach * (theSamplesStep - 1) + theSamplesIn);
			BufferDatas outs = theCurrentOuts.samples(i * chunksEach * theSamplesOut, chunksEach * theSamplesOut);
			if (i == theWorkers.count())
				thePrimary->processChunks(ins, outs, chunksEach);
			else
				theWorkers[i]->processChunks(ins, outs, chunksEach);
		}
		serviceSubs();
		return DidWork;
	}
#if 0
	// Wait until there's room in the queue for another job, or the last iteration just pushed a plunger
	// (which means the queue may look full, but the current worker is, in fact, not busy, since it only
	// pushed a plunger).
	while (theQueueLen == (uint)theWorkers.count() && !wasPlunger)
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
	if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Waiting for a nominal (%d) or at least minimal (%d) amount of data...", qPrintable(theName), wouldReadSamples, theSamplesIn);
	(*w)->theReaders[0]->waitForElements(wouldReadSamples * (*w)->theReaders[0]->type()->scope());

	// Make sure that we don't have to exit now.
	if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Checking exit status...", qPrintable(theName));
	checkExit();

	// Find out how much we will be reading
	uint availableSamples = min(wouldReadSamples, (*w)->theReaders[0]->type()->samples((*w)->theReaders[0]->elementsReady()));
	if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Found %d samples ready for reading...", qPrintable(theName), availableSamples);

	// If we don't get to read a useful amount (i.e. less than one single chunk)
	if (availableSamples < theSamplesIn)
	{
#ifdef EDEBUG
		if ((*w)->theReaders[0]->elementsReady() % (*w)->theReaders[0]->type()->scope())
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
		if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Plunger after %d samples. Skipping samples and plunging on %p...", qPrintable(theName), availableSamples, *w);
		(*w)->skipPlungeAndSend(availableSamples);
		checkExit();
	}
	else
	{	willReadChunks = ((availableSamples - theSamplesIn) / theSamplesStep) + 1;
		willReadSamples = theSamplesStep * (willReadChunks - 1) + theSamplesIn;
		discardFromOthers = willReadChunks * theSamplesStep;

		if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Found %d samples (%d chunks) available. Reading & sending to %p...", qPrintable(theName), willReadSamples, willReadChunks, *w);
		(*w)->peekAndSend(willReadSamples, willReadChunks);
		(*w)->skip(discardFromOthers);
		checkExit();
	}

	// If this is the first time we're pushing something to this SubProc, then we want the
	// queue len to increase. It would be the first time if the last time was data.
	if (MESSAGES && theDebug) qDebug("DomProcessor: Checking wasPlunger (%d).", wasPlunger);
	theQueueLock.lock();
	if (!wasPlunger)
	{
		if (MESSAGES && theDebug) qDebug("DomProcessor: Incrementing queue length.");
		theQueueLen++;
		theQueueChanged.wakeAll();
	}

	theLimbo = false;
	theQueueChanged.wakeAll();
	theQueueLock.unlock();

	if (hMESSAGES && theDebug) (*w)->theReaders[0]->debug();
	if (MESSAGES && theDebug) qDebug("DomProcessor: Skipping %d samples on others...", discardFromOthers);
	foreach (DxCoupling* x, theWorkers)
		if (x != *w) x->skip(discardFromOthers);

	if (hMESSAGES && theDebug) (*w)->theReaders[0]->debug();

	// If we processed real data, then we want to go on.
	// If we only processed a plunger, we want to stay still.
	if (willReadChunks)
	{	w++;
		if (w == theWorkers.end()) w = theWorkers.begin();
		if (MESSAGES && theDebug)
		{	qDebug("\n---- NEXT WORKER ----");
			char c = 'A';
			for (QList<DxCoupling*>::Iterator x = theWorkers.begin(); x != theWorkers.end() && x != w; x++, c++) {}
			qDebug("--------- %c ---------", c);
		}
	}

	// We set it here so that if we were a plunger (!willReadChunks) then next time
	// we dont move the queue on (to keep in sync).
	wasPlunger = (!willReadChunks);

	guard();

	theQueueLock.lock();

	// If we just pushed a plunger, wait until it has been processed (i.e. limbo has been acknowledged)
	if (wasPlunger)
	{	if (MESSAGES && theDebug) qDebug("DomProcessor: Waiting for limbo to be acked (or to be stopped)...");
		while (!theLimbo)
		{	checkExitDontLock();
			theQueueChanged.wait(&theQueueLock);
		}
		if (MESSAGES && theDebug) qDebug("DomProcessor: Limbo acked!");
	}
#endif
}

#if 0
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
	// ~DSCoupling removes the ptr from the list itself.
	while (theWorkers.size())
		delete theWorkers.last();
	assert(theWorkers.isEmpty());
}

bool DomProcessor::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	_p.fillRect(QRectF(QPointF(0, 0), _s), QBrush(Qt::black, Qt::Dense4Pattern));
	return true;
//	return thePrimary->paintProcessor(_p);
}

bool DomProcessor::createAndAddWorker()
{
	SubProcessor *sub = SubProcessorFactory::create(thePrimary->type());
	if (!sub) return false;
	addWorker(sub);
	return true;
}

bool DomProcessor::createAndAddWorker(const QString &host, uint key)
{
	return ProcessorForwarder::createCoupling(this, host, key, thePrimary->type()) != 0;
}

void DomProcessor::wantToStopNow()
{
	if (MESSAGES) qDebug("DomProcessor[%s]: Initiating stop..", qPrintable(theName));

	if (MESSAGES) qDebug("DomProcessor[%s]: Unlocking queue...", qPrintable(theName));
	{	QMutexLocker lock(&theQueueLock);
		theQueueChanged.wakeAll();
	}

	if (MESSAGES) qDebug("DomProcessor[%s]: %d workers.", qPrintable(theName), theWorkers.count());

	if (MESSAGES) qDebug("DomProcessor[%s]: Informing couplings...", qPrintable(theName));
	foreach (DxCoupling* w, theWorkers)
		w->stopping();
	if (MESSAGES) qDebug("DomProcessor[%s]: OK.", qPrintable(theName));
}

void DomProcessor::haveStoppedNow()
{
	if (MESSAGES) qDebug("DomProcessor[%s]: Stopped.", qPrintable(theName));

	if (MESSAGES) qDebug("DomProcessor[%s]: Waiting on eater...", qPrintable(theName));
	theEaterThread.wait();

	if (MESSAGES) qDebug("DomProcessor[%s]: Closing trapdoors...", qPrintable(theName));
	foreach (DxCoupling* w, theWorkers)
		w->stopped();

	if (MESSAGES) qDebug("DomProcessor[%s]: Stopping workers...", qPrintable(theName));
	foreach (DxCoupling* w, theWorkers)
		w->stop();

	if (MESSAGES) qDebug("DomProcessor[%s]: Deleting our readers and resurecting...", qPrintable(theName));
	for (uint i = 0; i < numInputs(); i++)
		if (theInputs[i])
		{
			foreach (DxCoupling* w, theWorkers)
				delete w->theReaders[i];
			theInputs[i]->resurectReader();
		}
}

bool DomProcessor::processorStarted()
{
	if (MESSAGES) qDebug("DomProcessor[%s]: Starting...", qPrintable(theName));

	// Sort out all the buffer readers.
	for (uint i = 0; i < numInputs(); i++)
		if (theInputs[i])
			theInputs[i]->killReader();
			// In fact otherwise this would be a fatal error, but now it not the time to catch that.
			// It will be caught in confirmTypes instead.
	foreach (DxCoupling* w, theWorkers)
	{
		w->theReaders.resize(numInputs());
		for (uint i = 0; i < numInputs(); i++)
			if (theInputs[i])
				w->theReaders[i] = theInputs[i]->newReader();
	}
	if (MESSAGES) qDebug("DomProcessor[%s]: Startup done (workers: %d)", qPrintable(theName), theWorkers.count());
	return true;
}

void DomProcessor::processor()
{
	if (MESSAGES && theDebug) qDebug("DomProcessor[%s]::processor(): Starting main loop...", qPrintable(theName));

	// Figure out theNomChunks from the buffer capacities.
	uint minInCap = Undefined;
	for (uint i = 0; i < numInputs(); i++)
		if (minInCap > (*theWorkers.begin())->theReaders[i]->size() / (*theWorkers.begin())->theReaders[i]->type()->scope())
			minInCap = (*theWorkers.begin())->theReaders[i]->size() / (*theWorkers.begin())->theReaders[i]->type()->scope();
	theNomChunks = ((minInCap - theSamplesIn) / theSamplesStep + 1) / theWorkers.count();
	uint minOutCap = Undefined;
	for (uint i = 0; i < numOutputs(); i++)
		if (minOutCap > output(i).maximumScratchSamplesEver())
			minOutCap = output(i).maximumScratchSamplesEver();
	if (minOutCap != Undefined)
		theNomChunks = min(theNomChunks, minOutCap / theSamplesOut / theWorkers.count());

	// We set nom to half of the maximum possible in order to behave normally when the output
	// processor hits its worst case of having a minimum pushing chunk of size n/2+1 where n is
	// the size of the buffer. In this case it is only able to push one chunk at once so the
	// buffer could never be more than n/2+1 full at any one time; if we therefore only ever wait
	// for halve the maximum possible chunks then we should be ok.
	theNomChunks = (theNomChunks + 1 - theSamplesIn / theSamplesStep) / 2;

	if (lMESSAGES && theDebug) qDebug("DomProcessor[%s]: tNC=%d, mIC=%d, mOC=%d)...", qPrintable(theName), theNomChunks, minInCap, minOutCap);

	// Start all sub-processors
	foreach (DxCoupling* w, theWorkers)
	{
		w->theLoad = theNomChunks;
		w->go();
	}

	QMutexLocker qlock(&theQueueLock);
	theQueuePos = theWorkers.begin();
	theQueueLen = 0;
	theLimbo = false;

	if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Starting eater thread...", qPrintable(theName));
	theStopped = false;
	theEaterThread.start();

	bool wasPlunger = false;

	QList<DxCoupling*>::Iterator w = theWorkers.begin();
	while (true)
	{

		// Wait until there's room in the queue for another job, or the last iteration just pushed a plunger
		// (which means the queue may look full, but the current worker is, in fact, not busy, since it only
		// pushed a plunger).
		while (theQueueLen == (uint)theWorkers.count() && !wasPlunger)
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
		if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Waiting for a nominal (%d) or at least minimal (%d) amount of data...", qPrintable(theName), wouldReadSamples, theSamplesIn);
		(*w)->theReaders[0]->waitForElements(wouldReadSamples * (*w)->theReaders[0]->type()->scope());

		// Make sure that we don't have to exit now.
		if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Checking exit status...", qPrintable(theName));
		checkExit();

		// Find out how much we will be reading
		uint availableSamples = min(wouldReadSamples, (*w)->theReaders[0]->type()->samples((*w)->theReaders[0]->elementsReady()));
		if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Found %d samples ready for reading...", qPrintable(theName), availableSamples);

		// If we don't get to read a useful amount (i.e. less than one single chunk)
		if (availableSamples < theSamplesIn)
		{
#ifdef EDEBUG
			if ((*w)->theReaders[0]->elementsReady() % (*w)->theReaders[0]->type()->scope())
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
			if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Plunger after %d samples. Skipping samples and plunging on %p...", qPrintable(theName), availableSamples, *w);
			(*w)->skipPlungeAndSend(availableSamples);
			checkExit();
		}
		else
		{	willReadChunks = ((availableSamples - theSamplesIn) / theSamplesStep) + 1;
			willReadSamples = theSamplesStep * (willReadChunks - 1) + theSamplesIn;
			discardFromOthers = willReadChunks * theSamplesStep;

			if (MESSAGES && theDebug) qDebug("DomProcessor[%s]: Found %d samples (%d chunks) available. Reading & sending to %p...", qPrintable(theName), willReadSamples, willReadChunks, *w);
			(*w)->peekAndSend(willReadSamples, willReadChunks);
			(*w)->skip(discardFromOthers);
			checkExit();
		}

		// If this is the first time we're pushing something to this SubProc, then we want the
		// queue len to increase. It would be the first time if the last time was data.
		if (MESSAGES && theDebug) qDebug("DomProcessor: Checking wasPlunger (%d).", wasPlunger);
		theQueueLock.lock();
		if (!wasPlunger)
		{
			if (MESSAGES && theDebug) qDebug("DomProcessor: Incrementing queue length.");
			theQueueLen++;
			theQueueChanged.wakeAll();
		}
		theLimbo = false;
		theQueueChanged.wakeAll();
		theQueueLock.unlock();

		if (hMESSAGES && theDebug) (*w)->theReaders[0]->debug();
		if (MESSAGES && theDebug) qDebug("DomProcessor: Skipping %d samples on others...", discardFromOthers);
		foreach (DxCoupling* x, theWorkers)
			if (x != *w) x->skip(discardFromOthers);

		if (hMESSAGES && theDebug) (*w)->theReaders[0]->debug();

		// If we processed real data, then we want to go on.
		// If we only processed a plunger, we want to stay still.
		if (willReadChunks)
		{	w++;
			if (w == theWorkers.end()) w = theWorkers.begin();
			if (MESSAGES && theDebug)
			{	qDebug("\n---- NEXT WORKER ----");
				char c = 'A';
				for (QList<DxCoupling*>::Iterator x = theWorkers.begin(); x != theWorkers.end() && x != w; x++, c++) {}
				qDebug("--------- %c ---------", c);
			}
		}

		// We set it here so that if we were a plunger (!willReadChunks) then next time
		// we dont move the queue on (to keep in sync).
		wasPlunger = (!willReadChunks);

		guard();

		theQueueLock.lock();

		// If we just pushed a plunger, wait until it has been processed (i.e. limbo has been acknowledged)
		if (wasPlunger)
		{	if (MESSAGES && theDebug) qDebug("DomProcessor: Waiting for limbo to be acked (or to be stopped)...");
			while (!theLimbo)
			{	checkExitDontLock();
				theQueueChanged.wait(&theQueueLock);
			}
			if (MESSAGES && theDebug) qDebug("DomProcessor: Limbo acked!");
		}
	}
}

void DomProcessor::eater()
{
	setThreadProcessor();

	uint lastPri = 0;

	if (MESSAGES && theDebug) qDebug("Eater: Starting...");
	QTime clock;
	clock.start();
	bool balanceLoad = theBalanceLoad && theWorkers.count() > 1;
	uint tc = theNomChunks * theWorkers.count();
	float speeds[theWorkers.count()];
	for (uint i = 0; i < (uint)theWorkers.count(); i++) speeds[i] = 0.;
	QMutexLocker lock(&theQueueLock);
	while (1)
	{
		if (MESSAGES && theDebug) qDebug("Eater: Waiting for a new producer...");
		while (!theQueueLen)
		{	//if (MESSAGES && theDebug) qDebug("Eater: None in queue yet...");
			checkExitDontLock();
			theQueueChanged.wait(&theQueueLock);
		}
		DxCoupling *w = *theQueuePos;
		theQueueLock.unlock();
		if (MESSAGES && theDebug) qDebug("Eater: Waiting for producer %p to return results...", w);
		bool justPlunger;
		{	BufferDatas d = w->returnResults();

			if (w == thePrimaryCoupling && lastPri && balanceLoad)
			{
				int i = 0;
				for (QList<DxCoupling*>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++,i++)
					speeds[i] = speeds[i] * .95 + float((*w)->theLoad) / float((*w)->theLastTimeTaken+1.) * .05;
				if (uint(clock.elapsed()) > theBalanceInterval)
				{
					// Sort out load balancing.
					float ts = 0.;
					int i = 0;
					for (QList<DxCoupling*>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++,i++)
						ts += speeds[i] * (dynamic_cast<DSCoupling*>(*w) ? theLocalFudge : 1.);
					if (lMESSAGES && theDebug)
					{
						QString s;
						qDebug("Total chunks: %d. Total speeds (after fudge): %f", tc, ts);
						i = 0;
						for (QList<DxCoupling*>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++,i++)
							s += "   [" + QString::number((*w)->theLoad) + "/" + QString::number((*w)->theLastTimeTaken+1.) + "=" + QString::number(speeds[i]) + "]";
						qDebug("Time/load:%s", qPrintable(s));
						clock.start();
					}
					i = 0;
					for (QList<DxCoupling*>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++,i++)
					{	float ratio = speeds[i] * (dynamic_cast<DSCoupling *>(*w) ? theLocalFudge : 1.) / ts;
						(*w)->theLoad = uint(round(  /*round(ratio * 16.f) / 16.f*/ratio * float(tc)  ));
						if (lMESSAGES && theDebug) qDebug("Adjusting load... speed = %f, tc = %d, ratio = %f, ratio16 = %f, ts = %f, res = %d", speeds[i], tc, ratio, round(ratio * 16.f) / 16.f, ts, (*w)->theLoad);
					}
					if (theDebug)
					{
						QString s;
						for (QList<DxCoupling*>::Iterator w = theWorkers.begin(); w != theWorkers.end(); w++)
							s += "   " + QString::number((*w)->theLoad);
						qDebug("New loads:%s", qPrintable(s));
					}

					//for (int i = 0; i < theWorkers.count(); i++) speeds[i] = 0.;
					clock.start();
				}
			}
			lastPri = w->theLastTimeTaken;

/*			else if (lastPri && w->theLastTimeTaken)
			{	if (w->theLastTimeTaken > lastPri * 4 / 3 && w)
				{	// TODO: use relative increments, not just 1
					if (
					w->theLoad = min(theMaxChunks, (uint)max(1, w->theLoad - 1));
					if (MESSAGES && theDebug) qDebug("Eater: SubProc %p has been decreased to %d due to %d ms lateness (Primary=%d)", w, w->theLoad, w->theLastTimeTaken, lastPri);
				}
				else if (w->theLastTimeTaken < lastPri * 3 / 4)
				{	w->theLoad = min(theMaxChunks, (uint)max(1, w->theLoad + 1));
					if (MESSAGES && theDebug) qDebug("Eater: SubProc %p has been increased to %d due to %d ms speed (Primary=%d)", w, w->theLoad, w->theLastTimeTaken, lastPri);
				}
			}
*/
			checkExit();
			// for some reason, unless we print off the value of plunged, plunged seems to
			// be unreliable in valgrind. we can, however, assume (for now) that
			// plunged == (d.size() == 0), since we only ever send a plunger with no data.
			justPlunger = (d.size() == 0) && numOutputs();
			//if (MESSAGES) qDebug("Eater: Got results (plunger: %d)...", plu);

			if (!justPlunger)
				for (uint i = 0; i < numOutputs(); i++)
				{
					for (uint e = 0; e < d[i].elements(); e++)
					{
						if (isnan(d[i][e]) || isinf(d[i][e]))
						{
							qDebug("%s: Contains non-finite value on output %d, element %d", qPrintable(name()), i, e);
						}
					}
				}

			if (MESSAGES && theDebug)
			{	if (justPlunger)
					qDebug("Eater: Pushing plunger...");
				else
					qDebug("Eater: Pushing data (samples=%d)", numOutputs() ? d[0].samples() : 0);
			}
			for (uint i = 0; i < numOutputs(); i++)
				if (justPlunger)
					output(i).pushPlunger();
				else
					output(i).push(d[i]);
			if (MESSAGES && theDebug) qDebug("Eater: Results returned.");
		}
		theQueueLock.lock();

		if (!justPlunger)
		{
			if (MESSAGES && theDebug) qDebug("Eater: Removing producer %p...", w);
			theQueueLen--;
			theQueuePos++;
			if (theQueuePos == theWorkers.end())
				theQueuePos = theWorkers.begin();
			theQueueChanged.wakeAll();
		}
		else
		{	if (MESSAGES && theDebug) qDebug("Eater: Acking limbo...");
			theLimbo = true;
			theQueueChanged.wakeAll();
			if (MESSAGES && theDebug) qDebug("Eater: Waiting for limbo to be reset, so I can continue (or stopping)...");
			while (theLimbo)
			{	checkExitDontLock();
				theQueueChanged.wait(&theQueueLock);
			}
			if (MESSAGES && theDebug) qDebug("Eater: Continuing.");
		}
	}
}

void DomProcessor::addWorker(SubProcessor *worker)
{
	new DSCoupling(this, worker);
}

void DomProcessor::ratify(DxCoupling *c)
{
	if (theIsInitialised)
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
	if (MESSAGES) qDebug("Eater: Exiting...");
}

void DomProcessor::checkExitDontLock()
{
	if (MESSAGES&&0) qDebug("DomProcessor [%p]: Checking exit (not locking)...", this);
	bool doThrow = false;
	theStop.lock();
	if (theStopping) doThrow = true;
	theStop.unlock();
	if (MESSAGES&&0) qDebug("DomProcessor [%p]: doThrow = %d.", this, doThrow);

	if (doThrow) bail();
}

void DomProcessor::checkExit()
{
	if (MESSAGES&&0) qDebug("DomProcessor [%p]: Checking exit (will lock)...", this);
	bool doThrow = false;
	theStop.lock();
	if (theStopping) doThrow = true;
	theStop.unlock();
	if (MESSAGES&&0) qDebug("DomProcessor [%p]: doThrow = %d.", this, doThrow);

	if (doThrow)
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
	if (theMulti&Out && outTypes.count() && outTypes.populated(0))
		outTypes.setFill(outTypes.ptrAt(0), false);

	if (ret)
		foreach (DxCoupling* w, theWorkers)
			w->specifyTypes(inTypes, outTypes);
	return ret;
}

PropertiesInfo DomProcessor::specifyProperties() const
{
	return PropertiesInfo(thePrimary->specifyProperties().stashed())
						 ("Latency/Throughput", 0.5, "Throughput to latency optimisation weighting. Towards 0 for low latency at the cost of CPU usage and throughput, towards 1 for high throughput at the cost of memory and latency. { Value >= 0; Value <= 1 }")
						 ("Alter Buffer", true, "Change buffer size according to optimal configuration.")
						 ("Optimal Throughput", 262144, "Optimal size of buffer for maximum throughput in elements.")
						 ("Balance Load", true, "Attempt to balance load between different workers (if there are more than 1).")
						 ("Balance Interval", 10000, "Minimum time between balancing intervals in milliseconds. A larger value results in more accurate estimation but is less quick to adapt.")
						 ("Local Fudge", 0.01, "Coefficient of load for a SubProcessor being local. Above 1.0 results in the localhost being given more than estimated, below 1.0 results in it being given less then the estimate.")
						 ("Debug", false, "Debug this DomProcessor.");
}

void DomProcessor::initFromProperties(const Properties &properties)
{
	Properties tp = properties;
	Properties wp = tp.unstashed();
	theWeighting = max(0., min(1., tp["Latency/Throughput"].toDouble()));
	theAlterBuffer = tp["Alter Buffer"].toBool();
	theOptimalThroughput = tp["Optimal Throughput"].toInt();
	theDebug = tp["Debug"].toBool();
	theBalanceLoad = tp["Balance Load"].toBool();
	theBalanceInterval = tp["Balance Interval"].toInt();
	theLocalFudge = tp["Local Fudge"].toDouble();
	foreach (DxCoupling* w, theWorkers)
		w->initFromProperties(wp);
	theProperties = wp;
	setupIO(thePrimary->theNumInputs, thePrimary->theNumOutputs);
	foreach (DxCoupling* w, theWorkers)
		w->defineIO(numInputs(), numOutputs());
}

void DomProcessor::specifyInputSpace(QVector<uint> &samples)
{
	uint minimumSize = (theSamplesIn + theSamplesStep * (theWorkers.count() - 1));
	uint optimalSize = Undefined;
	for (uint i = 0; i < (uint)samples.count(); i++)
		if (optimalSize > max(theOptimalThroughput / input(i).type().scope(), minimumSize))
			optimalSize = max(theOptimalThroughput / input(i).type().scope(), minimumSize);
	theWantSamples = uint(ceil(exp((log(double(optimalSize)) - log(double(minimumSize))) * theWeighting + log(double(minimumSize)))));
	for (uint i = 0; i < (uint)samples.count(); i++)
		samples[i] = theAlterBuffer ? theWantSamples : minimumSize;

	if (theDebug && lMESSAGES) qDebug("sIS (%s): minimum=%d, workers=%d, optimal=%d, want=%d, alter=%d", qPrintable(name()), minimumSize, theWorkers.count(), optimalSize, theWantSamples, theAlterBuffer);
}

void DomProcessor::specifyOutputSpace(QVector<uint> &samples)
{
	for (uint i = 0; i < (uint)samples.count(); i++)
		samples[i] = theAlterBuffer ? ((theWantSamples - theSamplesIn) / theSamplesStep + 1) * theSamplesOut : theSamplesOut * theWorkers.count();
}
#endif
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
	if (theMulti&Out && outTypes.count() && outTypes.populated(0))
		outTypes.setFill(outTypes.ptrAt(0), false);

	if (ret)
		foreach (DxCoupling* w, theWorkers)
			w->specifyTypes(inTypes, outTypes);
	return ret;
}

PropertiesInfo DomProcessor::specifyProperties() const
{
	return PropertiesInfo(thePrimary->specifyProperties().stashed())
						 ("Latency/Throughput", 0.5, "Throughput to latency optimisation weighting. Towards 0 for low latency at the cost of CPU usage and throughput, towards 1 for high throughput at the cost of memory and latency. { Value >= 0; Value <= 1 }")
						 ("Alter Buffer", true, "Change buffer size according to optimal configuration.")
						 ("Optimal Throughput", 262144, "Optimal size of buffer for maximum throughput in elements.")
						 ("Debug", false, "Debug this DomProcessor.");
}

void DomProcessor::initFromProperties(const Properties &properties)
{
	Properties tp = properties;
	Properties wp = tp.unstashed();
	theWeighting = max(0., min(1., tp["Latency/Throughput"].toDouble()));
	theAlterBuffer = tp["Alter Buffer"].toBool();
	theOptimalThroughput = tp["Optimal Throughput"].toInt();
	theDebug = tp["Debug"].toBool();
	thePrimary->initFromProperties(wp);
	foreach (DxCoupling* w, theWorkers)
		w->initFromProperties(wp);
	theProperties = wp;
	setupIO(thePrimary->theNumInputs, thePrimary->theNumOutputs);
	thePrimary->defineIO(numInputs(), numOutputs());
	foreach (DxCoupling* w, theWorkers)
		w->defineIO(numInputs(), numOutputs());
}

#undef MESSAGES
