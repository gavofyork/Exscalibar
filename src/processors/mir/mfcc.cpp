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

#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
using namespace Geddei;

#define BINS 24
#define CEPSTRA BINS

class MFCC : public SubProcessor
{
	uint theMarkers[BINS + 2], bandWidth;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);

public:
	MFCC() : SubProcessor("MFCC") {}
};

void MFCC::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	static float theBins[BINS];
	float t;

	// mel scaling with BINS triangular-sum-log band bins
	for (uint i = 1; i < BINS - 1; i++)
	{	t = 0;
		for (uint j = theMarkers[i - 1]; j < theMarkers[i]; j++)
			t += abs(ins[0][j]) * float(j - theMarkers[i - 1]) / float(theMarkers[i] - theMarkers[i - 1]);
		for (uint j = theMarkers[i]; j < theMarkers[i + 1]; j++)
			t += abs(ins[0][j]) * float(theMarkers[i + 1] - j) / float(theMarkers[i + 1] - theMarkers[i]);
		theBins[i - 1] = log(t+.0000000001f);
	}
	// ceptrum transformation (dct) TODO: use FFTW
	for (uint i = 0; i < CEPSTRA; i++)
	{	t = 0;
		for (uint j = 0; j < BINS; j++)
			t += theBins[j] * cos(M_PI / BINS * (i + 1.0) * (j + 0.5));
		outs[0][i] = t / float(BINS);
	}
}

float toMel(float hertz) { return 1127.01048 * log(1.0 + hertz / 700.0); }
float toHertz(float mel) { return 700.0 * (exp(mel / 1127.01048 - 1.0)); }

bool MFCC::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (!inTypes[0].isA<FreqSteppedSpectrum>()) return false;
	const FreqSteppedSpectrum &in = inTypes[0].asA<FreqSteppedSpectrum>();
	outTypes[0] = Spectrum(CEPSTRA, in.frequency());

	float maxMel = toMel(in.nyquist());
	for (uint i = 0; i < BINS + 2; i++)
		theMarkers[i] = int(toHertz(float(i) * maxMel / float(BINS + 2))) / int(in.step());
	return true;
}

EXPORT_CLASS(MFCC, 0,2,1, SubProcessor);
