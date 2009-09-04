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
#include "subprocessor.h"
using namespace Geddei;

#include "spectrum.h"
using namespace SignalTypes;

class SimpleSplit: public SubProcessor
{
	int theWidth;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);

public:
	SimpleSplit();
};

SimpleSplit::SimpleSplit(): SubProcessor("SimpleSplit", Out)
{
}

void SimpleSplit::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	for(uint i = 0; i < multiplicity(); i++)
		out[i].copyFrom(in[0].mid(i * theWidth, theWidth));
}

const bool SimpleSplit::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if(!inTypes[0].isA<Spectrum>()) return false;
	Spectrum s = inTypes[0].asA<Spectrum>();
	theWidth = s.scope() / multiplicity();
	s.setScope(theWidth);
	outTypes = s;
	return true;
}

void SimpleSplit::initFromProperties(const Properties &)
{
	setupIO(1, Undefined, 1, 1, 1);
}

EXPORT_CLASS(SimpleSplit, 0,1,0, SubProcessor);
