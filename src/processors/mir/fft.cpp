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

#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
#include "wave.h"
using namespace Geddei;

#define PI 3.1415926535898

#if defined(HAVE_FFTW3F) || 1

#include <fftw3.h>

class FFT: public SubProcessor
{
public:
	FFT() : SubProcessor("FFT"), m_plan(0), m_in(0), m_out(0) {}
	~FFT();

private:
	virtual QString simpleText() const { return QChar(0x237c); }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(270, 96, 160); }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;

	bool m_optimise;
	DECLARE_1_PROPERTY(FFT, m_optimise);

	uint m_arity;
	fftwf_plan m_plan;
	float* m_in;
	float* m_out;
};

PropertiesInfo FFT::specifyProperties() const
{
	return PropertiesInfo("Optimise", true, "True if time is taken to optimise the calculation.", false, "O", AVbool);
}

void FFT::initFromProperties()
{
	setupSamplesIO(1, 1, 1);
}

bool FFT::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	Typed<WaveChunk> in = inTypes[0];
	if (!in) return false;
	m_arity = in->length();
	outTypes[0] = FreqSteppedSpectrum(m_arity / 2 + 1, in->frequency(), in->rate() / float(m_arity));
	if (m_in) fftwf_free(m_in);
	if (m_out) fftwf_free(m_out);
	if (m_plan) fftwf_destroy_plan(m_plan);
	m_in = (float *)fftwf_malloc(sizeof(float) * m_arity);
	m_out = (float *)fftwf_malloc(sizeof(float) * m_arity);
	m_plan = fftwf_plan_r2r_1d(m_arity, m_in, m_out, FFTW_R2HC, m_optimise ? FFTW_MEASURE : FFTW_ESTIMATE);
	return true;
}

FFT::~FFT()
{
	if (m_in) fftwf_free(m_in);
	if (m_out) fftwf_free(m_out);
	if (m_plan) fftwf_destroy_plan(m_plan);
}

void FFT::processChunk(BufferDatas const& _in, BufferDatas& _out) const
{
	_in[0].copyTo(m_in, m_arity);
	fftwf_execute(m_plan);
	_out[0][0] = m_out[0] / float(m_arity / 2);
	for (uint i = 1; i < m_arity / 2; i++)
	{
		float xsq = m_out[i] * m_out[i] + m_out[m_arity - i] * m_out[m_arity - i];
		_out[0][i] = isFinite(xsq) && xsq > 0 ? sqrt(xsq) / float(m_arity / 2) : 0;
	}
	_out[0][m_arity / 2] = m_out[m_arity / 2] / float(m_arity / 2);
}

#else

#include <math.h>
using namespace std;

class FFT : public SubProcessor
{
	uint theLogSize, m_hop, m_size, theSizeMask;
	float *costable, *sintable;
	uint *bitReverse;
	float *real, *imag;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
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
	for (uint i = 0; i < m_size; i++)
	{	real[i] = in[0][bitReverse[i] & theSizeMask];
		imag[i] = 0;
	}
	uint exchanges = 1, factfact = m_size / 2;
	float fact_real, fact_imag, tmp_real, tmp_imag;

	for (uint i = theLogSize; i != 0; i--)
	{
		for (uint j = 0; j != exchanges; j++)
		{
			fact_real = costable[j * factfact];
			fact_imag = sintable[j * factfact];

			for (uint k = j; k < m_size; k += exchanges << 1)
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

	float theMagnitude = (m_size / 2);
	theMagnitude *= theMagnitude;

	for (uint i = 0; i < m_size / 2; i++)
		out[0][i] = ((real[i] * real[i]) + (imag[i] * imag[i])) / theMagnitude;
	out[0][0] /= 4;
}

void FFT::initFromProperties(const Properties &properties)
{
	theLogSize = int(floor(log(double(properties["Size"].toInt())) / log(2.0)));
	m_hop = properties["Step"].toInt();
	m_size = 1 << theLogSize;
	if (m_size != (uint)properties["Size"].toInt())
		qDebug("*** WARNING: Using simple FFT as FFTW not found. Can only use FFT sizes that are\n"
			   "             powers of 2. Size truncated from %d (=2^%f) to %d (=2^%d). Fix this\n"
			   "             by installing libfftw and recompiling the FFT subprocessor.", properties["Size"].toInt(), log(double(properties["Size"].toInt())) / log(2.0), m_size, theLogSize);
	theSizeMask = m_size - 1;
	setupIO(1, 1, m_size, m_hop, 1);

	// set up lookups
	delete [] sintable;
	delete [] costable;
	delete [] bitReverse;
	sintable = new float[m_size / 2];
	costable = new float[m_size / 2];
	for (uint i = 0; i < m_size / 2; i++)
	{	costable[i] = cos(2 * PI * i / m_size);
		sintable[i] = sin(2 * PI * i / m_size);
	}
	bitReverse = new uint[m_size];
	for (uint i = 0; i < m_size; i++)
		bitReverse[i] = reverseBits(i, theLogSize);

	real = new float[m_size];
	imag = new float[m_size];
}

PropertiesInfo FFT::specifyProperties() const
{
	return PropertiesInfo("Size", 2048, "The size of the block (in samples) from which to conduct a short time Fourier transform.")
						 ("Step", 1024, "The number of samples between consequent sampling blocks.")
						 ("Optimise", true, "True if time is taken to optimise the calculation.");
}

bool FFT::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (!inTypes[0].isA<Wave>()) return false;
	outTypes[0] = FreqSteppedSpectrum(m_size / 2 + 1, inTypes[0].asA<Contiguous>().frequency() / float(m_hop), inTypes[0].asA<Contiguous>().frequency() / float(m_size));
	return true;
}

#endif

EXPORT_CLASS(FFT, 0,9,0, SubProcessor);
