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

#define MESSAGES 1
#define pMESSAGES 0

namespace Geddei
{

QThreadStorage<Processor **> Processor::theOwningProcessor;

Processor::Processor(const QString &type, MultiplicityType multi): theName(""), theType(type),
	theWidth(24), theHeight(12), theMinWidth(24), theMinHeight(12), m_isResizable(false), theIOSetup(false), theStopping(false), theIsInitialised(false), theAllDone(false),
	theTypesConfirmed(false), theError(NotStarted), theErrorData(0), m_inputSpace(1), m_outputSpace(1), theMulti(multi), theHardMultiplicity(Undefined), thePlungersStarted(false), thePlungersEnded(false)
{
}

Processor::~Processor()
{
	if (MESSAGES) qDebug("Deleting Processor...");
	// Stop the threads now, as we don't want the threadCleanup() executing after
	// some of the class is deallocated!
	if (isRunning()) stop();
	for (uint i = 0; i < (uint)theOutputs.size(); i++)
		if (theOutputs[i])
			delete theOutputs[i];
	// TODO: THIS MUST BE MUTUALLY EXCLUSIVE TO ANYTHING ACCESSING IT REMOTELY
	// (I.E. THROUGH GROUP, FOR THE ENTIRE REMOTE ACTION) --- LOCK IN GROUP?
	for (uint i = 0; i < (uint)theInputs.size(); i++)
		if (theInputs[i])
			delete theInputs[i];
	// FINISH MUTUAL EXCLUSIVITY

	if (MESSAGES) qDebug("Deleted Processor.");
}

double Processor::secondsPassed() const
{
	double m = 0.0;
	for (uint i = 0; i < (uint)theInputs.size(); i++)
		if (theInputs[i])
			m = max(m, theInputs[i]->secondsPassed());
	return m;
}

double Processor::secondsPassed(float _s, uint _i) const
{
	if ((int)_i < theInputs.size() && theInputs[_i])
		return theInputs[_i]->secondsPassed(_s);
	return 0.0;
}

void Processor::startPlungers()
{
	if (pMESSAGES) qDebug("> Processor::startPlungers() [%s]: %d finished", qPrintable(name()), thePlungersEnded);
	QFastMutexLocker lock(&thePlungerSystem);
	if (!thePlungersStarted)
	{
		thePlungersStarted = true;
		thePlungersEnded = false;
		for (uint index = 0; index < numInputs(); index++)
		{	thePlungersNotified[index] = 0;
			thePlungersLeft[index] = 1;
		}
		unpause();
		for (uint i = 0; i < (uint)theOutputs.count(); i++)
			theOutputs[i]->startPlungers();
	}
	if (pMESSAGES) qDebug("< Processor::startPlungers() [%s]: %d finished", qPrintable(name()), thePlungersEnded);
}

void Processor::noMorePlungers()
{
	if (pMESSAGES) qDebug("> Processor::noMorePlungers() [%s]: %d finished", qPrintable(name()), thePlungersEnded);
	QFastMutexLocker lock(&thePlungerSystem);
	if (!thePlungersEnded)
	{
		thePlungersEnded = true;
		unpause();
		// Pass it on...
		for (uint i = 0; i < numOutputs(); i++)
			theOutputs[i]->noMorePlungers();
	}
	if (pMESSAGES) qDebug("< Processor::noMorePlungers() [%s]: %d finished", qPrintable(name()), thePlungersEnded);
}

void Processor::plungerSent(uint index)
{
	if (pMESSAGES) qDebug("> Processor::plungerSent() [%s]: %d left, %d finished", qPrintable(name()), thePlungersLeft[index], thePlungersEnded);
	QFastMutexLocker lock(&thePlungerSystem);

	thePlungersLeft[index]++;
	thePlungersNotified[index]++;
	unpause();
	//Figure out if we have already propogated notification for the thePlungersNotified[index]'th plunger.
	bool alreadyHadIt = false;
	for (uint i = 0; i < numInputs() && !alreadyHadIt; i++)
		if (thePlungersNotified[i] >= thePlungersNotified[index] && i != index)
			// Another input got here before us: Ignore the plunger.
			alreadyHadIt = true;
	if (!alreadyHadIt)
		for (uint i = 0; i < numOutputs(); i++)
			theOutputs[i]->plungerSent();
	if (pMESSAGES) qDebug("< Processor::plungerSent() [%s]: [aHI: %d] %d left, %d notified, %d finished", qPrintable(name()), alreadyHadIt, thePlungersLeft[index], thePlungersNotified[index], thePlungersEnded);
}

void Processor::disconnectAll()
{
	if (isRunning())
	{	qWarning("*** WARNING: Processor::disconnect: Disconnecting input on a running processor.\n"
				 "             Stopping first.");
		stop();
	}
	for (uint i = 0; i < (uint)theOutputs.count(); i++)
		delete theOutputs[i];
}

void Processor::specifyInputSpace(QVector<uint> &samples)
{
	for (uint i = 0; i < (uint)theInputs.size(); i++)
		samples[i] = m_inputSpace;
}

void Processor::specifyOutputSpace(QVector<uint> &samples)
{
	for (uint i = 0; i < (uint)theOutputs.size(); i++)
		samples[i] = m_outputSpace;
}

void Processor::dropInput(uint index)
{
	if (isRunning())
	{	qWarning("*** WARNING: Dropping input on a running processor. Stopping first.");
		stop();
	}
	assert(index < (uint)theInputs.size());
	if (!theInputs[index])
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
	if (MESSAGES) qDebug("Registering output link on %s.", qPrintable(name()));
	assert(port < (uint)theOutputs.size());
	assert(theOutputs[port] == 0);
	theOutputs[port] = me;
}

void Processor::undoRegisterOut(LxConnection *me, uint port)
{
	if (MESSAGES) qDebug("Unregistering output link on %s.", qPrintable(name()));
	assert(port < (uint)theOutputs.size());
	assert(theOutputs[port] == me);
	theOutputs[port] = 0L;
	if (MESSAGES) qDebug("Done.");
}

void Processor::doRegisterIn(xLConnection *me, uint port)
{
	if (MESSAGES) qDebug("Registering input link on %s.", qPrintable(name()));
	assert(port < (uint)theInputs.size());
	assert(theInputs[port] == 0);
	theInputs[port] = me;
}

void Processor::undoRegisterIn(xLConnection *me, uint port)
{
	if (MESSAGES) qDebug("Unregistering input link on %s.", qPrintable(name()));
	assert(port < (uint)theInputs.size());
	assert(theInputs[port] == me);
	theInputs[port] = 0L;
	if (MESSAGES) qDebug("Done.");
}

Processor *Processor::threadProcessor()
{
	if (!theOwningProcessor.localData())
		return 0;
	return *(theOwningProcessor.localData());
}

void Processor::setThreadProcessor()
{
	if (!theOwningProcessor.localData())
		theOwningProcessor.setLocalData(new Processor*);
	*(theOwningProcessor.localData()) = this;
}

void Processor::unsetThreadProcessor()
{
	if (!theOwningProcessor.localData())
		theOwningProcessor.setLocalData(new Processor*);
	*(theOwningProcessor.localData()) = 0;
}

void Processor::checkExit()
{
	if (MESSAGES&&0) qDebug("Processor [%p]: Checking exit...", this);
	bool doThrow = false;
	theStop.lock();
	if (theStopping) doThrow = true;
	theStop.unlock();
	if (MESSAGES&&0) qDebug("Processor [%p]: doThrow = %d.", this, doThrow);

	if (doThrow) bail();
}

void Processor::plunged(uint index)
{
	if (MESSAGES) qDebug("> Processor::plunged() [%s:%d] thePlungedInputs=%d", qPrintable(name()), index, thePlungedInputs[index]);
	thePlungedInputs[index]++;
	{	QFastMutexLocker lock(&thePlungerSystem);
		thePlungersLeft[index]--;
	}
	if (MESSAGES) qDebug("= Processor::plunged() [%s:%d]: %d left, %d finished, %d total", qPrintable(name()), index, thePlungersLeft[index], thePlungersEnded, thePlungedInputs[index]);

	// If we're not the first to get this far, then exit now. We only give a plunger for the "full-speed" input.
	for (uint i = 0; i < (uint)thePlungedInputs.count(); i++)
		if (thePlungedInputs[i] >= thePlungedInputs[index] && index != i) return;
	// Otherwise this must be the first to get to this plunger: We carry on with the plunge operation.

	// We don't want to notify the user if the plunger is the one that Geddei puts at the
	// end to flush everything out.
	if (thePlungersLeft[index] || !thePlungersEnded)
		receivedPlunger();
	for (uint i = 0; i < (uint)theOutputs.size(); i++)
		theOutputs[i]->pushPlunger();
}

void Processor::resetTypes()
{
	theConfirming.lock();
	theTypesConfirmed = false;
	theConfirming.unlock();
}

bool HeavyProcessor::guard()
{
	thePause.lock();
	while (thePaused)
		theUnpaused.wait(&thePause);
	thePause.unlock();
	checkExit();
	theGuardsCrossed++;
	return true;
}

bool HeavyProcessor::thereIsInputForProcessing(uint samples)
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
	for (uint i = 0; i < numInputs(); i++)
		dataReady[i] = false;

