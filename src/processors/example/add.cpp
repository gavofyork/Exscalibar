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
#include "processor.h"
#include "buffer.h"
using namespace Geddei;

class DLLEXPORT Add: public Processor
{
	int count;
	virtual void processor();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);
public:
	Add() : Processor("Add") {}
};

void Add::processor()
{
	while (true)
	{	input(0).waitForSamples();
		input(1).waitForSamples();
		int samples = min(min(input(0).samplesReady(), input(1).samplesReady()), output(0).maximumScratchSamples());
		// TODO: allow pushing of greater than buffer-size of elements (with foreign BDs)

		const BufferData i0 = input(0).readSamples(samples);
		const BufferData i1 = input(1).readSamples(samples);
		BufferData o = output(0).makeScratchSamples(samples);
		for (uint i = 0; i < o.elements(); i++)
			o[i] = i0[i] + i1[i];
		output(0).push(o);
	}
}

bool Add::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes.allSame()) return false;
	outTypes[0] = inTypes[0];
	return true;
}

void Add::initFromProperties(const Properties &)
{
	setupIO(2, 1);
}

EXPORT_CLASS(Add, 0,1,0, Processor);
