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


#include "qfactoryexporter.h"

#include "bufferdata.h"
#include "buffer.h"
#include "processor.h"
#include "processorforwarder.h"
#include "processorgroup.h"
using namespace Geddei;

#include "contiguous.h"
using namespace TransmissionTypes;

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
	QFastMutexLocker lock(&thePlunging);
	thePlungersCaught++;
	thePlunge.wakeAll();
	qDebug("--- [%s] Total samples received: %d", qPrintable(name()), theTotalSamples);
}

void Monitor::resetPlungerMemory()
{
	QFastMutexLocker lock(&thePlunging);
	if (MESSAGES) qDebug("Monitor::resetPlunger(): Reseting (from %d)...", thePlungersCaught);
	thePlungersCaught = 0;
}

void Monitor::waitForPlunger()
{
	if (MESSAGES) qDebug("Monitor::waitForPlunger(): Waiting for plunger...");
	QFastMutexLocker lock(&thePlunging);
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

bool Monitor::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (!inTypes[0].isA<Contiguous>())
		return false;
	outTypes[0] = inTypes[0];
	m_arity = inTypes[0].asA<Contiguous>().arity();
	theFrequency = inTypes[0].asA<Contiguous>().frequency();
	return true;
}

void Monitor::initFromProperties(const Properties &)
{
	setupIO(1, 1);
}

EXPORT_CLASS(Monitor, 0,1,1, Processor);