	// Keep looping here until we reach an acceptable state
	while (1)
	{
		if (pMESSAGES) qDebug("Processor [%s]: Plunging as much as possible (waiting for %d samples)...", qPrintable(name()), samples);
		// Make any necessary updates...
		//TODO: this state could be cached to potentially prevent plungeSync call
		for (uint i = 0; i < numInputs(); i++)
			while (!dataReady[i] && thePlungersLeft[i] > 0)
				if (theInputs[i]->plungeSync(samples))
					dataReady[i] = true;

		// Could something change here?
		// thePlungersEnded could change...
		// Doesn't matter - if that changes, we note it later anyway, after lock...
		// thePlungersLeft could change... it could increment only...
		// what if it did?

		// Check, exit if necessary...
		{
			if (MESSAGES) qDebug("Processor: Locking plunger system...");
			// Lock the system for now...
			QFastMutexLocker lock(&thePlungerSystem);

			if (MESSAGES) qDebug("Processor: Checking status...");
			uint votesToEnd = 0;
			uint votesToContinue = 0;
			for (uint i = 0; i < numInputs(); i++)
			{
				if (pMESSAGES) qDebug("Processor [%s]: Input %d (DR: %d)...", qPrintable(name()), i, dataReady[i]);
				// If there's a plunger left to be gotten and we have to process stuff before we can get it then exit.
				// FIXME: check and maybe make safe plungeSync with thePlungerSystem locked.
				if (dataReady[i]) votesToContinue++;
				if (!thePlungersLeft[i] && thePlungersEnded) votesToEnd++;
			}
			if (pMESSAGES) qDebug("Processor [%s]: %d votes to end...", qPrintable(name()), votesToEnd);
			if (theInputs.count() && votesToEnd == numInputs()) return false;
			if (votesToContinue == numInputs()) return true;

			if (MESSAGES) qDebug("Processor: Waiting for something to change...");
			// Wait until something changes...
			pause();
			thePlungerSystem.unlock();
			guard();
			thePlungerSystem.lock();
		}
	}
}

