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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
using namespace SignalTypes;

#ifdef HAVE_FFTW

#include <fftw3.h>

class Cepstrum : public SubProcessor
{
	bool theOptimise;
	uint theSize, theType;
	fftwf_plan thePlan;
	float *theIn, *theOut;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo("Optimise", true, "True if time is taken to optimise the calculation.")
		                     ("Type", 0, "The type of cepstrum to calculate, according to libfftw.");
	}

public:
	Cepstrum() : SubProcessor("Cepstrum") {}
	~Cepstrum();
};

void Cepstrum::initFromProperties(const Properties &properties)
{
	theOptimise = properties["Optimise"].toBool();
	theType = properties["Type"].toInt();
	setupIO(1, 1, 1, 1, 1);
	theIn = 0;
	theOut = 0;
	thePlan = 0;
}

Cepstrum::~Cepstrum()
{
	if(theIn) fftwf_free(theIn);
	if(theOut) fftwf_free(theOut);
	if(thePlan) fftwf_destroy_plan(thePlan);
}

bool Cepstrum::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if(!inTypes[0].isA<Spectrum>()) return false;
	const Spectrum &s = inTypes[0].asA<Spectrum>();

	theSize = s.size();
	if(theIn) fftwf_free(theIn);
	if(theOut) fftwf_free(theOut);
	if(thePlan) fftwf_destroy_plan(thePlan);
	theIn = (float *)fftwf_malloc(sizeof(float) * theSize);
	theOut = (float *)fftwf_malloc(sizeof(float) * theSize);
	thePlan = fftwf_plan_r2r_1d(theSize, theIn, theOut, theType == 0 ? FFTW_REDFT00 : theType == 1 ? FFTW_REDFT10 : theType == 2 ? FFTW_REDFT01 : FFTW_REDFT11, theOptimise ? FFTW_MEASURE : FFTW_ESTIMATE);

	outTypes[0] = Spectrum(s.size() / 2, s.frequency(), s.step());
	return true;
}

void Cepstrum::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
//	qDebug("PC: %f, %f, %f...", ins[0][0], ins[0][1], ins[0][2]);
	ins[0].copyTo(theIn);
	fftwf_execute(thePlan);
	for(uint i = 0; i < theSize / 2; i++)
		theOut[i] /= theSize;
	outs[0].copyFrom(theOut);
//	qDebug("PCx: %f, %f, %f...", outs[0][0], outs[0][1], outs[0][2]);
//	qDebug("*");
}

#else

class Cepstrum : public SubProcessor
{
	uint theSize;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);

public:
	Cepstrum() : SubProcessor("Cepstrum") {}
};

void Cepstrum::initFromProperties(const Properties &)
{
	setupIO(1, 1, 1, 1, 1);
}

void Cepstrum::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for(uint i = 0; i < theSize / 2; i++)
	{	float t = 0.0;
		for(uint j = 0; j < theSize; j++)
			t += ins[0][j] * cos(M_PI / float(theSize / 2) * (i + 1.0) * (j + 0.5));
		outs[0][i] = t / float(theSize);
	}
}

bool Cepstrum::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if(!inTypes[0].isA<Spectrum>()) return false;
	const Spectrum &s = inTypes[0].asA<Spectrum>();

	theSize = s.scope();
	outTypes[0] = Spectrum(s.size() / 2, s.frequency(), s.step());
	return true;
}

#endif

EXPORT_CLASS(Cepstrum, 0,2,0, SubProcessor);
