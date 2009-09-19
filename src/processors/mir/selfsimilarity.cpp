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

#include <stdint.h>

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
#include "value.h"
#include "matrix.h"
using namespace SignalTypes;

static inline float cosineDistance(const float *x, const float *y, uint bandWidth)
{
	double ret = 0., mx = 0., my = 0.;

	for (uint i = 0; i < bandWidth; i++)
	{
		ret += abs(x[i] * y[i]);
		mx += x[i] * x[i];
		my += y[i] * y[i];
	}
	float div = sqrt(mx) * sqrt(my);
	if (!isFinite(div) || !isFinite(ret))
		qDebug() << "BAD NUMBER: " << ret << div << mx << my;
	if (isFinite(ret) && isFinite(div) && div > 0)
		return 1.f - ret / div;
	return 1;
}

static inline float magnitudeDistance(const float *x, const float *y, uint bandWidth)
{
	float ret = 0.f;
	for (uint i = 0; i < bandWidth; i++)
		ret += (x[i] - y[i]) * (x[i] - y[i]);
	return ret > 0 ? sqrt(ret / bandWidth) : 1.f;
}

static inline float magnitudeGreater(const float *x, const float *y, uint bandWidth)
{
	float xmag = 0.f;
	float ymag = 0.f;
	for (uint i = 0; i < bandWidth; i++)
		xmag += x[i] * x[i], ymag += y[i] * y[i];
	return sqrt(xmag / (ymag + 1));
}


class Distance: public SubProcessor
{
	uint m_scope;
	float(*m_distance)(const float *, const float *, const uint);

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const
	{
		float const* p = in[0].readPointer();
		out[0][0] = m_distance(p, p + m_scope, m_scope);
	}
	virtual void initFromProperties(Properties const&) { setupIO(1, 1, 2, 1, 1); }
	virtual void updateFromProperties(Properties const& _p);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
	{
		m_scope = inTypes[0].scope();
		outTypes[0] = Value(inTypes[0].frequency());
		return true;
	}
	virtual PropertiesInfo specifyProperties() const;

public:
	Distance() : SubProcessor("Distance") {}
};

void Distance::updateFromProperties(const Properties &properties)
{
	if (properties["Distance Function"].toInt() == 0)
		m_distance = cosineDistance;
	else if (properties["Distance Function"].toInt() == 1)
		m_distance = magnitudeDistance;
	else if (properties["Distance Function"].toInt() == 2)
		m_distance = magnitudeGreater;
	else
		qFatal("*** ERROR: Invalid distance function index given.");
}

PropertiesInfo Distance::specifyProperties() const
{
	return PropertiesInfo("Distance Function", 0, "The distance function to be used when calculating the similarity. { 0: Cosine; 1: Magnitude ; 2: MagnitudeGreater }");
}

EXPORT_CLASS(Distance, 0,2,0, SubProcessor);

class SelfSimilarity : public SubProcessor
{
	uint theSize, theStep, theBandWidth;
	mutable QVector<float> theMatrix;
	float(*theDistance)(const float *, const float *, const uint);

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
	uint step;
	if (theMatrix.isEmpty())
	{
		theMatrix.resize(theSize * theSize);
		step = theSize;
	}
	else
		step = 1;

	for (uint c = 0; c < chunks; c++)
	{
		if (step < theSize)
			memmove(theMatrix.data(), theMatrix.data() + (theSize + 1) * step, (theSize * (theSize - step) - step) * sizeof(float));
		for (uint p = 0; p < step; p++)
		{
			uint li = theSize * (theSize - p) - 1 - p;
			uint ls = theSize - 1 - p + c;
			BufferData latestBD = in[0].sample(ls);
			float const* latest = latestBD.readPointer();
			for (uint i = 1; i < theSize - p; i++)
			{
				theMatrix[li - i] = theDistance(in[0].sample(ls - i).readPointer(), latest, theBandWidth);
				theMatrix[li - i * theSize] = theDistance(latest, in[0].sample(ls - i).readPointer(), theBandWidth);
			}
			theMatrix[li] = 0;
		}
		out[0].sample(0).copyFrom(theMatrix.constData());
		step = 1;
	}
	return;
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
	theMatrix.clear();
	updateFromProperties(properties);
	setupIO(1, 1, theSize, theStep, 1);
}

void SelfSimilarity::updateFromProperties(const Properties &properties)
{
	if (properties["Distance Function"].toInt() == 0)
		theDistance = cosineDistance;
	else if (properties["Distance Function"].toInt() == 1)
		theDistance = magnitudeDistance;
	else if (properties["Distance Function"].toInt() == 2)
		theDistance = magnitudeGreater;
	else
		qFatal("*** ERROR: Invalid distance function index given.");
}

PropertiesInfo SelfSimilarity::specifyProperties() const
{
	return PropertiesInfo("Size", 64, "The size of the block (in samples) from which to create a self-similarity matrix.")
						 ("Distance Function", 0, "The distance function to be used when calculating the similarity. { 0: Cosine; 1: Magnitude ; 2: MagnitudeGreater }");
}

EXPORT_CLASS(SelfSimilarity, 0,2,0, SubProcessor);