bool HeavyProcessor::thereIsInputForProcessing()
{
	QVector<uint> sready(numInputs());
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
	for (uint i = 0; i < numInputs(); i++)
		dataReady[i] = false;

	// Keep looping here until we reach an acceptable state
	while (1)
	{
		if (MESSAGES) qDebug("Processor: Plunging as much as possible...");
		// Make any necessary updates...
		//TODO: this state could be cached to potentially prevent plungeSync call
		for (uint i = 0; i < numInputs(); i++)
			while (!dataReady[i] && thePlungersLeft[i] > 0)
				if (theInputs[i]->plungeSync(sready[i]))
					dataReady[i] = true;

		// Could something change here?
		// thePlungersEnded could change...
		// Doesn't matter - if that changes, we note it later anyway, after lock...
		// thePlungersLeft could change... it could increment only...
		// what if it did?

		// Check, exit if necessary...
		{
			if (MESSAGES) qDebug("Processor: Locking plunger system...");
			// Lock the system for now...
			QFastMutexLocker lock(&thePlungerSystem);

			if (MESSAGES) qDebug("Processor: Checking status...");
			uint votesToEnd = 0;
			uint votesToContinue = 0;
			for (uint i = 0; i < numInputs(); i++)
			{
				if (MESSAGES) qDebug("Processor: Input %d...", i);
				// If there's a plunger left to be gotten and we have to process stuff before we can get it then exit.
				// FIXME: check and maybe make safe plungeSync with thePlungerSystem locked.
				if (dataReady[i]) votesToContinue++;
				if (!thePlungersLeft[i] && thePlungersEnded) votesToEnd++;
			}
			if (MESSAGES) qDebug("Processor: %d votes to end...", votesToEnd);
			if (theInputs.count() && votesToEnd == numInputs())
				return false;
			if (votesToContinue == numInputs()) return true;

			if (MESSAGES) qDebug("Processor: Waiting for something to change...");
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
	if (pMESSAGES) qDebug("= Processor::plunge() [%s]", qPrintable(name()));
	if (numInputs())
	{	qWarning("*** CRITICAL: Non-source Processors may not introduce plungers!");
		return;
	}
	for (uint i = 0; i < (uint)theOutputs.count(); i++)
		theOutputs[i]->plungerSent();
	for (uint i = 0; i < (uint)theOutputs.count(); i++)
		theOutputs[i]->pushPlunger();
}

void Processor::doInit(const QString &name, ProcessorGroup *g, const Properties &properties)
{
	if (MESSAGES) qDebug("Processor::init()");

	assert (!theIsInitialised);

	if (MESSAGES) qDebug("Initialising (M=%d)...", properties.keys().contains("Multiplicity") ? properties["Multiplicity"].toInt() : 0);

	if (MESSAGES) for (uint i = 0; i < (uint)properties.keys().count(); i++) qDebug("properties[%s] = %s", qPrintable(properties.keys()[i]), qPrintable(properties[properties.keys()[i]].toString()));
	Properties p = specifyProperties();
	if (MESSAGES) for (uint i = 0; i < (uint)p.keys().count(); i++) qDebug("p[%s] = %s", qPrintable(p.keys()[i]), qPrintable(p[p.keys()[i]].toString()));
	p.set(properties);
	if (MESSAGES) for (uint i = 0; i < (uint)p.keys().count(); i++) qDebug("p[%s] = %s", qPrintable(p.keys()[i]), qPrintable(p[p.keys()[i]].toString()));
	theHardMultiplicity = properties.keys().contains("Multiplicity") ? p["Multiplicity"].toInt() : 0;
	if (!theHardMultiplicity) theHardMultiplicity = Undefined;
	theGivenMultiplicity = theHardMultiplicity;

	theName = name;
	setGroup(*g);
	initFromProperties(p);
	updateFromProperties(p);
	theIsInitialised = true;
}

void Processor::onMultiplicitySet(uint _m)
{
	theGivenMultiplicity = _m;
	if (!theIOSetup)
		setupIO((theMulti&In) && !(theMulti&Const) ? Undefined : numInputs(), (theMulti&Out) && !(theMulti&Const) ? Undefined : numOutputs());
}

const PropertiesInfo Processor::properties() const
{
	if (theMulti && !(theMulti&Const))
		return specifyProperties()("Multiplicity", 0, "Force the multiplicity of the object to be a value [> 0 to force].");
	else
		return specifyProperties();
}

bool Processor::draw(QPainter& _p, QSizeF const& _s) const
{
	return paintProcessor(_p, _s);
}

bool Processor::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	QRectF area(QPointF(0, 0), _s);

	_p.setFont(QFont("Helvetica", min(_s.width(), _s.height()), QFont::Black, false));

	_p.setPen(QColor(0, 0, 0, 192));
	_p.drawText(area.translated(0, 1), Qt::AlignCenter, simpleText());

	QLinearGradient g2(0, (_s.height() - min(_s.width(), _s.height())) / 2, 0, min(_s.width(), _s.height()));
	g2.setColorAt(.35f, Qt::white);
	g2.setColorAt(.65f, Qt::lightGray);
	_p.setPen(QPen(QBrush(g2), 1));
	_p.drawText(area, Qt::AlignCenter, simpleText());

	return true;
}

void Processor::setupVisual(uint width, uint height, uint redrawPeriod, uint minWidth, uint minHeight, bool _isResizable)
{
	theWidth = width;
	theHeight = height;
	theRedrawPeriod = redrawPeriod;
	theMinWidth = minWidth;
	theMinHeight = minHeight;
	m_isResizable = _isResizable;
}

bool Processor::go()
{
	if (MESSAGES) qDebug("> Processor::go() (name=%s)", qPrintable(name()));
	if (!theIsInitialised || !theIOSetup)
	{
		if (MESSAGES) qDebug("= Processors::go() (name=%s) Not initialised!", qPrintable(name()));
		QFastMutexLocker lock(&theErrorSystem);
		theError = NotInitialised;
		theErrorWritten.wakeAll();
		if (MESSAGES) qDebug("< Processors::go() (name=%s)", qPrintable(name()));
		return false;
	}
	if (!theTypesConfirmed)
	{
		if (MESSAGES) qDebug("= Processors::go() (name=%s) Types not confirmed!", qPrintable(name()));
		QFastMutexLocker lock(&theErrorSystem);
		theError = TypesNotConfirmed;
		theErrorWritten.wakeAll();
		if (MESSAGES) qDebug("< Processors::go() (name=%s)", qPrintable(name()));
		return false;
	}
	assert(!isRunning());

	theErrorSystem.lock();
	theError = Pending;
	theErrorSystem.unlock();

	theCustomError = QString::null;
	if (!(processorStarted() && processorStarted(theCustomError)))
	{
		if (MESSAGES) qDebug("= Processors::go() (name=%s) Unable to start!", qPrintable(name()));
		QFastMutexLocker lock(&theErrorSystem);
		theError = Custom;
		theErrorWritten.wakeAll();
		if (MESSAGES) qDebug("< Processors::go() (name=%s)", qPrintable(name()));
		return false;
	}
	theAllDone = false;

	start();

	if (MESSAGES) qDebug("< Processor::go() (name=%s)", qPrintable(name()));
	return true;
}

void Processor::stop()
{
	if (MESSAGES) qDebug("> Processor::stop() (name=%s)", qPrintable(name()));

	// This little mechanism is just in case two stop()s are called simultaneously.
	bool justExit = false;
	{	QFastMutexLocker lock(&theStop);
		if (!isRunning()) return;
		if (theStopping)
			justExit = true;
		else
			theStopping = true;
	}
	if (justExit) { wait(); return; }

	// NOTE: We cannot assert(isRunning()) here since it is possible that the process exited
	// when theStopping was set to true and doesn't require all the trapdoor stuff. It
	// doesn't make our next actions wrong, just redundant.

	getReadyForStopping();

	for (uint i = 0; i < (uint)theInputs.size(); i++)
		if (theInputs[i])
			theInputs[i]->sinkStopping();
	for (uint i = 0; i < (uint)theOutputs.size(); i++)
		if (theOutputs[i])
			theOutputs[i]->sourceStopping();
	if (MESSAGES) qDebug("= Processor::stop(): OK. Registering wait to stop...");
	wantToStopNow();
	if (MESSAGES) qDebug("= Processor::stop(): wait()");
	wait();
	if (MESSAGES) qDebug("= Processor::stop(): OK. Registering stopped...");
	haveStoppedNow();
	if (MESSAGES) qDebug("= Processor::stop(): Closing inputs.");
	for (uint i = 0; i < (uint)theInputs.size(); i++)
		if (theInputs[i])
			theInputs[i]->sinkStopped();
	if (MESSAGES) qDebug("= Processor::stop(): Closing outputs.");
	for (uint i = 0; i < (uint)theOutputs.size(); i++)
		if (theOutputs[i])
			theOutputs[i]->sourceStopped();

	theStop.lock();
	theStopping = false;
	theStop.unlock();

	if (MESSAGES) qDebug("< Processor::stop()");
}

bool Processor::confirmTypes()
{
	QFastMutexLocker lock(&theConfirming);

	if (theTypesConfirmed)
	{
		// If we're already running, best not to clear the buffer...
		{	QFastMutexLocker lock(&theErrorSystem);
			if (!theError) return true;
		}
		// refresh outputs in case of a reconnection
		assert(theTypesCache.count() == (uint)theOutputs.size());
		assert((uint)theSizesCache.count() == (uint)theOutputs.size());
		if (MESSAGES) qDebug("Processor::confirmTypes(): (%s) Enforcing outputs minima (from cache):", qPrintable(name()));
		for (uint i = 0; i != theTypesCache.count(); i++)
		{
			if (!theOutputs[i])
				theOutputs[i] = new LxConnectionNull(this, i);
			if (MESSAGES) qDebug("Processor::confirmTypes(): (%s) Output %d : %d samples", qPrintable(name()), i, theSizesCache[i]);
			theOutputs[i]->setType(theTypesCache[i]);
			theOutputs[i]->setMinimumWrite(theSizesCache[i]);
		}
		return true;
	}

	Types inTypes(theInputs.count());
	uint ii = 0;
	for (QVector<xLConnection *>::Iterator i = theInputs.begin(); i != theInputs.end(); i++,ii++)
	{	if (!*i)
		{	if (MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) Input not connected.", qPrintable(name()));
			theTypesConfirmed = false;
			QFastMutexLocker lock(&theErrorSystem);
			theError = InputNotConnected;
			theErrorData = ii;
			theErrorWritten.wakeAll();
			return false;
		}
		Type t = (*i)->type();
		if (t.isNull())
		{	if (MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) Input %d is null - exiting with error.", qPrintable(name()), ii);
			theTypesConfirmed = false;
			QFastMutexLocker lock(&theErrorSystem);
			theError = InputTypeNull;
			theErrorData = ii;
			theErrorWritten.wakeAll();
			return false;
		}
		// We copy this pointer's data now, and inTypes will autodelete it on exit from function.
		inTypes[ii] = t;
	}
	assert(inTypes.count() == (uint)theInputs.size());

	theTypesCache.resize(theOutputs.size());

	// Do a quick check to make sure that we're going by the multiplicity rules
	if ((theMulti&In) && inTypes.count())
	{		if (!inTypes.allSame())
			{	if (MESSAGES) qDebug("Processor::confirmTypes(): (%s) No input types not homogeneous.", qPrintable(name()));
				theTypesConfirmed = false;
				QFastMutexLocker lock(&theErrorSystem);
				theError = InputsNotHomogeneous;
				theErrorData = ii;
				theErrorWritten.wakeAll();
				return false;
			}
	}

	theTypesConfirmed = verifyAndSpecifyTypes(inTypes, theTypesCache);
	if (!theTypesConfirmed)
	{
		if (MESSAGES) qDebug("Processor::confirmTypes(): (%s) Invalid inputs (verifyAndSpecifyTypes() returned false)", qPrintable(name()));
		QFastMutexLocker lock(&theErrorSystem);
		theError = InvalidInputs;
		theErrorWritten.wakeAll();
	}

	// NOTE: DomProcessor::verifyAndSpecifyTypes depends on this code.
	if (theTypesCache.count())
	{
		if (theTypesCache[0].isNull())
		{	if (MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) No output type specified.", qPrintable(name()));
			theTypesConfirmed = false;
			QFastMutexLocker lock(&theErrorSystem);
			theError = OutputsNull;
			theErrorData = ii;
			theErrorWritten.wakeAll();
			return false;
		}
		theTypesCache.fillEmpty(theTypesCache[0]);
		for (uint i = 0; i < theTypesCache.count(); i++)
			assert (!theTypesCache[i].isNull());
		// TODO: Enforce the same basic class rule.
	}

	{
		QVector<uint> sizes(theInputs.count());
		specifyInputSpace(sizes);
		int ii = 0;
		if (MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) Enforcing inputs minima:", qPrintable(name()));
		for (QVector<xLConnection *>::Iterator i = theInputs.begin(); i != theInputs.end(); i++, ii++)
		{	if (MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) Input %d : %d samples", qPrintable(name()), ii, sizes[ii]);
			(*i)->setMinimumRead(sizes[ii]);
		}
	}

	theSizesCache.resize(theOutputs.size());
	specifyOutputSpace(theSizesCache);
	if (theTypesConfirmed)
	{	assert(theTypesCache.count() == (uint)theOutputs.count());
		if (MESSAGES) qDebug("Processor::confirmInputTypes(): Enforcing outputs minima for %s:", qPrintable(name()));
		for (uint i = 0; i < theTypesCache.count(); i++)
		{
			if (!theOutputs[i])
				theOutputs[i] = new LxConnectionNull(this, i);
			if (theTypesCache[i].isNull())
				qFatal("*** FATAL: TypesCache has unpopulated entry (%d in %s). Bailing.", i, qPrintable(name()));
			if (MESSAGES) qDebug("Processor::confirmInputTypes(): Output %d: Setting type...", i);
			theOutputs[i]->setType(theTypesCache[i]);
			if (MESSAGES) qDebug("Processor::confirmInputTypes(): Output %d: Enforcing minimum %d", i, theSizesCache[i]);
			theOutputs[i]->setMinimumWrite(theSizesCache[i]);
		}
	}

	if (MESSAGES) qDebug("Processor::confirmInputTypes(): (%s) All done (%d).", qPrintable(name()), theTypesConfirmed);
	return theTypesConfirmed;
}

void Processor::split(uint sourceIndex)
{
	if (isRunning())
	{	qWarning("*** ERROR: Processor::split: %s[%d]: Cannot change connection states while running.", qPrintable(name()), sourceIndex);
		return;
	}
	if (sourceIndex >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::split: %s[%d]: Invalid source index to connect from.", qPrintable(name()), sourceIndex);
		return;
	}
	if (theOutputs[sourceIndex])
	{	qWarning("*** ERROR: Processor::split: %s[%d]: Cannot split an output that is already connected.", qPrintable(name()), sourceIndex);
		return;
	}

	new Splitter(this, sourceIndex);
}

void Processor::share(uint sourceIndex, uint bufferSize)
{
	if (isRunning())
	{	qWarning("*** ERROR: Processor::share: %s[%d]: Cannot change connection states while running.", qPrintable(name()), sourceIndex);
		return;
	}
	if (sourceIndex >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::share: %s[%d]: Invalid source index to connect from.", qPrintable(name()), sourceIndex);
		return;
	}
	if (theOutputs[sourceIndex])
	{	qWarning("*** ERROR: Processor::share: %s[%d]: Cannot share an output that is already connected.", qPrintable(name()), sourceIndex);
		return;
	}

	new LMConnection(this, sourceIndex, bufferSize);
}

void Processor::connectCheck() const
{
	qDebug("Processor::connectCheck: %s: theMulti=%d.", qPrintable(name()), theMulti);
	assert(theMulti&Out);
}

bool Processor::readyRegisterIn(uint sinkIndex) const
{
	if (sinkIndex >= (uint)theInputs.size())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Invalid sink index to connect to. (%p inputs: %d)", qPrintable(name()), sinkIndex, this, theInputs.size());
		return false;
	}
	if (theInputs[sinkIndex])
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Cannot connect to an already connected input.", qPrintable(name()), sinkIndex);
		return false;
	}
	return true;
}

