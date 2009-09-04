/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
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
using namespace Geddei;

#include "wave.h"
using namespace SignalTypes;

class WaveGen: public Processor
{
	enum WaveType { Sine, Square, Triangular } theWaveType;
	float theFrequency, theRate;
	int theChunk, theChunksPerPlunge, theStopAfterChunks;

	virtual void processor();
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);
	virtual PropertiesInfo specifyProperties() const;
	virtual void specifyOutputSpace(Q3ValueVector<uint> &samples);
public:
	WaveGen() : Processor("WaveGen", NotMulti, Guarded) {}
};

void WaveGen::processor()
{
	float phase = 0;
	int chunksLeft = 0, hadChunks = 0;
	while(!theStopAfterChunks || theStopAfterChunks > hadChunks)
	{
		BufferData d = output(0).makeScratchSamples(theChunk);
		for(int i = 0; i < theChunk; i++)
			d[i] = sin(2.0 * 3.14159265898 * (float(i) * theFrequency / theRate + phase));
		output(0) << d;
		phase += float(theChunk) * theFrequency / theRate;
		if(phase > 1.0) phase -= 1.0;
		if(theChunksPerPlunge && ++chunksLeft == theChunksPerPlunge)
		{	plunge();
			chunksLeft = 0;
		}
		hadChunks++;
	}
}

const bool WaveGen::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes)
{
	outTypes[0] = Wave(theRate);
	return true;
}

void WaveGen::specifyOutputSpace(Q3ValueVector<uint> &samples)
{
	samples[0] = theChunk;
}

void WaveGen::initFromProperties(const Properties &properties)
{
	theChunksPerPlunge = 0;
	theFrequency = properties.get("Frequency").toDouble();
	theRate = properties.get("Sample Rate").toDouble();
	double pps = properties.get("PlungersPerSecond").toDouble();
	int spc = properties.get("SamplesPerChunk").toInt();
	if(pps > 0. && properties.get("ChunksPerPlunger").toInt())
	{	theChunksPerPlunge = properties.get("ChunksPerPlunger").toInt();
		theChunk = int(theRate / (pps * theChunksPerPlunge));
	}
	else if(spc)
		theChunk = spc;
	else
		theChunk = int(theRate / 2.);
	theStopAfterChunks = int(floor(properties["Stop After"].toDouble() * theRate / theChunk));
	theWaveType = Sine;
	setupIO(0, 1);
}

PropertiesInfo WaveGen::specifyProperties() const
{
	return PropertiesInfo("Frequency", 440., "Frequency of the wave to generate.")
	                     ("Sample Rate", 22050., "Rate at which samples of the wave are produced.")
	                     ("ChunksPerPlunger", 1, "Number of output chunks produced between each plunger. This is mutually exclusive with SamplesPerChunk.")
	                     ("PlungersPerSecond", 1., "Number of plungers produced per second. This is mutually exclusive with SamplesPerChunk.")
	                     ("SamplesPerChunk", 0, "Number of samples in each chunk of output data. This is mutually exclusive with ChunksPerPlunger and PlungersPerSecond.")
	                     ("Stop After", 0., "Number of plungers before this processors task has ended. If zero, continue indefinately. ");
}

EXPORT_CLASS(WaveGen, 0,1,0, Processor);
