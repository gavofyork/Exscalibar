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

#include "geddei.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace SignalTypes;

class Magnitude: public SubProcessor
{
	virtual void initFromProperties(const Properties &);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &in, SignalTypeRefs &out);
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;

public:
	Magnitude();
};

Magnitude::Magnitude(): SubProcessor("Magnitude")
{
}

void Magnitude::initFromProperties(const Properties &)
{
	setupIO(1, 1, 1, 1, 1);
}

bool Magnitude::verifyAndSpecifyTypes(const SignalTypeRefs &in, SignalTypeRefs &out)
{
	if (!in[0].isA<Spectrum>()) return false;
	out = Value(in[0].frequency());
	return true;
}

void Magnitude::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	out[0][0] = 0.;
	for (uint i = 0; i < in[0].elements(); i++)
		out[0][0] += in[0][i] * in[0][i];
	out[0][0] = sqrt(out[0][0]);
}

EXPORT_CLASS(Magnitude, 0,1,0, SubProcessor);
