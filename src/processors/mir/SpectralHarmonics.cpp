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

#include <cstdlib>
#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "bufferdata.h"
#include "processor.h"
#include "buffer.h"
using namespace Geddei;

#include "value.h"
#include "spectrum.h"
#include "matrix.h"
using namespace Geddei;

#define AutoPropertiesStart

class SpectralHarmonics: public SubProcessor
{
public:
	SpectralHarmonics(): SubProcessor("SpectralHarmonics") {}

private:
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual QString simpleText() const { return QChar(0x266D); }

	FreqSteppedSpectrum m_signal;

	float m_lowerBand;
	float m_threshold;
	float m_deltaThreshold;
	float m_ratioThreshold;
	float m_maxThreshold;

	float m_mag;
	int m_harmonics;
	int m_maxPass;
};


void SpectralHarmonics::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	BufferData const in = ins[0];
	BufferData pass = outs[0];
	BufferData har = outs[1];

	int sc = m_signal.bins();

	har.copyFrom(in);
	pass.copyFrom(in);

	for (int i = 0; i < sc; i++)
		har(0, i) = 0.f;

	float total = 0.f;
	for (int i = 0; i < sc; i++)
		total += in[i];

	for (int n = 0; n < m_maxPass; n++)
	{
		int mi = 0;
		int mm = 0;
		float men = 0.f;

		for (int i = max<int>(2, round(m_signal.frequencyBand(m_lowerBand))) * m_mag; i < sc; i++)
		{
			// freq: ( [0, sc) + 1 ) / harmonics
			// from: ( 0 .. (sc - (freq * harmonics)) ] + 1
			float freq = float(i + 1) / float(m_mag);

			int m = 0;
//			for (int m = 1; m < (int)floor(sc / freq - m_harmonics); m++)
			{
				float en = 0.f;
				float ens[m_harmonics];
				for (int h = 1; h <= m_harmonics; h++)
					en += (ens[h - 1] = pass[(int)round((m + h) * freq)] + pass[(int)round((m + h) * freq + .5f)]);
				qSort(ens, ens + m_harmonics - 1);
				en = ens[m_harmonics / 2];
				if (en > men)
				{
					mi = i;
					mm = m;
					men = en;
				}
			}
		}

//		if (men < total / sc * m_threshold)
	//		break;

		for (int h = 1; h <= m_harmonics
					|| ((int)round((mm + h) * (float(mi + 1) / float(m_mag))) < sc
						&& max(pass[(int)round((mm + h) * (float(mi + 1) / float(m_mag)))], pass[(int)round((mm + h) * (float(mi + 1) / float(m_mag)))])
								> men / 2.f); h++)
		{
			pass[(int)round((mm + h) * (float(mi + 1) / float(m_mag)))] = 0.f;
			pass[(int)round((mm + h) * (float(mi + 1) / float(m_mag)) + .5f)] = 0.f;
		}
		har[mi] += men;
	}
/*
//		float mphen = 0.f;
//		float lphen = 0.f;
		float phens[i / kkkfkfkf];
		float tphen = 0.f;
		for (int ph = 0; ph < i / kkkfkfkf; ph++)
		{
			phens[ph] = 0.f;
			for (int k = 0; k < m_signal.bins() * kkkfkfkf / i; k++)
			{
				phens[ph] += in[k * i / kkkfkfkf + ph];
			}
			tphen += phens[ph];
		}
//		qSort(phens, phens+(i / kkkfkfkf));
		har[i] = max(0.f, phens[0] / (tphen - phens[0]));
//		float avg = total / (m_signal.bins() * kkkfkfkf / i);	// avg en per phase
//		float med = phens[i / kkkfkfkf / 2];
//		har[i] = mphen;//max(0.f, mphen / med);// / (total / m_signal.bins());//lphen;*/
//	}
/*	for (int i = max<int>(2, round(m_signal.frequencyBand(m_lowerBand))); i < (m_signal.bins() - 1) / 2; i++)
	{
		float en = pass[i];
		float mb = i;
		float men = en;
		float maxen = total / i;
		for (int j = i; j < (int)m_signal.bins() / 2; j += i)
		{
			en += pass[j];
			if (pass[j] > men)
			{
				mb = j;
				men = pass[j];
			}
		}
		if (en - men > m_deltaThreshold && en / men > m_ratioThreshold && men > maxen * m_maxThreshold)
		{
			for (int j = i; j < (int)m_signal.bins() / 2; j += i)
				pass[j] = 0;
			har[i] = en;
		}
	}*/
}

bool SpectralHarmonics::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (!inTypes[0].isA<FreqSteppedSpectrum>())
		return false;
	m_signal = inTypes[0].asA<FreqSteppedSpectrum>();
	outTypes[0] = inTypes[0];
	outTypes[1] = FreqSteppedSpectrum(m_signal.bins(), m_signal.frequency(), m_signal.step() / m_harmonics, m_signal.max(), m_signal.min());
	return true;
}

void SpectralHarmonics::initFromProperties(const Properties & _p)
{
	setupIO(1, 2);
	m_harmonics = _p["Harmonics"].toInt();
}

void SpectralHarmonics::updateFromProperties(Properties const& _p)
{
	m_lowerBand = _p["LowerBand"].toFloat();
	m_threshold = _p["Threshold"].toFloat();
	m_deltaThreshold = _p["DeltaThreshold"].toFloat();
	m_ratioThreshold = _p["RatioThreshold"].toFloat();
	m_maxThreshold = _p["MaxThreshold"].toFloat();
	m_mag = max(_p["Harmonics"].toFloat(), _p["Magnification"].toFloat());
	m_maxPass = _p["MaxPass"].toInt();
}

PropertiesInfo SpectralHarmonics::specifyProperties() const
{
	return PropertiesInfo
			("LowerBand", 43.0, "The lower band", true, QChar(0x21E4), AVfrequency)
			("Threshold", .1f, "The width threshold.", true, QChar(0x03B8), AV(0.125, 8, AllowedValue::Log2))
			("DeltaThreshold", 0.f, "The delta threshold.", true, QChar(0x03B4), AVunity)
			("RatioThreshold", 2.f, "The ratio threshold.", true, ":", AV(1.f, 32.f, AllowedValue::Log2))
			("MaxThreshold", 1.3f, "The maximum threshold.", true, "M", AV(1.f, 4.f, AllowedValue::Log2))
			("MaxPass", 3, "MaxPass.", true, "X", AV(1, 12))
			("Magnification", 3.f, "Mag.", true, "G", AV(2.f, 12.f))
			("Harmonics", 3, "Harmonics.", false, "H", AV(2, 12));
}

EXPORT_CLASS(SpectralHarmonics, 0,3,0, SubProcessor);
