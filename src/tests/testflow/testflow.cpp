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

#include <iostream>

#include "geddei.h"
using namespace Geddei;

#include "wave.h"
using namespace SignalTypes;

class PlungeGenerator : public Processor
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
				plunge();
			}
		}
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes) { outTypes[0] = Wave(22050); return true; }
	virtual void initFromProperties(const Properties &) { setupIO(0, 1); }

public:
	PlungeGenerator() : Processor("PlungeGenerator") {}
};

class PlungeDetector : public Processor
{
	void processor()
	{
		while (true)
		{
			BufferData s = input(0).readSamples(5);
			output(0) << s;
		}
	}

	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { if (!inTypes[0].isA<Wave>()) return false; outTypes[0] = inTypes[0]; return true; }
	virtual void initFromProperties(const Properties &) { setupIO(1, 1); }
	virtual void receivedPlunger()
	{
		std::cout << "Detector: Received plunger!" << std::endl;
	}
public:
	PlungeDetector() : Processor("PlungeDetector") {}
};

QFastWaitCondition theCondition;
QMutex theMutex;
int theCount;

class PlungeEater : public Processor
{
	void processor()
	{
		while (true)
		{
			input(0).readSamples(1);
		}
	}

	virtual void initFromProperties(const Properties &) { setupIO(1, 0); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &) { return true; }
	virtual void receivedPlunger()
	{
		std::cout << "Eater: Received plunger!" << std::endl;
		QMutexLocker lock(&theMutex);
		theCount++;
		theCondition.wakeAll();
	}
public:
	PlungeEater() : Processor("PlungeEater") {}
};

int main()
{
	Processor	*O = new PlungeGenerator,
				*D = new PlungeDetector,
				*I = new PlungeEater;

	ProcessorGroup objects;
	O->init("O", objects);
	D->init("D", objects);
	I->init("I", objects);

	std::cout << "Connecting..." << std::endl;
	(*O)[0] >>= (*D)[0].setSize(10);
	(*D)[0] >>= (*I)[0].setSize(10);

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
	objects.disconnectAll();
}
