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

#include "matrix.h"
using namespace SignalTypes;

class CrossSimilarity: public SubProcessor
{
	int theScope, theCount;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &/*properties*/) { setupIO(Undefined, 1, 1, 1, 1); }

public:
	CrossSimilarity(): SubProcessor("CrossSimilarity", In) {}
};

void CrossSimilarity::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	for(int i = 0; i < theCount; i++)
		for(int ii = 0; ii < theCount; ii++)
		{	out[0][i * theCount + ii] = 0.;
			for(int j = 0; j < theScope; j++)
				out[0][i * theCount + ii] += (1. - abs(in[i][j] - in[ii][j])) / float(theScope);
		}
}

const bool CrossSimilarity::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	theScope = inTypes[0].scope();
	theCount = multiplicity();
	outTypes[0] = SquareMatrix(theCount, inTypes[0].frequency());
	return true;
}

EXPORT_CLASS(CrossSimilarity, 0,1,0, SubProcessor);
