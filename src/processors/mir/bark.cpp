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

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
using namespace SignalTypes;

//template<class C> inline C max(C a, C b) { return a > b ? a : b; }

class Bark : public SubProcessor
{
	uint theBins[24], bandWidth, theUsedBins;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);

public:
	Bark() : SubProcessor("Bark") {}
};

static const uint barkBands[24] = { 100, 200, 300, 400, 510, 630, 770, 920, 1080, 1270, 1480, 1720, 2000, 2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500 };

void Bark::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for(uint i = 0; i < theUsedBins; i++)
		outs[0][i] = 0.0;
	uint bandCount = 0;
	for(uint i = 0, o = 0; i < bandWidth; i++)
	{
		assert(o < theUsedBins);
		if(i == theBins[o])
		{	
			outs[0][o] /= max(float(bandCount), 1.f);
			bandCount = 0;
			++o;
			if(o == theUsedBins) break;
		}
		outs[0][o] += ins[0][i];
		bandCount++;
	}
//	qDebug("Bark: Max in = %f, max out = %f", ins[0][maxi], outs[0][maxo]);
}

const bool Bark::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	// TODO: should only take a "stepped spectrum" in.
	if(!inTypes[0].isA<Spectrum>()) return false;
	const Spectrum &in = inTypes[0].asA<Spectrum>();
	outTypes[0] = Spectrum(theUsedBins, in.frequency(), 1);

	bandWidth = in.size();
	uint o = 0;
	float inc = in.step(), f = 0.;
	for(uint i = 0; i < 24; i++)
	{	while(f < barkBands[i])
		{	f += inc;
			o++;
		}
		theBins[i] = o;
	}
	return true;
}

void Bark::initFromProperties(const Properties &)
{
	theUsedBins = 20;
	setupIO(1, 1, 1, 1, 1);
}

EXPORT_CLASS(Bark, 0,2,0, SubProcessor);
