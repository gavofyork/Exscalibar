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
using namespace std;

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
#include "matrix.h"
using namespace SignalTypes;

class SelfSimilarity : public SubProcessor
{
	uint theSize, theStep, theBandWidth;
	mutable QVector<float> theMatrix;
	float(*theDistance)(const float *, const float *, const uint);

	static inline float cosineDistance(const float *x, const float *y, uint bandWidth)
	{
		double ret = 0., mx = 0., my = 0.;

		for (uint i = 0; i < bandWidth; i++)
		{	ret += abs(x[i] * y[i]);
			mx += x[i] * x[i];
			my += y[i] * y[i];
		}
		float div = sqrt(mx) * sqrt(my);
		if (!isnan(div)) if (!isnan(ret / div))
			return ret / div;
		return 0;
	}

	static inline float magnitudeDistance(const float *x, const float *y, uint bandWidth)
	{
		float ret = 0.f;
		for (uint i = 0; i < bandWidth; i++)
			ret += (x[i] - y[i]) * (x[i] - y[i]);
		if (!isnan(ret))
			return ret > 0 ? 1.f / (1.f + sqrt(ret)) : 1.f;
		return 0;
	}

	virtual void processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks);
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;

public:
	SelfSimilarity() : SubProcessor("SelfSimilarity") {}
};

void SelfSimilarity::processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks)
{
	uint step = theStep;	// theSize to invalidate the lot.

	for (uint c = 0; c < chunks; c++)
	{	if (step < theSize)
			memmove(theMatrix.data(), theMatrix.data() + (theSize * theStep) + theStep, (theSize * (theSize - theStep) - theStep) * sizeof(float));
		for (uint i = theSize - step; i < theSize; i++)
		{	const float *d0i = in[0].sample(c * theStep + i).readPointer();
			for (uint j = 0; j < (i + 1); j++)
				theMatrix[j*theSize + i] = theMatrix[i*theSize + j] = theDistance(in[0].sample(c * theStep + j).readPointer(), d0i, theBandWidth);
		}
		out[0].sample(c).copyFrom(theMatrix.constData());
		step = theStep;
	}
}

bool SelfSimilarity::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<Spectrum>()) return false;
	outTypes[0] = SquareMatrix(theSize, inTypes[0].frequency() / theStep, inTypes[0].frequency());
	theBandWidth = inTypes[0].scope();
	return true;
}

void SelfSimilarity::initFromProperties(const Properties &properties)
{
	theSize = properties.get("Size").toInt();
	theStep = properties.get("Step").toInt();
	theMatrix.resize(theSize * theSize);
	updateFromProperties(properties);
	setupIO(1, 1, theSize, theStep, 1);
}

void SelfSimilarity::updateFromProperties(const Properties &properties)
{
	if (properties["Distance Function"].toInt() == 0)
		theDistance = cosineDistance;
	else if (properties["Distance Function"].toInt() == 1)
		theDistance = magnitudeDistance;
	else
		qFatal("*** ERROR: Invalid distance function index given.");
}

PropertiesInfo SelfSimilarity::specifyProperties() const
{
	return PropertiesInfo("Size", 64, "The size of the block (in samples) from which to create a self-similarity matrix.")
						 ("Step", 16, "The number of samples between consequent sampling blocks.")
						 ("Distance Function", 0, "The distance function to be used when calculating the similarity. { 0: Cosine; 1: Magnitude }");
}

EXPORT_CLASS(SelfSimilarity, 0,2,0, SubProcessor);
