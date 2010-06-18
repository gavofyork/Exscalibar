/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

#include "qfactoryexporter.h"

#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
using namespace Geddei;

class Bandwise: public SubProcessor
{
public:
	enum { RegularBanding = 0, BarkBanding };

	Bandwise() : SubProcessor("Bandwise", OutConst) {}

private:
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void initFromProperties(const Properties &properties);
	virtual PropertiesInfo specifyProperties() const;
	virtual QString simpleText() const { return QChar(0x21F6); }

	QVector<uint>	m_bins;
	uint			m_banding;
};

// REPOT: Move these out to a helper header.

float barkFrequency(float _band)
{
	static uint barkBands[26] = { 0, 100, 200, 300, 400, 510, 630, 770, 920, 1080, 1270, 1480, 1720, 2000, 2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500, 25000 };
	if (_band <= 0.f)
		return 0.f;
	if (_band >= 24.f)
		return 15500.f;
	return lerp<float>(barkBands[(int)floor(_band)], barkBands[(int)floor(_band) + 1], _band - floor(_band));
}

float barkBand(float _freq)
{
	static uint barkBands[26] = { 0, 100, 200, 300, 400, 510, 630, 770, 920, 1080, 1270, 1480, 1720, 2000, 2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500, 25000 };
	if (_freq < 0.f)
		return 0;
	if (_freq > 15500.f)
		return 24;
	uint i;
	for (i = 0; i < 24 && barkBands[i + 1] <= _freq; i++) {}
	return i + (barkBands[i + 1] - _freq) / (barkBands[i + 1] - barkBands[i]);
}

PropertiesInfo Bandwise::specifyProperties() const
{
	return PropertiesInfo	("Banding", 0, "The method for subband-splitting. { 0: Regular; 1: Bark-banding }")
							("Multiplicity", 3, "Number of subspectra to create. { 1.. }");
}

void Bandwise::initFromProperties(Properties const& _p)
{
	m_banding = _p["Banding"].toInt();
	m_bins.resize(_p["Multiplicity"].toInt());
	setupIO(1, m_bins.size());
}

bool Bandwise::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (!inTypes[0].isA<FreqSteppedSpectrum>())
		return false;
	const FreqSteppedSpectrum &in = inTypes[0].asA<FreqSteppedSpectrum>();

	// need to populate m_bins with numbers of bands for each output spectrum

	if (m_banding == RegularBanding)
	{
		int lastBand = 0;
		for (int i = 0; i < m_bins.size(); i++)
		{
			int band = (i + 1) * in.arity() / m_bins.size();
			m_bins[i] = band - lastBand;
			lastBand = band;
		}
	}
	else if (m_banding == BarkBanding)
	{
		int lastBand = 0;
		float mb = barkBand(in.nyquist());
		for (int i = 0; i < m_bins.size(); i++)
		{
			int band = (int)round(in.frequencyBand(barkFrequency(float(i + 1) * mb / float(m_bins.size()))));
			m_bins[i] = band - lastBand;
			lastBand = band;
		}
	}

	for (int i = 0; i < m_bins.count(); i++)
		outTypes[i] = FreqSteppedSpectrum(max(1u, m_bins[i]), in.frequency(), in.step(), in.max(), in.min());	// Wrong - doesn't give first band's freq offset.

	return true;
}

void Bandwise::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	uint c = 0;
	for (uint i = 0; i < outs.count(); i++)
	{
		outs[i].copyFrom(ins[0].mid(c, m_bins[i]));
		c += m_bins[i];
	}
}

EXPORT_CLASS(Bandwise, 0,9,0, SubProcessor);