class DLLEXPORT xLsConnectionReal: public xLConnection
{
protected:
	xLsConnectionReal(Sink* _newSink, uint _newSinkIndex): xLConnection(_newSink, _newSinkIndex) {}
	~xLsConnectionReal() {}

	Type const& type() const { if (theType.isNull()) const_cast<xLsConnectionReal*>(this)->pullType(); return theType; }

	void commit()
	{
		if ((int)m_toTransfer < m_readBuffer.size())
			memmove(m_readBuffer.data(), m_readBuffer.data() + m_readBuffer.size() - m_toTransfer, (m_readBuffer.size() - m_toTransfer) * sizeof(float));
		memcpy(m_readBuffer.data() + m_readBuffer.size() - min<int>(m_readBuffer.size(), m_toTransfer), m_writeBuffer.data() + m_toTransfer - min<int>(m_readBuffer.size(), m_toTransfer), min<int>(m_readBuffer.size(), m_toTransfer) * sizeof(float));
		m_toTransfer = 0;
	}

	virtual bool pullType() = 0;
	virtual void reset() { m_samplesRead = 0; m_latestPeeked = 0; qDebug() << "Reseting connection."; m_readBuffer.fill(0); m_writeBuffer.fill(0); m_toTransfer = 0; }

	QVector<float> m_writeBuffer;
	QVector<float> m_readBuffer;
	uint m_toTransfer;

