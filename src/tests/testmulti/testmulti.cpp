/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
 *
 * This file is part of Exscalibar.
 *
 * Exscalibar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Exscalibar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Exscalibar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cmath>
using namespace std;

#include <QThread>

#include "geddei.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace SignalTypes;

#include "player.h"
#include "monitor.h"

class DownSample : public SubProcessor
{
	uint theCount, theScope, theStep;
	enum { Mean = 0, Max, Min };
	uint theConsolidate;

	virtual void processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);

public:
	DownSample() : SubProcessor("DownSample") {}
};

void DownSample::processChunks(const BufferDatas &ins, BufferDatas &outs, uint chunks) const
{
	if (theCount <= 1)
		if (theScope > 1)
			for (uint i = 0; i < chunks; i++)
				outs[0].sample(i).copyFrom(ins[0].sample(i * theStep));
		else
			for (uint i = 0; i < chunks; i++)
				outs[0][i] = ins[0][i * theStep];
	else
	{	for (uint j = 0; j < chunks; j++)
			for (uint i = 0; i < theScope; i++)
				outs[0](j, i) = 0;
		for (uint j = 0; j < chunks; j++)
		{	for (uint i = 0; i < theCount; i++)
			{	BufferData d = ins[0].sample(i + j*theStep);
				const float *inSample = d.readPointer();
				if (theConsolidate == Mean)
					for (uint k = 0; k < theScope; k++)
						outs[0](j, k) += inSample[k];
				else if (theConsolidate == Max)
					for (uint k = 0; k < theScope; k++)
						if (outs[0](j, k) < inSample[k] || !k) outs[0](j, k) = inSample[k];
				else if (theConsolidate == Min)
					for (uint k = 0; k < theScope; k++)
						if (outs[0](j, k) > inSample[k] || !k) outs[0](j, k) = inSample[k];
			}
		}
		for (uint j = 0; j < chunks; j++)
			for (uint i = 0; i < theScope; i++)
				outs[0](j, i) /= theCount;
	}
}

bool DownSample::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	theScope = inTypes[0].scope();
	outTypes = inTypes[0];
	outTypes[0].asA<SignalType>().setFrequency(inTypes[0].frequency() / (float)theStep);
	return true;
}

void DownSample::initFromProperties(const Properties &properties)
{
	// TODO: setupDimensions according to properties if neccessary.
	theCount = properties.get("Spread").toInt();
	theStep = properties.get("Divisor").toInt();
	theConsolidate = properties["Consolidate"].toInt();
	setupSamplesIO(max(theCount, theStep), theStep, 1);
}

PropertiesInfo DownSample::specifyProperties() const
{
	return PropertiesInfo("Spread", 1, "The number of samples from which to calculate the value. A larger number results in a smoother output, a lower number is faster. This should be >= 1 and <= Divisor.")
						 ("Divisor", 2, "The sampling frequency divisor.")
						 ("Consolidate", 0, "The method to determine the final sample when Spread > 1. { 0: Average; 1: Max; 2: Min }");
}


class SpectrumSource: public HeavyProcessor
{
	int theWidth;
	virtual void processor()
	{
		qDebug("O>");
		for (int j = 0; j < 18; j++)
		{
			BufferData d = output(0).makeScratchSamples(1);
			for (int i = 0; i < theWidth; i++)
				if (i < theWidth / 2)
					d[i] = 1.0 / (j + 1.0);
				else
					d[i] = .5;
			output(0) << d;
		}
		plunge();
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes)
	{
		outTypes[0] = Spectrum(theWidth, 1, 1);
		return true;
	}
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo("Width", 18, "The size of the spectrum to be generated.");
	}
	virtual void initFromProperties(const Properties &p)
	{
		setupIO(0, 1);
		theWidth = p["Width"].toInt();
	}
public:
	SpectrumSource(): HeavyProcessor("SpectrumSource") {}
};

class SuitableSplit: public SubProcessor
{
	float theBass, theTreble;
	int theBand[4];
	virtual void initFromProperties (const Properties &) { setupIO(1, 3); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes[0] = Spectrum(2); outTypes[1] = Spectrum(2); outTypes[2] = Spectrum(inTypes[0].scope() - 4); return true; }
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const
	{	cerr << "Sizes: i=" << ins[0].elements() << ", o = " << outs[0].elements() << ", " << outs[1].elements() << ", " << outs[2].elements() << endl;
	}
public:
	SuitableSplit(): SubProcessor("SuitableSplit", OutConst) {}
};

int main(int argc, char **argv)
{
	int w = 0, c = 16;
	if (argc > 1) w = QString(argv[1]).toInt();
	if (argc > 2) c = QString(argv[2]).toInt();

	ProcessorGroup g;

	qDebug("Initialising All...");
	SpectrumSource O;
	O.init("O", g);
	DomProcessor S(new SuitableSplit);
	S.init("S", g, Properties("Multiplicity", 3));
	MultiProcessor M(new SubFactoryCreator("SelfSimilarity"));
	M.init("M", g, Properties("Latency/Throughput", 0.)("Size", 18)("Step", 1));
	MultiProcessor N(new BasicSubCreator<DownSample>);
	N.init("N", g, Properties("Divisor", 1));
	DomProcessor V("CrossSimilarity");
	V.init("V", g);
	Monitor W;
	W.init("W", g);

	qDebug("Connecting All...");
	O[0] >>= S[0];
	S >>= M;
	M >>= N;
	N >>= V;
	V[0] >>= W[0];

	qDebug("Verifying types...");
	if (!g.confirmTypes()) qDebug("Problem confirming types.");
	if (!g.go()) qDebug("Problem starting.");
	W.waitForPlunger();
	qDebug("Stopping...");
	g.stop();
	g.disconnectAll();
	return 0;
}


