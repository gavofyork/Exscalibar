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

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

class Mean : public SubProcessor
{
	uint theSize;

	virtual void processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &) { setupIO(Undefined, 1, 1, 1, 1); }

public:
	Mean() : SubProcessor("Mean", In) {}
};

void Mean::processChunks(const BufferDatas &ins, BufferDatas &outs, uint chunks) const
{
	for (uint c = 0; c < chunks; c++)
		for (uint j = 0; j < theSize; j++)
			outs[0](c, j) = 0.;
	for (uint i = 0; i < multiplicity(); i++)
		for (uint c = 0; c < chunks; c++)
			for (uint j = 0; j < theSize; j++)
				outs[0](c, j) += ins[i](c, j) / float(multiplicity());
}

bool Mean::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	outTypes[0] = inTypes[0];
	theSize = inTypes[0].scope();
	return true;
}

EXPORT_CLASS(Mean, 0,3,0, SubProcessor);