	mutable uint64_t m_samplesRead;
	mutable uint64_t m_latestPeeked;

private:
	virtual void sinkStopping() {}
	virtual void sinkStopped() {}
	virtual uint elementsReady() const { return m_readBuffer.size(); }
	virtual void waitForElements(uint) const {}
	virtual BufferData const readElements(uint _elements) { assert((int)_elements <= m_readBuffer.size()); return BufferData(m_readBuffer.constData() + (uint)m_readBuffer.size() - _elements, (uint)m_readBuffer.size(), type().size()); }
	virtual BufferData const peekElements(uint _elements) { return readElements(_elements); }
	virtual void enforceMinimumRead(uint _elements) { m_readBuffer.resize(_elements); m_readBuffer.fill(0); }
	virtual void enforceMinimumWrite(uint _elements) { m_writeBuffer.resize(_elements); m_writeBuffer.fill(0); }
	virtual BufferReader* newReader() { return 0; }
	virtual void killReader() {}
	virtual void resurectReader() {}
	virtual uint capacity() const { return (m_readBuffer.size() + m_writeBuffer.size()) / theType->size(); }
	virtual float filled() const { return 1.0; }
	virtual bool plungeSync(uint) const { return true; }
	virtual bool require(uint, uint = Undefined) { return true; }
	virtual double secondsPassed() const { return 0.0; }
	virtual double secondsPassed(float) const { return 0.0; }
};

class DLLEXPORT LLsConnection: public LxConnectionReal, public xLsConnectionReal
{
	//* Reimplementations from LxConnection
	virtual bool waitUntilReady() { return theSink->waitUntilReady(); }
	virtual Tristate isReadyYet() { return theSink->isGoingYet(); }
	virtual const Type& type() const { return xLsConnectionReal::type(); }
	virtual void setType(Type const& _type) { LxConnectionReal::setType(_type); }
	virtual void resetType() { theType = TransmissionType(); xLsConnectionReal::reset(); }
	virtual void reset() { xLsConnectionReal::reset(); }
	virtual void sourceStopping() {}
	virtual void sourceStopped() {}
	virtual BufferData makeScratchElements(uint _elements, bool autoPush) { return BufferData(_elements, type().size(), m_writeBuffer.data(), this, autoPush ? BufferInfo::Activate : BufferInfo::Ignore); }
	virtual void pushPlunger() { plungerSent(); }
	virtual void startPlungers() { theSink->startPlungers(); }
	virtual void plungerSent() { theSink->plungerSent(theSinkIndex); }
	virtual void noMorePlungers() { theSink->noMorePlungers(); }
	virtual uint freeInDestinationBuffer(uint) { return m_writeBuffer.size(); }
	virtual uint freeInDestinationBufferEver() { return m_writeBuffer.size(); }

	//* Reimplementation from xLConnection.
	virtual bool pullType() { theSource->confirmTypes(); return !theType.isNull(); }

