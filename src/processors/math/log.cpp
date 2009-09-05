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

class Log : public SubProcessor
{
	uint theSize;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &/*properties*/) { setupIO(1, 1, 1, 1, 1); }

public:
	Log() : SubProcessor("Log") {}
};

void Log::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for (uint i = 0; i < theSize; i++)
		outs[0][i] = log(ins[0][i]);
}

bool Log::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	outTypes[0] = inTypes[0];
	theSize = inTypes[0].scope();
	return true;
}

EXPORT_CLASS(Log, 0,2,0, SubProcessor);
