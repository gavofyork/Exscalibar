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

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "processor.h"
#include "buffer.h"
using namespace Geddei;

class Split : public Processor
{
	virtual void processor();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);
public:
	Split() : Processor("Split", Out, Guarded) {}
};

void Split::processor()
{
	while (thereIsInputForProcessing(1))
	{	// Get our maximum amount of samples to transfer that everyone can agree on
		uint m = input(0).samplesReady();
		for (uint i = 0; i < numOutputs(); i++) m = min(m, output(i).maximumScratchSamples());
		// Read them
		BufferData d = input(0).readSamples(m);
		// Push them
		// TODO: allow pushing of greater than buffer-size of elements (with foreign BDs), so no maxSS() call neccessary
		for (uint i = 0; i < numOutputs(); i++)
			output(i).push(d);
	}
}

bool Split::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	outTypes = inTypes[0];
	return true;
}

void Split::initFromProperties(const Properties &)
{
	setupIO(1, Undefined);
}

EXPORT_CLASS(Split, 0,1,0, Processor);
