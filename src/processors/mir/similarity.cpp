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
using namespace std;

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "processor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
#include "matrix.h"
using namespace SignalTypes;

class Similarity : public Processor
{
	uint theSize, theStep;

	static inline float cosineDistance(const float *x, const float *y, uint bandWidth)
	{
		double ret = 0., mx = 0., my = 0.;

		for (uint i = 0; i < bandWidth; i++)
		{	ret += x[i] * y[i];
			mx += x[i] * x[i];
			my += y[i] * y[i];
		}
		float div = sqrt(mx) * sqrt(my);
		if (!isnan(div)) if (!isnan(ret / div))
			return ret / div;
		return 0;
	}

protected:
	virtual void processor();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void specifyInputSpace(Q3ValueVector<uint> &samples) { samples[0] = theSize; }
	virtual void specifyOutputSpace(Q3ValueVector<uint> &samples) { samples[0] = 1; }
public:
	Similarity() : Processor("Similarity") {}
};

void Similarity::processor()
{
	uint bandWidth = input(0).type().asA<Spectrum>().size();

	float *theMatrix = new float[theSize * theSize];

	// start off by invalidating the whole lot.
	uint step = theSize;

	while (true)
	{
		if (step < theSize)
			memmove(theMatrix, theMatrix + (theSize * theStep) + theStep, (theSize * (theSize - theStep) - theStep) * sizeof(float));
		{	const BufferData d0 = input(0).peekSamples(theSize), d1 = input(1).peekSamples(theSize);
			for (uint i = theSize - step; i < theSize; i++)
			{	const float *d0i = d0.sample(i).readPointer(), *d1i = d1.sample(i).readPointer();
				for (uint j = 0; j < (i + 1); j++)
				{	theMatrix[j*theSize + i] = cosineDistance(d0.sample(j).readPointer(), d1i, bandWidth);
					theMatrix[i*theSize + j] = cosineDistance(d0i, d1.sample(j).readPointer(), bandWidth);
				}
			}
		}
		input(0).readSamples(step);
		input(1).readSamples(step);
		BufferData out = output(0).makeScratchSample();
		out.copyFrom(theMatrix);
		output(0) << out;
		step = theStep;
	}
}

bool Similarity::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<Spectrum>()) return false;
	outTypes[0] = SquareMatrix(theSize, inTypes[0].frequency() / theStep, inTypes[0].frequency());
	return true;
}

void Similarity::initFromProperties(const Properties &properties)
{
	theSize = properties.get("Size").toInt();
	theStep = properties.get("Step").toInt();
	setupIO(2, 1);
}

PropertiesInfo Similarity::specifyProperties() const
{
	return PropertiesInfo("Size", 64, "The size of the block (in samples) from which to create a similarity matrix.")
						 ("Step", 16, "The number of samples between consequent sampling blocks.");
}

EXPORT_CLASS(Similarity, 0,1,0, Processor);