	//* Reimplementation from LxConnectionReal.
	virtual void bufferWaitForFree() {}
	virtual uint bufferElementsFree() { return m_writeBuffer.size(); }
	virtual void transport(BufferData const& _data) { m_toTransfer = _data.elements(); commit(); theSource->checkExit(); }

	friend class Processor;
	LLsConnection(Source* _newSource, uint _newSourceIndex, Sink* _newSink, uint _newSinkIndex):
		LxConnectionReal(_newSource, _newSourceIndex),
		xLsConnectionReal(_newSink, _newSinkIndex)
	{}
};


const Connection *Processor::connect(uint _sourceIndex, Sink *_sink, uint _sinkIndex, uint _bufferSize)
{
	if (isRunning())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Cannot change connection states while running.", qPrintable(name()), _sourceIndex);
		return 0;
	}
	if (_sourceIndex >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Invalid source index to connect from.", qPrintable(name()), _sourceIndex);
		return 0;
	}

	if (!_sink->readyRegisterIn(_sinkIndex)) return 0;

	if (theOutputs[_sourceIndex] == 0)
	{
		if (specifyInputMode(_sourceIndex))
			return new LLsConnection(this, _sourceIndex, _sink, _sinkIndex);
		else
			return new LLConnection(this, _sourceIndex, _sink, _sinkIndex, _bufferSize);
	}
	else if (dynamic_cast<LMConnection *>(theOutputs[_sourceIndex]))
	{
		LMConnection *c = dynamic_cast<LMConnection *>(theOutputs[_sourceIndex]);
		return new MLConnection(_sink, _sinkIndex, c);
	}
	else if (dynamic_cast<Splitter *>(theOutputs[_sourceIndex]))
	{
		Splitter *s = dynamic_cast<Splitter *>(theOutputs[_sourceIndex]);
		return new LLConnection(s, 0, _sink, _sinkIndex, _bufferSize);
	}
	else
	{	qWarning("*** ERROR: Processor::connect: Output %s[%d] already connected and is neither split\n"
			   "           nor share()d.", qPrintable(name()), _sourceIndex);
		return 0;
	}
}

const Connection *Processor::connect(uint sourceIndex, const QString &sinkHost, uint sinkKey, const QString &sinkProcessorName, uint sinkIndex, uint bufferSize)
{
	if (isRunning())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Cannot change connection states while running.", qPrintable(name()), sourceIndex);
		return 0;
	}
	if (sourceIndex >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::connect: %s[%d]: Invalid source index to connect from.", qPrintable(name()), sourceIndex);
		return 0;
	}

	// TODO: Need remote version of readyRegisterIn

	if (theOutputs[sourceIndex] == 0)
		return ProcessorForwarder::createConnection(this, sourceIndex, bufferSize, sinkHost, sinkKey, sinkProcessorName, sinkIndex);
	else
	{	Splitter *s = dynamic_cast<Splitter *>(theOutputs[sourceIndex]);

		if (!s)
		{	qWarning("*** ERROR: Processor::connect: %s[%d]: This output is already connected and not declared\n"
				   "           split.", qPrintable(name()), sourceIndex);
			return 0;
		}

		return ProcessorForwarder::createConnection(s, 0, bufferSize, sinkHost, sinkKey, sinkProcessorName, sinkIndex);
	}
}

void Processor::disconnect(uint index)
{
	if (isRunning())
	{	qWarning("*** WARNING: Processor::disconnect: %s[%d]: Disconnecting input on a running processor.\n"
				 "             Stopping first.", qPrintable(name()), index);
		stop();
	}
	if (index >= (uint)theOutputs.size())
	{	qWarning("*** ERROR: Processor::disconnect: %s[%d]: Invalid output index to connect from.", qPrintable(name()), index);
		return;
	}
	if (!theOutputs[index])
	{	qWarning("*** ERROR: Processor::disconnect: %s[%d]: Output is not connected.", qPrintable(name()), index);
		return;
	}

	delete theOutputs[index];
}

Connection::Tristate Processor::isGoingYet()
{
	if (theError == Pending || theError == NotStarted)
		return Connection::Pending;
	else if (theError == NoError)
		return Connection::Succeeded;
	else
		return Connection::Failed;
}

Processor::ErrorType Processor::waitUntilGoing(int *errorData)
{
	QFastMutexLocker lock(&theErrorSystem);
	while (theError == Pending || theError == NotStarted)
		theErrorWritten.wait(&theErrorSystem);
	if (errorData) *errorData = theErrorData;
	return theError;
}

