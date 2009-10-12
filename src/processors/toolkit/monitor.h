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

#pragma once

#include <qdatetime.h>
#include <qmutex.h>
#include <qwaitcondition.h>

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "qfastwaitcondition.h"
#include "processor.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <geddei/processor.h>
#endif

/** @ingroup Toolkit
 * @brief Communications monitoring Processor object.
 * @author Gav Wood <gav@kde.org>
 *
 * This is an extended Processor-based class that is essentially just a
 * repeater. It may be placed between two other Processor objects and through
 * the extra API methods can give the developer information as to the speed and
 * nature of the data transfer.
 *
 * It also has a couple of methods allowing the developer to block based on
 * waiting for plungers, meaning they can be counted or otherwise acted upon.
 */
class DLLEXPORT Monitor: public HeavyProcessor
{
	virtual bool processorStarted();
	virtual void processor();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);
	virtual void receivedPlunger();
	virtual void specifyOutputSpace(QVector<uint> &samples);

	QFastMutex thePlunging;
	QFastWaitCondition thePlunge;
	uint thePlungersCaught;

	uint theScope;
	float theFrequency;

	uint theTotalSamples;
	double theTotalTime;
	QTime theTimer;

public:
	/**
	 * Determine the realtime throughput.
	 *
	 * @return The number of bits that flowed through this processor per
	 * (real time) second.
	 */
	double averageThroughput() { return double(elementsProcessed() * 32) / theTotalTime; }

	/**
	 * Determine time elapsed between go() and stop().
	 *
	 * @return The number of seconds elapsed between go() and stop().
	 */
	double elapsedTime() { return theTotalTime; }

	/**
	 * Determine the number of (32-bit) elements that flowed through this object.
	 *
	 * @return The total number of elements to flow through this object.
	 */
	uint elementsProcessed() { return theTotalSamples * theScope; }

	/**
	 * Determine the number of samples that flowed through this object.
	 *
	 * @return The total number of samples to flow through this object.
	 */
	uint samplesProcessed() { return theTotalSamples; }

	/**
	 * Determine the total amount of signal time to flow through this object.
	 *
	 * @return the amount of signal, measured in seconds, that flowed through
	 * this object.
	 */
	double signalProcessed() { return double(theTotalSamples) / theFrequency; }

	/**
	 * Determine how many times faster than realtime the signal flowed through
	 * this object. If the signal is flowing through in real time (e.g. because
	 * the source of the network is realtime-based) then this should be very
	 * close to 1.0.
	 *
	 * @return The number of times faster than real the signal was processed.
	 */
	double timesFasterThanReal() { return signalProcessed() / theTotalTime; }

	/**
	 * Block until a plunger flows through this object. If a plunger has
	 * already flowed through that hasn't itself been waited for explicity,
	 * exit immediately. If you don't want this to happen, then call
	 * resetPlungerMemory() first.
	 *
	 * Typically used in a basic for () loop to count until the required number
	 * of plungers has passed through the network.
	 *
	 * @sa resetPlungerMemory()
	 */
	void waitForPlunger();

	/**
	 * Any plungers that have flowed through previously will be instantly
	 * forgotten. waitForPlunger will only exit for plungers delivered after
	 * this call.
	 *
	 * @sa waitForPlunger()
	 */
	void resetPlungerMemory();

	Monitor(): HeavyProcessor("Monitor", NotMulti, Guarded), thePlungersCaught(0) {}
};
