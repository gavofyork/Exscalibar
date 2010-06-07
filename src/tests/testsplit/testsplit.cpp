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

#include <iostream>

#include "geddei.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace TransmissionTypes;

class PlungeGenerator : public HeavyProcessor
{
	virtual void processor()
	{
//		while (true)
		{
			for (unsigned int i = 0; i < 5; i++)
			{	BufferData s = output(0).makeScratchSamples(10);
				for (unsigned int j = 0; j < 10; j++)
					s[j] = i;
				output(0) << s;
				std::cout << "Generator " << qPrintable(name()) << ": Pushing plunger..." << std::endl;
				plunge();
			}
		}
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes) { outTypes[0] = Wave(22050); return true; }
	virtual void initFromProperties(const Properties &) { setupIO(0, 1); }

public:
	PlungeGenerator(): HeavyProcessor("PlungeGenerator") {}
};

class PlungeDetector : public HeavyProcessor
{
	void processor()
	{
		while (true)
		{
			BufferData s = input(0).readSamples(5);
			output(0) << s;
		}
	}

	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
	{
		outTypes[0] = inTypes[0];
		return true;
	}

	void initFromProperties(const Properties &)
	{
		setupIO(1, 1);
	}

	virtual void receivedPlunger()
	{
		std::cout << "Detector " << qPrintable(name()) << ": Received plunger!" << std::endl;
	}
public:
	PlungeDetector(): HeavyProcessor("PlungeDetector") {}
};

uint theCount;
QFastWaitCondition theCondition;
QFastMutex theMutex;

class PlungeEater : public HeavyProcessor
{
	void processor()
	{
		while (true)
		{
			input(0).readSamples(1);
		}
	}

	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &)
	{
		return true;
	}

	virtual void receivedPlunger()
	{
		std::cout << "Eater " << qPrintable(name()) << ": Received plunger!" << std::endl;
		QFastMutexLocker lock(&theMutex);
		theCount++;
		theCondition.wakeAll();
	}

	void initFromProperties(const Properties &)
	{
		setupIO(1, 0);
	}
public:
	PlungeEater(): HeavyProcessor("PlungeEater") {}
};

int main()
{
	Processor	*O = new PlungeGenerator,
				*D = new PlungeDetector,
				*E = new PlungeDetector,
				*F = new PlungeDetector,
				*I = new PlungeEater;

	ProcessorGroup objects;
	O->init("O", objects);
	D->init("D", objects);
	E->init("E", objects);
	F->init("F", objects);
	I->init("I", objects);

	std::cout << "Connecting..." << std::endl;
	(*O)[0].setSize(10).split();
	(*O)[0] >>= (*D)[0].setSize(10);
	(*O)[0] >>= (*F)[0].setSize(10);
	(*O)[0] >>= (*E)[0].setSize(10);
	(*E)[0] >>= (*I)[0].setSize(10);

	std::cout << "Checking network..." << std::endl;
	if (!objects.confirmTypes())
		qFatal("Couldn't confirm types.");

	theCount = 0;
	theMutex.lock();

	std::cout << "Starting objects..." << std::endl;
	objects.go();

	std::cout << "Waiting for 5 plungers..." << std::endl;
	while (theCount < 5)
		theCondition.wait(&theMutex);
	theMutex.unlock();

	std::cout << "Stopping objects..." << std::endl;
	objects.stop();

	std::cout << "Disconnecting..." << std::endl;
	(*O)[0] --;
	(*E)[0] --;
}