QString Processor::error() const
{
	switch (theError)
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

bool Processor::waitUntilReady()
{
	return waitUntilGoing() == NoError;
}

void Processor::bail()
{
	throw BailException();
}

void Processor::reset()
{
	if (MESSAGES) qDebug("> Processor::reset() [%s]", qPrintable(name()));

	thePlungersStarted = false;
	thePlungersEnded = false;
	theError = NotStarted;

	// unconfirm types --- or the next time our consumer will assume they're already confirmed (or !
	if (MESSAGES) qDebug("= Processor::reset(): Unconfirming types...");
	for (uint i = 0; i < (uint)theOutputs.size(); i++)
		if (theOutputs[i])
			theOutputs[i]->resetType();

	// undo our meddling with output slots
	if (MESSAGES) qDebug("= Processor::reset(): Deleting null outputs...");
	for (uint i = 0; i < (uint)theOutputs.size(); i++)
		if (dynamic_cast<LxConnectionNull *>(theOutputs[i]))
		{	delete theOutputs[i];
			theOutputs[i] = 0L;
		}

	if (MESSAGES) qDebug("< Processor::reset()");
}

void Processor::setupIO(uint inputs, uint outputs, uint _inputSpace, uint _outputSpace)
{
	assert(!isRunning());

	if (theIOSetup)
	{
		qWarning() << "*** Processor::setupIO(): IO has already been setup! Bailing.";
		return;
	}

	uint rinputs = inputs;
	uint routputs = outputs;
	m_inputSpace = _inputSpace;
	m_outputSpace = _outputSpace;

	if ((theMulti&In) && !(theMulti&Const))
	{	if (rinputs != Undefined)
			qWarning("*** Processor::setupIO(): You have specified %d inputs in setupIO, but the"
					 "    processor has non-fixed multiple inputs. Overriding to multiplicity %d.", rinputs, theGivenMultiplicity);
		rinputs = theGivenMultiplicity;
	}
	else if ((theMulti&In) && (theMulti&Const))
	{
		if (!rinputs)
		{	qWarning("*** Processor::setupIO() [%s]: You have specified 0 inputs in setupIO,"
					 "    but the processor has fixed multiple inputs. SetupIO aborted.", qPrintable(name()));
			return;
		}
		theGivenMultiplicity = rinputs;
	}
	else if (!((theMulti&In) && !(theMulti&Const)) && rinputs == Undefined)
		qFatal("*** Processor::setupIO(): Undefined inputs, when non/fixed Multi.");

	if ((theMulti&Out) && !(theMulti&Const))
	{	if (routputs != Undefined)
			qWarning("*** Processor::setupIO(): You have specified %d outputs in setupIO, but the"
					 "    processor has non-fixed multiple outputs. Overriding to multiplicity %d.", routputs, theGivenMultiplicity);
		routputs = theGivenMultiplicity;
	}
	else if ((theMulti&Out) && (theMulti&Const))
	{
		if (!routputs)
		{	qWarning("*** Processor::setupIO() [%s]: You have specified 0 outputs in setupIO,"
					 "    but the processor has fixed multiple outputs. SetupIO aborted.", qPrintable(name()));
			return;
		}
		if (theGivenMultiplicity != Undefined && theGivenMultiplicity != routputs)
		{	qWarning("*** Processor::setupIO() [%s]: Incompatible multiplicity. Bailing.", qPrintable(name()));
			return;
		}
		theGivenMultiplicity = routputs;
	}
	else if (!((theMulti&Out) && !(theMulti&Const)) && routputs == Undefined)
		qFatal("*** Processor::setupIO(): Undefined outputs, when not unfixed Multi.");

	if (theMulti == InOutConst)
	{	if (rinputs != routputs)
		{	qWarning("*** Processor::setupIO(): You have specified %d inputs in setupIO, but %d"
					 "    outputs. InOutConst processors must have the same multiplicity. Overriding"
					 "    to minimum of the two (%d).", rinputs, routputs, min(rinputs, routputs));
			rinputs = routputs = min(rinputs, routputs);
		}
	}

	bool wellDefined = true;

	if (rinputs == Undefined)
	{
		wellDefined = false;
		rinputs = 0;
	}
	uint ois = theInputs.size();
	for (uint i = rinputs; i < (uint)theInputs.size(); i++)
		delete theInputs[i];
	theInputs.resize(rinputs);
	for (uint i = ois; i < rinputs; i++)
		theInputs[i] = 0;

	if (routputs == Undefined)
	{
		wellDefined = false;
		routputs = 0;
	}
	uint oos = theOutputs.size();
	for (uint i = routputs; i < (uint)theOutputs.size(); i++)
		delete theOutputs[i];
	theOutputs.resize(routputs);
	for (uint i = oos; i < routputs; i++)
		theOutputs[i] = 0L;

	if (!wellDefined)
		return;

	// We need to size up the plunger stuff.
	thePlungersLeft.resize(rinputs);
	thePlungersNotified.resize(rinputs);
	thePlungedInputs.resize(rinputs);

	theIOSetup = true;
}

void Processor::resetMulti()
{
	if (theMulti && !(theMulti&Const) && theHardMultiplicity == Undefined)
	{
		theIOSetup = false;
		theGivenMultiplicity = Undefined;

		if (theMulti & In)
		{
			for (uint i = 0; i < (uint)theInputs.size(); i++)
				delete theInputs[i];
			theInputs.resize(0);
		}
		if (theMulti & Out)
		{
			for (uint i = 0; i < (uint)theOutputs.size(); i++)
				delete theOutputs[i];
			theOutputs.resize(0);
		}
	}
}

void Processor::updateFromProperties(Properties const& _p)
{
	loadProperties(_p, true);
	updateFromProperties();
}

void Processor::initFromProperties(Properties const& _p)
{
	updateDynamics(specifyProperties());
	loadProperties(_p, false);
	initFromProperties();
}

HeavyProcessor::HeavyProcessor(QString const& _type, MultiplicityType _m, uint _flags):
	QThread(0),
	Processor(_type, _m),
	thePaused(false),
	theFlags(_flags)
{
}

void HeavyProcessor::pause()
{
	QFastMutexLocker lock(&theStop);
	if (!theStopping)
	{	QFastMutexLocker lock(&thePause);
		thePaused = true;
	}
}

void HeavyProcessor::unpause()
{
	thePause.lock();
	theGuardsCrossed = 0;
	thePaused = false;
	theUnpaused.wakeAll();
	thePause.unlock();
}

void HeavyProcessor::getReadyForStopping()
{
	thePause.lock();
	if (thePaused)
	{	thePaused = false;
		theUnpaused.wakeAll();
	}
	thePause.unlock();
}

void HeavyProcessor::run()
{
	setThreadProcessor();

	if (!theTypesConfirmed)
		return;

	// Wait for them to confirm their own types before we start our processing/pushing data.
	if (MESSAGES) qDebug("Processor::run(): (%s) Waiting for outputs...", qPrintable(name()));
	for (uint i = 0; i < (uint)theOutputs.size(); i++)
		if (theOutputs[i])
		{	if (MESSAGES) qDebug("Processor::run(): (%s) Waiting on output %d...", qPrintable(name()), i);
			if (!theOutputs[i]->waitUntilReady())
			{	if (MESSAGES) qDebug("Processor::run(): (%s) Output %d had some error starting. Recursive failure imminent.", qPrintable(name()), i);
				QFastMutexLocker lock(&theErrorSystem);
				theError = RecursiveFailure;
				theErrorData = i;
				theErrorWritten.wakeAll();
				if (MESSAGES) qDebug("Processor::run(): (%s) Error recorded. Bailing...", qPrintable(name()));
				return;
			}
		}

	if (MESSAGES) qDebug("Processor::run(): (%s) All tests completed. Releasing lock and starting.", qPrintable(name()));
	theErrorSystem.lock();
	theError = NoError;
	theErrorWritten.wakeAll();
	theErrorSystem.unlock();
	for (uint i = 0; i < (uint)thePlungedInputs.count(); i++)
		thePlungedInputs[i] = 0L;

	// Execute processor with exception handler to bail it if it throws an int
	try
	{
		if (!theInputs.count())
			for (uint i = 0; i < (uint)theOutputs.count(); i++)
				theOutputs[i]->startPlungers();

		if (MESSAGES) qDebug("Processor[%s]: Plungers primed; starting task...", qPrintable(name()));

		processor();

		if (MESSAGES) qDebug("Processor[%s]: Task done.", qPrintable(name()));
		{	QFastMutexLocker lock(&theStop);
			theAllDone = true;
			theAllDoneChanged.wakeAll();
		}

		if (MESSAGES) qDebug("Processor[%s]: Informing of no more plungers...", qPrintable(name()));
		for (uint i = 0; i < (uint)theOutputs.count(); i++)
			theOutputs[i]->noMorePlungers();
		if (MESSAGES) qDebug("Processor[%s]: Dispatching last plunger...", qPrintable(name()));
		// We must remember that we started expecting a plunger that we never sent, so...
		// Send plunger without a corresponding plungerSent(), in order to make it symmetrical
		for (uint i = 0; i < (uint)theOutputs.count(); i++)
			theOutputs[i]->pushPlunger();

		if (MESSAGES) qDebug("Processor[%s]: Finished. Holding until stop()ed...", qPrintable(name()));
		while (1)
		{	pause();
			thereIsInputForProcessing();
		}
	}
	catch(BailException &) {}
	catch(int e) {}

	if (MESSAGES) qDebug("Processor stopping (name=%s).", qPrintable(name()));

	processorStopped();

	if (MESSAGES) qDebug("Stopped.");
}

void HeavyProcessor::waitUntilDone()
{
	if (!(theFlags & Guarded))
		qWarning("*** WARNING: Processor::waitUntilDone(): I'll never exit, since I'm not a\n"
				 "             Guarded Processor-derived object (name=%s).", qPrintable(name()));
	QFastMutexLocker lock(&theStop);
	while (!theAllDone)
		theAllDoneChanged.wait(&theStop);
}

CoProcessor::CoProcessor(QString const& _type, MultiplicityType _m):
	Processor(_type, _m)
{
}

void CoProcessor::waitUntilDone()
{
	//TODO
}

int CoProcessor::cyclesReady()
{
	QVector<uint> mData(numInputs());
	specifyInputSpace(mData);
	QVector<uint> rData(numInputs());
	requireInputSpace(rData);
	QVector<uint> mSpace(numOutputs());
	specifyOutputSpace(mSpace);

	uint cycles = UINT_MAX;
	for (uint i = 0; i < numOutputs(); i++)
	{
		uint bFree = min<uint>(UINT_MAX / 2, theOutputs[i]->maximumScratchSamples(0));
		if (bFree < mSpace[i])
			return 0;
		else if (mSpace[i] > 0)
			cycles = min(cycles, bFree / mSpace[i]);
	}

	if (numInputs())
	{
		QVector<uint> is(numInputs());
		for (uint i = 0; i < numInputs(); i++)
			if (theInputs[i]->require(rData[i], mData[i]))
				is[i] = min<uint>(UINT_MAX / 2, theInputs[i]->samplesReady() / max(1u, mData[i]));
			else
				is[i] = Undefined;
		cycles = min(cycles, cyclesAvailable(is));
	}

	return cycles;
}

int CoProcessor::doWork()
{
	if (MESSAGES&&0) qDebug("Processor[%s]: > doWork()", qPrintable(name()));
	int ret = DidWork;
	// TODO: check if canProcess could end up falling through trapdoor; if not then set/unsetThreadProcessor can be moved to go around process().
	// Same with try/catch.
	setThreadProcessor();
	try
	{
		if (theStopping)
			ret = WillNeverWork;
		else if (theError == Pending)
		{
			if (MESSAGES) qDebug("Processor::processCycle(): (%s) Checking outputs...", qPrintable(name()));
			bool allOk = true;
			for (uint i = 0; i < (uint)theOutputs.size(); i++)
			{
				assert(theOutputs[i]);
				Connection::Tristate e = theOutputs[i]->isReadyYet();
				if (e == Connection::Failed)
				{	if (MESSAGES) qDebug("Processor::processCycle(): (%s) Output %d had some error starting. Recursive failure imminent.", qPrintable(name()), i);
					QFastMutexLocker lock(&theErrorSystem);
					theError = RecursiveFailure;
					theErrorData = i;
					theErrorWritten.wakeAll();
					if (MESSAGES) qDebug("Processor::processCycle(): (%s) Error recorded. Bailing...", qPrintable(name()));
					ret = WillNeverWork;
					allOk = false;
					break;
				}
				else if (e == Connection::Pending)
					allOk = false;
			}
			if (allOk)
			{
				{
					QFastMutexLocker lock(&theErrorSystem);
					theError = NoError;
					theErrorWritten.wakeAll();
				}
				for (uint i = 0; i < (uint)thePlungedInputs.count(); i++)
					thePlungedInputs[i] = 0L;
				if (!theInputs.count())
					for (uint i = 0; i < (uint)theOutputs.count(); i++)
						theOutputs[i]->startPlungers();
			}
		}
		else if (theError == NoError)
		{
			ret = canProcess();
			if (ret > 0)
			{
				int cr = cyclesReady();
				if (cr > 0)
				{
					theGuardsCrossed++;
					ret = process();
				}
				else
					ret = NoWork;
			}

			if (ret == WillNeverWork)
			{
				if (MESSAGES) qDebug("Processor[%s]: Task done.", qPrintable(name()));
				{	QFastMutexLocker lock(&theStop);
					theAllDone = true;
					theAllDoneChanged.wakeAll();
				}

				if (MESSAGES) qDebug("Processor[%s]: Informing of no more plungers...", qPrintable(name()));
				for (uint i = 0; i < (uint)theOutputs.count(); i++)
					theOutputs[i]->noMorePlungers();
				if (MESSAGES) qDebug("Processor[%s]: Dispatching last plunger...", qPrintable(name()));
				// We must remember that we started expecting a plunger that we never sent, so...
				// Send plunger without a corresponding plungerSent(), in order to make it symmetrical
				for (uint i = 0; i < (uint)theOutputs.count(); i++)
					theOutputs[i]->pushPlunger();
			}
		}
	}
	catch(BailException &) { ret = WillNeverWork; }
	catch(int e) { ret = WillNeverWork; }
	unsetThreadProcessor();
	if (MESSAGES&&0) qDebug("Processor[%s]: < doWork() [returned %d]", qPrintable(name()), ret);
	return ret;
}

void CoProcessor::onStopped()
{
	processorStopped();
}

}

#undef MESSAGES
