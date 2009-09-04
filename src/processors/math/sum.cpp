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

#include <q3valuevector.h>
#include "qfactoryexporter.h"

#include "geddei.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace SignalTypes;

class Sum: public SubProcessor
{
	virtual void initFromProperties (const Properties &);
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const;
public:
	Sum();
};

Sum::Sum(): SubProcessor("Sum")
{
}

void Sum::initFromProperties (const Properties &)
{
	setupIO(1, 1, 1, 1, 1);
}

const bool Sum::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	outTypes = Value(inTypes[0].frequency());
	return true;
}

void Sum::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	outs[0][0] = 0.;
	for(uint i = 0; i < ins[0].elements(); i++)
		outs[0][0] += ins[0][i];
}

EXPORT_CLASS(Sum, 0,1,0, SubProcessor);
