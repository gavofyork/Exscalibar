/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#define __GEDDEI_BUILD

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "buffer.h"
#include "processor.h"
#include "processorforwarder.h"
#include "processorgroup.h"
using namespace Geddei;

#include "monitor.h"

#define MESSAGES 0

void Monitor::processor()
{
	theTimer.start();
	theTotalSamples = 0;
	while (thereIsInputForProcessing(1))
	{
		BufferData d = input(0).readSamples();
		theTotalSamples += d.samples();
		output(0).push(d);
	}
}

void Monitor::receivedPlunger()
{
	if (MESSAGES) qDebug("Monitor::receivedPlunger()");
	QMutexLocker lock(&thePlunging);
	thePlungersCaught++;
	thePlunge.wakeAll();
	qDebug("--- [%s] Total samples received: %d", qPrintable(name()), theTotalSamples);
}

void Monitor::resetPlungerMemory()
{
	QMutexLocker lock(&thePlunging);
	if (MESSAGES) qDebug("Monitor::resetPlunger(): Reseting (from %d)...", thePlungersCaught);
	thePlungersCaught = 0;
}

void Monitor::waitForPlunger()
{
	if (MESSAGES) qDebug("Monitor::waitForPlunger(): Waiting for plunger...");
	QMutexLocker lock(&thePlunging);
	while (!thePlungersCaught) thePlunge.wait(&thePlunging);
	if (MESSAGES) qDebug("Monitor::waitForPlunger(): Done. Caught: %d", thePlungersCaught);
	thePlungersCaught--;
	thePlunge.wakeAll();
}

bool Monitor::processorStarted()
{
	thePlungersCaught = 0;
	return true;
}

void Monitor::processorStopped()
{
	theTotalTime = double(theTimer.elapsed()) / 1000.0;
}

void Monitor::specifyOutputSpace(QVector<uint> &samples)
{
	samples[0] = input(0).capacity() / 2;
}

bool Monitor::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	outTypes[0] = inTypes[0];
	theScope = inTypes[0].scope();
	theFrequency = inTypes[0].frequency();
	return true;
}

void Monitor::initFromProperties(const Properties &)
{
	setupIO(1, 1);
}

EXPORT_CLASS(Monitor, 0,1,1, Processor);
