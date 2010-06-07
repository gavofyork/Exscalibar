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
using namespace TransmissionTypes;

class Peakify : public SubProcessor
{
public:
	Peakify() : SubProcessor("Peakify") {}

	int m_algorithm;
	uint m_bins;

private:
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual QString simpleText() const { return QChar(0x234F); }
};

void Peakify::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	BufferData const in = ins[0];
	BufferData out = outs[0];

	if (m_algorithm == 0)
	{
		out[0] = in[0];
		int peak = 0;
		for (uint i = 1; i < m_bins; i++)
			if (peak > -1)
				if (in[i] < in[i - 1])	// roll down.
				{
					out[peak] += in[i];
					out[i] = 0;
				}
				else					// out of the trough.
				{
					peak = -1;
					out[i] = in[i];
				}
			else
				if (in[i] > in[i - 1])	// climb up.
				{
					out[i] = out[i - 1] + in[i];
					out[i - 1] = 0;
				}
				else					// over the hill.
				{
					peak = i - 1;
					out[peak] += in[i];
					out[i] = 0;
				}
		out[0] = 0;
	}
	else if (m_algorithm == 1)
	{
		out[0] = 0;
		out[1] = 0;
		out[2] = 0;
		out[m_bins - 1] = 0;
		float off = in[0];
		for (uint i = 2; i < m_bins; i++)
			off = min(off, in[i]);
		for (uint i = 3; i < m_bins - 1; i++)
			if (in[i] > in[i - 1] && in[i] > in[i + 1]) // peak
				out[i] = (in[i] + max(in[i - 1], in[i + 1])) / 2.f - off;
			else
				out[i] = 0;
	}
	else
		out.copyFrom(in);
}

bool Peakify::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<Spectrum>())
		return false;
	m_bins = inTypes[0].asA<Spectrum>().bins();
	outTypes = inTypes;
	return true;
}

void Peakify::initFromProperties(const Properties &)
{
	setupIO(1, 1);
}

void Peakify::updateFromProperties(Properties const& _p)
{
	m_algorithm = _p["Algorithm"].toInt();
}

PropertiesInfo Peakify::specifyProperties() const
{
	return PropertiesInfo("Algorithm", 1, "Algorithm to use for peak finding.", true, QChar(0x2568), QList<AllowedValue>() << AllowedValue("Area", QChar(0x25A8), 0) << AllowedValue("Altitude", QChar(0x22A5), 1));
}

EXPORT_CLASS(Peakify, 0,3,0, SubProcessor);
