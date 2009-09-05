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

class Normalise: public Processor
{
	Q3ValueVector<float> f;
	uint theOutputSpace, theScope;

	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &in, SignalTypeRefs &out);
	virtual void specifyOutputSpace(Q3ValueVector<uint> &out);
	virtual void processor();
	virtual void receivedPlunger();

public:
	Normalise();
};

Normalise::Normalise(): Processor("Normalise", NotMulti, Guarded)
{
}

PropertiesInfo Normalise::specifyProperties() const
{
	return PropertiesInfo("OutputSpace", 8192, "The minimum amount of output space to insist upon for the output buffer");
}

void Normalise::initFromProperties(const Properties &p)
{
	theOutputSpace = p["OutputSpace"].toUInt();
	setupIO(1, 1);
}

bool Normalise::verifyAndSpecifyTypes(const SignalTypeRefs &in, SignalTypeRefs &out)
{
	theScope = in[0].scope();
	out[0] = in[0];
	return true;
}

void Normalise::specifyOutputSpace(Q3ValueVector<uint> &out)
{
	out[0] = theOutputSpace;
}

void Normalise::processor()
{
	f.clear();
	float last = 0.f;
	while(thereIsInputForProcessing(1))
	{
		const BufferData d = input(0).readSamples();
		for(uint i = 0; i < d.elements(); i++)
		{
			if(isinf(d[i]) || isnan(d[i]))
			{	qDebug("ERROR: Cannot normalise stream with nan/inf in it.");
				f.append(last);
			}
			else
				f.append(last = d[i]);
		}
	}
}

void Normalise::receivedPlunger()
{
	if(!f.size()) return;
	float mini = f[0], maxi = f[0], tu = 0., tb = 0., avgu = 0., avgb = 0., delta, avg = 0.;
	for(uint i = 1; i < f.size(); i++)
		if(f[i] > maxi) maxi = f[i];
		else if(f[i] < mini) mini = f[i];
	int t = 0;
	for(uint i = 0; i < f.size(); i++)
		if(f[i] != mini && f[i] != maxi)
		{ avg += f[i] / float(f.size()); t++; }
	for(uint i = 0; i < f.size(); i++)
		if(f[i] != mini && f[i] != maxi)
			if(f[i] > avg) { avgu += f[i]; tu++; }
			else { avgb += f[i]; tb++; }
	avgu /= float(tu);
	avgb /= float(tb);
	float avguu = 0., avgbb = 0.;
	tu = 0.; tb = 0.;
	for(uint i = 0; i < f.size(); i++)
		if(f[i] != mini && f[i] != maxi)
			if(f[i] > avgu) { avguu += f[i]; tu++; }
			else if(f[i] < avgb) { avgbb += f[i]; tb++; }
	avguu /= float(tu);
	avgbb /= float(tb);
	mini = max(avg + (avgb - avg) * 2.f, avgbb);
	maxi = min(avg + (avgu - avg) * 2.f, avguu);
//		mini = avgbb;
//		maxi = avguu;
/*		for(uint i = 1; i < f.size(); i++)
			if(f[i] > maxi && f[i] < avg * 10.) maxi = f[i];
			else if(f[i] < mini && f[i] > avg / 10.) mini = f[i];
*/
	delta = maxi - mini;
	if(!delta) delta = 1.;
	BufferData d(f.size(), theScope);
	for(uint i = 0; i < f.size(); i++)
		d[i] = finite(f[i]) ? std::min(1.f, std::max(0.f, (f[i] - mini) / delta)) : 0.;
	output(0) << d;
	f.clear();
}

EXPORT_CLASS(Normalise, 0,1,1, Processor);


