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

#include "qfactoryexporter.h"

#include "contiguous.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

class DownSample : public SubProcessor
{
	uint theCount, theArity, theStep;
	double theOverlap, thePeriod;
	enum { Mean = 0, Max, Min };
	uint theConsolidate;

	virtual QString simpleText() const { return QChar(0x290B); }
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
		if (theArity > 1)
			for (uint i = 0; i < chunks; i++)
				outs[0].sample(i).copyFrom(ins[0].sample(i * theStep));
		else
			for (uint i = 0; i < chunks; i++)
				outs[0][i] = ins[0][i * theStep];
	else
	{	for (uint j = 0; j < chunks; j++)
			for (uint i = 0; i < theArity; i++)
				outs[0](j, i) = 0;
		for (uint j = 0; j < chunks; j++)
		{	for (uint i = 0; i < theCount; i++)
			{	BufferData d = ins[0].sample(i + j*theStep);
				const float *inSample = d.readPointer();
				if (theConsolidate == Mean)
					for (uint k = 0; k < theArity; k++)
						outs[0](j, k) += inSample[k];
				else if (theConsolidate == Max)
					for (uint k = 0; k < theArity; k++)
						if (outs[0](j, k) < inSample[k] || !k) outs[0](j, k) = inSample[k];
				else if (theConsolidate == Min)
					for (uint k = 0; k < theArity; k++)
						if (outs[0](j, k) > inSample[k] || !k) outs[0](j, k) = inSample[k];
			}
		}
		if (theConsolidate == Mean)
			for (uint j = 0; j < chunks; j++)
				for (uint i = 0; i < theArity; i++)
					outs[0](j, i) /= theCount;
	}
}

bool DownSample::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<Contiguous>())
		return false;
	theArity = inTypes[0].asA<TransmissionType>().arity();
	outTypes = inTypes[0];

	if (thePeriod != 0.0) theCount = uint(thePeriod * inTypes[0].asA<Contiguous>().frequency());
	if (theOverlap != 0.0) theStep = uint(theOverlap * inTypes[0].asA<Contiguous>().frequency());

	outTypes[0].asA<Contiguous>().setFrequency(inTypes[0].asA<Contiguous>().frequency() / (float)theStep);
	setupSamplesIO(max(theCount, theStep), theStep, 1);
	return true;
}

void DownSample::initFromProperties(const Properties &properties)
{
	// TODO: setupDimensions according to properties if neccessary.
	theCount = properties.get("Spread").toInt();
	theStep = properties.get("Divisor").toInt();
	thePeriod = properties.get("Period").toDouble();
	theOverlap = properties.get("Overlap").toDouble();
	theConsolidate = properties["Consolidate"].toInt();
	setupIO(1, 1);
}

PropertiesInfo DownSample::specifyProperties() const
{
	return PropertiesInfo("Spread", 1, "The number of samples from which to calculate the value. A larger number results in a smoother output, a lower number is faster. This should be >= 1 and <= Divisor.")
						 ("Divisor", 2, "The sampling frequency divisor.")
						 ("Period", 0.f, "The period of time to derive each sample from.")
						 ("Overlap", 0.f, "The period of overlap to leave between successive samples.")
						 ("Consolidate", 0, "The method to determine the final sample when Spread > 1. { 0: Average; 1: Max; 2: Min }");
}

EXPORT_CLASS(DownSample, 0,3,0, SubProcessor);
