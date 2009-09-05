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

#include <cstdlib>
#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "processor.h"
#include "buffer.h"
using namespace Geddei;

#include "value.h"
#include "matrix.h"
using namespace SignalTypes;

class Checkerboard : public Processor
{
	uint theSize;
	float *theBoard;
	enum { Check = 0, Positive };
	uint theSign;

	virtual void processor();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);

public:
	Checkerboard();
};

Checkerboard::Checkerboard() : Processor("Checkerboard")
{
	theBoard = 0;
}

inline float sqr(float a) { return a * a; }

void Checkerboard::processor()
{
	float halfSize = theSize / 2;
	theBoard = new float[theSize * theSize];
	float max = 0;
	for (uint y = 0; y < theSize; y++)
		for (uint x = 0; x < theSize; x++)
		{	float xdist = (float(x) - halfSize) / (float(theSize) - halfSize), ydist = (float(y) - halfSize) / (float(theSize) - halfSize);
			float sign = theSign == Check ? (xdist * ydist < 0 ? -1 : 1) : 1;
			float distance = sqrt(sqr(xdist) + sqr(ydist)) / sqrt(2.0);
			float a = 1.0, b = 1.0, c = 1.0;
			theBoard[x*theSize + y] = sign * a * exp(-sqr(distance-b) / sqr(c));
			if (sign > 0) max += theBoard[x*theSize + y];
		}
	while (true)
	{
		const BufferData in = input(0).readSample();
		BufferData out = output(0).makeScratchSamples(1);
		out[0] = 0;
		for (uint i = 0; i < theSize * theSize; i++)
			out[0] += theBoard[i] * in[i];
		out[0] /= max;
		output(0) << out;
	}
}

void Checkerboard::processorStopped()
{
	delete [] theBoard;
}

bool Checkerboard::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<SquareMatrix>()) return false;
	theSize = inTypes[0].asA<SquareMatrix>().size();
	outTypes[0] = Value(inTypes[0].frequency());
	return true;
}

void Checkerboard::initFromProperties(const Properties &p)
{
	theSign = p["Sign"].toInt();
	setupIO(1, 1);
}

PropertiesInfo Checkerboard::specifyProperties() const
{
	return PropertiesInfo("Sign", Check, "Type of kernel to build. { 0: Checkerboard; 1: Positive }");
}

EXPORT_CLASS(Checkerboard, 0,2,0, Processor);
