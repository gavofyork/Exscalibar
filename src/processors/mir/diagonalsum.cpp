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
#include "subprocessor.h"
using namespace Geddei;

#include "spectrum.h"
#include "matrix.h"
using namespace SignalTypes;

class DiagonalSum : public SubProcessor
{
	uint theSize, theBandwidth;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &/*properties*/) { setupIO(1, 1, 1, 1, 1); }
public:
	DiagonalSum() : SubProcessor("DiagonalSum") {}
};

bool DiagonalSum::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<SquareMatrix>()) return false;
	theSize = inTypes[0].asA<SquareMatrix>().size();
	theBandwidth = theSize / 2;
	outTypes[0] = Spectrum(theBandwidth, inTypes[0].frequency(), inTypes[0].asA<SquareMatrix>().pitch());
	return true;
}

void DiagonalSum::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	for (uint offset = 0; offset < theBandwidth; offset++)
	{	out[0][offset] = 0;
		for (uint xy = 0; xy < theSize; xy++)
			out[0][offset] += in[0][(xy + (xy*theSize) + offset) % (theSize * theSize)];
		out[0][offset] /= theSize;
	}
}

EXPORT_CLASS(DiagonalSum, 0,2,0, SubProcessor);
