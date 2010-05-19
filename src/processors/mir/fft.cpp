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

#include "spectrum.h"
#include "wave.h"
using namespace SignalTypes;

#define PI 3.1415926535898

#ifdef HAVE_FFTW3F

#include <fftw3.h>

class FFT: public SubProcessor
{
	int theSize, theStep;

	fftwf_plan thePlan;
	float *theIn, *theOut;
	float *theWindow;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &p);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual QString simpleText() const { return QChar(0x237c); }

public:
	FFT() : SubProcessor("FFT") {}
	~FFT();
};

void FFT::initFromProperties(const Properties &p)
{
	theStep = p["Step"].toInt();
	theSize = p["Size"].toInt();
	setupSamplesIO(theSize, theStep, 1);
	theIn = (float *)fftwf_malloc(sizeof(float) * theSize);
	theOut = (float *)fftwf_malloc(sizeof(float) * theSize);
	thePlan = fftwf_plan_r2r_1d(theSize, theIn, theOut, FFTW_R2HC, p["Optimise"].toBool() ? FFTW_MEASURE : FFTW_ESTIMATE);
	theWindow = new float[theSize];
	for (int i = 0; i < theSize; ++i)
	{
		theWindow[i] = .5f * (1.f - cos(2.f * PI * float(i) / float(theSize - 1)));
	}
}

FFT::~FFT()
{
	delete [] theWindow;
	fftwf_destroy_plan(thePlan);
	fftwf_free(theIn);
	fftwf_free(theOut);
}

void FFT::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	for (int i = 0; i < theSize; i++)
	{	theIn[i] = in[0][i] * theWindow[i];
	}

	fftwf_execute(thePlan);

	out[0][0] = theOut[0] / float(theSize / 2);
	for (int i = 1; i < theSize / 2; i++)
	{
		float xsq = theOut[i] * theOut[i] + theOut[theSize - i] * theOut[theSize - i];
		out[0][i] = isFinite(xsq) && xsq > 0 ? sqrt(xsq) / float(theSize / 2) : 0;
	}
	out[0][theSize / 2] = theOut[theSize / 2] / float(theSize / 2);
}

#else

#include <math.h>
using namespace std;

class FFT : public SubProcessor
{
	uint theLogSize, theStep, theSize, theSizeMask;
	float *costable, *sintable;
	uint *bitReverse;
	float *real, *imag;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);

public:
	FFT();
	virtual ~FFT();
};

int reverseBits(uint initial, uint logSize)
{
	uint reversed = 0;
	for (uint loop = 0; loop < logSize; loop++)
	{
		reversed <<= 1;
		reversed += (initial & 1);
		initial >>= 1;
	}
	return reversed;
}

FFT::FFT() : SubProcessor("FFT")
{
	sintable = 0;
	costable = 0;
	bitReverse = 0;
}

FFT::~FFT()
{
	delete [] sintable;
	delete [] costable;
	delete [] bitReverse;
	delete [] real;
	delete [] imag;
}

void FFT::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	for (uint i = 0; i < theSize; i++)
	{	real[i] = in[0][bitReverse[i] & theSizeMask];
		imag[i] = 0;
	}
	uint exchanges = 1, factfact = theSize / 2;
	float fact_real, fact_imag, tmp_real, tmp_imag;

	for (uint i = theLogSize; i != 0; i--)
	{
		for (uint j = 0; j != exchanges; j++)
		{
			fact_real = costable[j * factfact];
			fact_imag = sintable[j * factfact];

			for (uint k = j; k < theSize; k += exchanges << 1)
			{
				int k1 = k + exchanges;
				tmp_real = fact_real * real[k1] - fact_imag * imag[k1];
				tmp_imag = fact_real * imag[k1] + fact_imag * real[k1];
				real[k1] = real[k] - tmp_real;
				imag[k1] = imag[k] - tmp_imag;
				real[k]  += tmp_real;
				imag[k]  += tmp_imag;
			}
		}
		exchanges <<= 1;
		factfact >>= 1;
	}

	float theMagnitude = (theSize / 2);
	theMagnitude *= theMagnitude;

	for (uint i = 0; i < theSize / 2; i++)
		out[0][i] = ((real[i] * real[i]) + (imag[i] * imag[i])) / theMagnitude;
	out[0][0] /= 4;
}

void FFT::initFromProperties(const Properties &properties)
{
	theLogSize = int(floor(log(double(properties["Size"].toInt())) / log(2.0)));
	theStep = properties["Step"].toInt();
	theSize = 1 << theLogSize;
	if (theSize != (uint)properties["Size"].toInt())
		qDebug("*** WARNING: Using simple FFT as FFTW not found. Can only use FFT sizes that are\n"
			   "             powers of 2. Size truncated from %d (=2^%f) to %d (=2^%d). Fix this\n"
			   "             by installing libfftw and recompiling the FFT subprocessor.", properties["Size"].toInt(), log(double(properties["Size"].toInt())) / log(2.0), theSize, theLogSize);
	theSizeMask = theSize - 1;
	setupIO(1, 1, theSize, theStep, 1);

	// set up lookups
	delete [] sintable;
	delete [] costable;
	delete [] bitReverse;
	sintable = new float[theSize / 2];
	costable = new float[theSize / 2];
	for (uint i = 0; i < theSize / 2; i++)
	{	costable[i] = cos(2 * PI * i / theSize);
		sintable[i] = sin(2 * PI * i / theSize);
	}
	bitReverse = new uint[theSize];
	for (uint i = 0; i < theSize; i++)
		bitReverse[i] = reverseBits(i, theLogSize);

	real = new float[theSize];
	imag = new float[theSize];
}

#endif

bool FFT::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<Wave>()) return false;
	outTypes[0] = FreqSteppedSpectrum(theSize / 2 + 1, inTypes[0].frequency() / float(theStep), inTypes[0].frequency() / float(theSize));
	return true;
}

PropertiesInfo FFT::specifyProperties() const
{
	return PropertiesInfo("Size", 2048, "The size of the block (in samples) from which to conduct a short time Fourier transform.")
						 ("Step", 1024, "The number of samples between consequent sampling blocks.")
						 ("Optimise", true, "True if time is taken to optimise the calculation.");
}

EXPORT_CLASS(FFT, 0,9,0, SubProcessor);
