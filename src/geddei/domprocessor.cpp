/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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

#include <cmath>

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
	CoProcessor("DomProcessor", primary->theMulti),
	thePrimary(primary),
	theCurrentIns(0),
	theCurrentOuts(0)
{
	primary->thePrimaryOf = this;
}

DomProcessor::DomProcessor(const QString &primaryType):
	CoProcessor("DomProcessor", (thePrimary = SubProcessorFactory::create(primaryType))->theMulti),
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

void DomProcessor::addWorker(SubProcessor *worker)
{
	theWorkers.append(new DSCoupling(this, worker));
}

void DomProcessor::ratify(DxCoupling *c)
{
	if (theIOSetup)
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

	if (MESSAGES) qDebug("DomProcessor[%s]: OK.", qPrintable(theName));
}

void DomProcessor::haveStoppedNow()
{
	if (MESSAGES) qDebug("DomProcessor[%s]: Stopped.", qPrintable(theName));

	if (MESSAGES) qDebug("DomProcessor[%s]: Stopping workers...", qPrintable(theName));
	foreach (DxCoupling* w, theWorkers)
		w->stop();

	theCurrentIns.nullify();
	theCurrentOuts.nullify();
}

void DomProcessor::specifyInputSpace(QVector<uint> &samples)
{
	theWantChunks = theWorkers.count() + 1;

	if (theAlterBuffer)
	{
		uint minimumSamples = theWorkers.count() * theSamplesStep + theSamplesIn;
		uint optimalSamples = Undefined;
		for (uint i = 0; i < (uint)samples.count(); i++)
			optimalSamples = min(optimalSamples, max(minimumSamples, theOptimalThroughput / max(1u, input(i).type().size())));
		uint optimalChunks = (optimalSamples - theSamplesIn) / max(1u, theSamplesStep);

		// Formulate in terms of whole chunks, make sure it's divisible by the readers, recalculate how many samples.
		theWantChunks = max(theWantChunks, uint(ceil(exp((log(double(optimalChunks)) - log(double(theWantChunks))) * theWeighting + log(double(theWantChunks))))) / (theWorkers.count() + 1) * (theWorkers.count() + 1));

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
		w->go();

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
	return CanWork;
}

int DomProcessor::process()
{
	uint samples = Undefined;
	for (uint i = 0; i < numInputs(); i++)
		samples = min(samples, input(i).samplesReady());

	if (samples < theWantSamples)
	{
		// stream discontinuity.
		uint chunks = (samples - theSamplesIn) / theSamplesStep + 1;
		for (uint i = 0; i < theCurrentIns.count(); i++)
			theCurrentIns.copyData(i, input(i).peekSamples(samples));	// normally would be theWantSamples
		for (uint i = 0; i < theCurrentOuts.count(); i++)
			theCurrentOuts.copyData(i, output(i).makeScratchSamples(chunks * theSamplesOut));	// normally would be theWantChunks * theSamplesOut
		if (theWorkers.count())
			thePrimary->processChunks(theCurrentIns, theCurrentOuts, chunks);
		else
			thePrimary->processOwnChunks(theCurrentIns, theCurrentOuts, chunks);
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
			BufferDatas ins = theCurrentIns.samples(i * chunksEach * theSamplesStep, (chunksEach - 1) * theSamplesStep + theSamplesIn);
			BufferDatas outs = theCurrentOuts.samples(i * chunksEach * theSamplesOut, chunksEach * theSamplesOut);
			if (!theWorkers.count())
				thePrimary->processOwnChunks(ins, outs, chunksEach);
			else if (i == theWorkers.count())
				thePrimary->processChunks(ins, outs, chunksEach);
			else
				theWorkers[i]->processChunks(ins, outs, chunksEach);
		}
		serviceSubs();
		return DidWork;
	}
}

bool DomProcessor::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	// We can use just verifyAndSpecifyTypes here, since the outTypes will be recorded
	// for our primary in the for loop later anyway (assuming they're valid).
	bool ret = thePrimary->verifyAndSpecifyTypes(inTypes, outTypes);

	theSamplesIn = thePrimary->theIn;
	theSamplesStep = thePrimary->theStep;
	theSamplesOut = thePrimary->theOut;

	// Now we need a quick hack here since if we're a MultiOut, all outTypes==outTypes[0]:
	if ((theMulti&Out) && outTypes.count() && outTypes.populated(0))
		outTypes.fillEmpty(outTypes[0]);

	if (ret)
		foreach (DxCoupling* w, theWorkers)
			w->specifyTypes(inTypes, outTypes);
	return ret;
}

PropertiesInfo DomProcessor::specifyProperties() const
{
	PropertiesInfo ps = thePrimary->specifyProperties();
	qDebug() << ps.keys();
	ps = ps.stashed();
	qDebug() << ps.keys();
	return PropertiesInfo(ps)
						 ("Latency/Throughput", 0.5, "Throughput to latency optimisation weighting. Towards 0 for low latency at the cost of CPU usage and throughput, towards 1 for high throughput at the cost of memory and latency. { Value >= 0; Value <= 1 }")
						 ("Alter Buffer", true, "Change buffer size according to optimal configuration.")
						 ("Optimal Throughput", 262144, "Optimal size of buffer for maximum throughput in elements.")
						 ("Additional Threads", 0, "Extra threads to use for data parallelism.")
						 ("Debug", false, "Debug this DomProcessor.");
}

void DomProcessor::initFromProperties(const Properties &properties)
{
	Properties tp = properties;
	Properties wp = tp.unstash();
	theWeighting = max(0., min(1., tp["Latency/Throughput"].toDouble()));
	theAlterBuffer = tp["Alter Buffer"].toBool();
	theOptimalThroughput = tp["Optimal Throughput"].toInt();
	theDebug = tp["Debug"].toBool();
	for (int i = 0; i < tp["Additional Threads"].toInt(); i++)
		createAndAddWorker();
	thePrimary->initFromProperties(wp);
	foreach (DxCoupling* w, theWorkers)
		w->initFromProperties(wp);
	theProperties = wp;
	setupIO(thePrimary->theNumInputs, thePrimary->theNumOutputs);
	if (theIOSetup)
		onIOSetup();
}

void DomProcessor::onIOSetup()
{
	thePrimary->defineIO(numInputs(), numOutputs());
	foreach (DxCoupling* w, theWorkers)
		w->defineIO(numInputs(), numOutputs());
}

void DomProcessor::onMultiplicitySet(uint _m)
{
	Processor::onMultiplicitySet(_m);
	onIOSetup();
}

void DomProcessor::updateFromProperties(const Properties &properties)
{
	Properties tp = properties;
	Properties wp = tp.unstash();
	theDebug = tp["Debug"].toBool();

	thePrimary->updateFromProperties(wp);
	// TODO!!! DxCoupling API for updateFromProperties.
/*	foreach (DxCoupling* w, theWorkers)
		w->updateFromProperties(wp);*/
	theProperties = wp;
}

}

#undef MESSAGES
