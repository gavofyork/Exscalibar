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

class Fan: public SubProcessor
{
	virtual void initFromProperties (const Properties &);
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const;

public:
	Fan();
};

Fan::Fan(): SubProcessor("Fan", Out)
{
}

void Fan::initFromProperties(const Properties &)
{
	setupIO(1, Undefined, 1, 1, 1);
}

const bool Fan::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if(!inTypes[0].isA<Spectrum>()) return false;
	for(uint i = 0; i < outTypes.count(); i++)
		outTypes[i] = Spectrum(inTypes[0].scope() / outTypes.count(), inTypes[0].frequency());
	return true;
}

void Fan::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for(uint i = 0, k = 0; i < outs.count(); i++)
		for(uint j = 0; j < outs[i].elements(); j++,k++)
			outs[i][j] = ins[0][k];
}

EXPORT_CLASS(Fan, 0,1,0, SubProcessor);

/*
class CustomSegment: public SubProcessor
{
	QValueVector<uint> theBands;
	virtual PropertiesInfo specifyProperties() const { return PropertiesInfo("Band0", 8, "")("Band1", 8, "")("Band2", 8, ""); }
	virtual void initFromProperties (const Properties &p) { setupIO(1, 3, 1, 1, 1); theBands.resize(3); theBands[0] = p["Band0"].toInt(); theBands[1] = p["Band1"].toInt(); theBands[2] = p["Band2"].toInt(); }
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
	{
		if(!inTypes[0].isA<Spectrum>()) return false;
		for(uint i = 0; i < theBands.count(); i++)
			outTypes[i] = Spectrum(theBands[i], inTypes[0].frequency());
		return true;
	}
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const
	{
		uint from = 0;
		for(uint i = 0; i < theBands.count(); i++)
		{
			for(uint j = 0; j < theBands[i]; j++)
				outs[i][j] = ins[0][from + j];
			from += theBands[i];
		}
	}
public:
	CustomSegment(): SubProcessor("CustomSegment") {}
};
*/
