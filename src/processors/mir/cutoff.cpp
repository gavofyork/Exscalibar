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

#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
using namespace SignalTypes;

class CutOff : public SubProcessor
{
	float theFreqFrom, theFreqTo;
	uint theFrom, theTo, theSize;

	virtual void processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);

	void processSamples(const BufferData &in, BufferData &out, uint chunks);

public:
	CutOff() : SubProcessor("CutOff") {}
};

void CutOff::processChunks(const BufferDatas &ins, BufferDatas &outs, uint chunks) const
{
	for (uint i = 0; i < chunks; i++)
		outs[0].sample(i).copyFrom(ins[0].mid(i * theSize + theFrom, theTo - theFrom));
}

bool CutOff::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<Spectrum>()) return false;
	const Spectrum &s = inTypes[0].asA<Spectrum>();

	float base = 0;
	theSize = s.size();
	theFrom = ::min(s.size(), (uint)::max(0, (int)round((theFreqFrom - base) / s.step())));
	theTo = ::min(s.size(), (uint)::max(0, (int)round((theFreqTo - base) / s.step())));

	outTypes[0] = Spectrum(theTo - theFrom, s.frequency(), s.step());
	return true;
}

void CutOff::initFromProperties(const Properties &properties)
{
	theFreqFrom = properties.get("Frequency From").toDouble();
	theFreqTo = properties.get("Frequency To").toDouble();
	setupIO(1, 1, 1, 1, 1);
}

PropertiesInfo CutOff::specifyProperties() const
{
	return PropertiesInfo("Frequency From", 0., "The lower bound of bands allowed to pass.")
	                     ("Frequency To", 4000., "The uppoer bound of bands allowed to pass.");
}

EXPORT_CLASS(CutOff, 0,1,0, SubProcessor);
