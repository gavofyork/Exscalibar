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

class Checkerboard : public CoProcessor
{
	uint theSize;
	float *theBoard;
	enum { Check = 0, Positive, TopRight };
	uint theSign;
	bool theTaper;
	float m_max;

	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);

public:
	Checkerboard();
};

Checkerboard::Checkerboard(): CoProcessor("Checkerboard")
{
	theBoard = 0;
}

inline float sqr(float a) { return a * a; }

bool Checkerboard::processorStarted()
{
	float halfSize = theSize / 2;
	theBoard = new float[theSize * theSize];
	m_max = 0;
	for (uint y = 0; y < theSize; y++)
		for (uint x = 0; x < theSize; x++)
		{	float xdist = (float(x) - halfSize) / (float(theSize) - halfSize), ydist = (float(y) - halfSize) / (float(theSize) - halfSize);
			float sign = theSign == Check ? (xdist * ydist < 0 ? -1 : 1) : theSign == TopRight ? xdist > 0 && ydist > 0 ? 1 : -1 : 1;
			float distance = sqrt(sqr(xdist) + sqr(ydist)) / sqrt(2.0);
			float a = 1.0, b = 1.0, c = 1.0;
			theBoard[x*theSize + y] = sign * (theTaper ? a * exp(-sqr(distance-b) / sqr(c)) : 1.f);
			if (sign > 0) m_max += theBoard[x*theSize + y];
		}
	return true;
}

int Checkerboard::process()
{
	const BufferData in = input(0).readSample();
	BufferData out = output(0).makeScratchSamples(1);
	out[0] = 0;
	for (uint i = 0; i < theSize * theSize; i++)
		out[0] += theBoard[i] * in[i];
	out[0] /= m_max;
	output(0) << out;
	return DidWork;
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
	updateFromProperties(p);
	setupIO(1, 1);
}

void Checkerboard::updateFromProperties(const Properties &p)
{
	theSign = p["Sign"].toInt();
	theTaper = p["Taper"].toBool();
}

PropertiesInfo Checkerboard::specifyProperties() const
{
	return PropertiesInfo("Sign", Check, "Type of kernel to build. { 0: Checkerboard; 1: Positive; 2: TopRight }")
							("Taper", true, "Kernel should have a Gaussian taper.");
}

EXPORT_CLASS(Checkerboard, 0,2,0, Processor);
