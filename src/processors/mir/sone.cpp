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

#include "qfactoryexporter.h"

#include "Geddei"
using namespace Geddei;

#include "SignalTypes"
using namespace SignalTypes;

class Terhardt : public SubProcessor
{
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);

public:
	float *theMult;
	Terhardt() : SubProcessor("Terhardt"), theMult(NULL) {}
	~Terhardt() { delete [] theMult; }
};

void Terhardt::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for (uint i = 0; i < ins[0].elements(); i++)
	{
		outs[0][i] = ins[0][i] * theMult[i];
	}
}

bool Terhardt::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	// TODO: should only take a "stepped spectrum" in.
	if (!inTypes[0].isA<Spectrum>()) return false;
	const Spectrum &in = inTypes[0].asA<Spectrum>();
	outTypes = inTypes;
	delete [] theMult;
	theMult = new float[in.scope()];
	for (uint i = 0; i < in.scope(); i++)
	{
		float f = in.bandFrequency(i);
		float Adb = -3.64f * pow(f * 0.001f, -0.8f) + 6.5f * exp(-0.6 * pow(0.001f * f - 3.3f, 2.f)) - .001f * pow(.001f * f, 4.f);
		theMult[i] = f > 0.f ? exp(Adb / 10.f) : 0.f;
	}
	return true;
}

EXPORT_CLASS(Terhardt, 1,0,1, SubProcessor);

class Sone : public SubProcessor
{
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);

public:
	Sone() : SubProcessor("Sone") {}
};

void Sone::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for (uint i = 0; i < ins[0].elements(); i++)
	{
		// outs[0][i] => dB below max.
		// assume -90dB is SPL
		float dBSPL = 10.f * log10(ins[0][i]) + 90.f;
		if (dBSPL < 0.f) dBSPL = 0.f;
		outs[0][i] = dBSPL > 40.f ? pow(2.f, (dBSPL - 40.f) / 10.f) : pow(dBSPL / 40.f, 2.642f);
	}
}

bool Sone::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	// TODO: should only take a "stepped spectrum" in.
	if (!inTypes[0].isA<Spectrum>()) return false;
//	const Spectrum &in = inTypes[0].asA<Spectrum>();
	outTypes[0] = Spectrum(inTypes[0].asA<Spectrum>().size(), inTypes[0].asA<Spectrum>().frequency(), inTypes[0].asA<Spectrum>().step(), 60, 0);
	return true;
}

EXPORT_CLASS(Sone, 1,0,1, SubProcessor);
/*
class Rhythm: public SubProcessor
{
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);


public:
	Rhythm() : SubProcessor("Rhythm") {}
};

void Rhythm::initFromProperties(const Properties &)
{
	setupSamplesIO(64, 4, 1);
}

bool Rhythm::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	return true;
}

void Rhythm::processChunk(const BufferDatas &in, BufferDatas &out) const
{

}
*/
/**
 * @brief Takes what comes in and makes a histogram with it.
 */
class Histogram: public SubProcessor
{
	uint theRows, theColumns, theCount, theStep;
	float thePeriod, theOverlap;
	float *theWindow;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
public:
	Histogram() : SubProcessor("Histogram") { theWindow = NULL; }
	~Histogram() { delete [] theWindow; }
};

PropertiesInfo Histogram::specifyProperties() const
{
	return PropertiesInfo
		("Period", 12.f, "The period of time to derive each histogram from.")
		("Overlap", 6.f, "The period of overlap to leave between successive histograms.")
		("Rows", 50, "The vertical resolution of the histograms.");
}

void Histogram::initFromProperties(const Properties &p)
{
	thePeriod = p["Period"].toDouble();
	theOverlap = p["Overlap"].toDouble();
	theRows = p["Rows"].toUInt();
	setupIO(1, 1);
}

bool Histogram::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<Spectrum>()) return false;
	theColumns = inTypes[0].scope();
	outTypes[0] = Matrix(theColumns, theRows, inTypes[0].frequency() / theStep, 0, 0);
	theCount = uint(thePeriod * inTypes[0].frequency());
	theStep = uint(theOverlap * inTypes[0].frequency());
	setupSamplesIO(theCount, theStep, 1);

	delete [] theWindow;
	theWindow = new float[theCount];
	for (uint i = 0; i < theCount; ++i)
	{
		theWindow[i] = 1.f;//.5f * (1.f - cos(2.f * M_PI * float(i) / float(theCount - 1)));
	}

	return true;
}

void Histogram::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	float mout = 0.f;
	for (uint i = 0; i < theColumns * theRows; ++i)
		out[0][i] = 0.f;
	for (uint s = 0; s < theCount; ++s)
	{	for (uint c = 0; c < theColumns; ++c)
		{	for (uint r = 0; float(r + 1) / float(theRows) < in[0](s, c) * theWindow[s]; ++r)
			{
				out[0][r + c * theRows] += 1.f;
				mout = max(mout, out[0][r + c * theRows]);
			}
		}
	}
	if (mout > 0.f)
		for (uint c = 0; c < theColumns; ++c)
			for (uint r = 0; r < theRows; ++r)
				out[0][r + c * theRows] /= mout;
}

EXPORT_CLASS(Histogram, 1,0,1, SubProcessor);

