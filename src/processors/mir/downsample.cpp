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

class DownSample : public SubProcessor
{
	uint theCount, theScope, theStep;
	double theOverlap, thePeriod;
	enum { Mean = 0, Max, Min };
	uint theConsolidate;

	virtual void processChunks(const BufferDatas &in, BufferDatas &out, const uint chunks) const;
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);

public:
	DownSample() : SubProcessor("DownSample") {}
};

void DownSample::processChunks(const BufferDatas &ins, BufferDatas &outs, const uint chunks) const
{
	if(theCount <= 1)
		if(theScope > 1)
			for(uint i = 0; i < chunks; i++)
				outs[0].sample(i).copyFrom(ins[0].sample(i * theStep));
		else
			for(uint i = 0; i < chunks; i++)
				outs[0][i] = ins[0][i * theStep];
	else
	{	for(uint j = 0; j < chunks; j++)
			for(uint i = 0; i < theScope; i++)
				outs[0](j, i) = 0;
		for(uint j = 0; j < chunks; j++)
		{	for(uint i = 0; i < theCount; i++)
			{	BufferData d = ins[0].sample(i + j*theStep);
				const float *inSample = d.readPointer();
				if(theConsolidate == Mean)
					for(uint k = 0; k < theScope; k++)
						outs[0](j, k) += inSample[k];
				else if(theConsolidate == Max)
					for(uint k = 0; k < theScope; k++)
						if(outs[0](j, k) < inSample[k] || !k) outs[0](j, k) = inSample[k];
				else if(theConsolidate == Min)
					for(uint k = 0; k < theScope; k++)
						if(outs[0](j, k) > inSample[k] || !k) outs[0](j, k) = inSample[k];
			}
		}
		if(theConsolidate == Mean)
			for(uint j = 0; j < chunks; j++)
				for(uint i = 0; i < theScope; i++)
					outs[0](j, i) /= theCount;
	}
}

const bool DownSample::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	theScope = inTypes[0].scope();
	outTypes = inTypes[0];
	
	if(thePeriod != 0.0) theCount = uint(thePeriod * inTypes[0].frequency());
	if(theOverlap != 0.0) theStep = uint(theOverlap * inTypes[0].frequency());
	
	outTypes[0].asA<SignalType>().setFrequency(inTypes[0].frequency() / (float)theStep);
	setupSamplesIO(max(theCount, theStep), theStep, 1);
	return true;
}

void DownSample::initFromProperties(const Properties &properties)
{
	// TODO: setupDimensions according to properties if neccessary.
	theCount = properties.get("Spread").toInt();
	theStep = properties.get("Divisor").toInt();
	thePeriod = properties.get("Period").toDouble();
	theOverlap = properties.get("Overlap").toDouble();
	theConsolidate = properties["Consolidate"].toInt();
	setupIO(1, 1);
}

PropertiesInfo DownSample::specifyProperties() const
{
	return PropertiesInfo("Spread", 1, "The number of samples from which to calculate the value. A larger number results in a smoother output, a lower number is faster. This should be >= 1 and <= Divisor.")
	                     ("Divisor", 2, "The sampling frequency divisor.")
						 ("Period", 0.f, "The period of time to derive each sample from.")
						 ("Overlap", 0.f, "The period of overlap to leave between successive samples.")
	                     ("Consolidate", 0, "The method to determine the final sample when Spread > 1. { 0: Average; 1: Max; 2: Min }");
}

EXPORT_CLASS(DownSample, 0,3,0, SubProcessor);
