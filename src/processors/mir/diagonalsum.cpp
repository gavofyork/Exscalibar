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

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
using namespace Geddei;

#include "spectrum.h"
#include "matrix.h"
using namespace SignalTypes;

class DiagonalSum : public SubProcessor
{
	uint theSize;
	uint theBandwidth;
	bool m_minimiseLatency;
	uint m_minWidth;
	float m_alpha;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo	("Minimise Latency", true, "Take only the minimum required amount of data from the closest to present.")
								("Alpha", 1.0, "Alpha value for the power function.")
								("Minimum Width", 16, "The minimum matrix width to be used while minimising latency.");
	}
	virtual void updateFromProperties(Properties const& _p)
	{
		m_minimiseLatency = _p["Minimise Latency"].toBool();
		m_alpha = _p["Alpha"].toDouble();
		m_minWidth = _p["Minimum Width"].toInt();
	}
public:
	DiagonalSum() : SubProcessor("DiagonalSum") {}
};

bool DiagonalSum::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<SquareMatrix>()) return false;
	theSize = inTypes[0].asA<SquareMatrix>().size();
	theBandwidth = theSize / 2;
	outTypes[0] = Spectrum(theBandwidth, inTypes[0].frequency(), inTypes[0].asA<SquareMatrix>().pitch());
	return true;
}

void DiagonalSum::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	for (uint n = 1; n < theBandwidth; n++)
	{
		out[0][n] = 0.f;
		float wSum = 0.f;
		for (uint i = n; i < theSize; i++)
		{
			float x = float((i - n) / n) / float(theSize / n - 1);
			float w = pow(x, m_alpha);
			wSum += w;
			out[0][n] += w * in[0][i * theSize + i - n];
		}
		out[0][n] /= wSum;
	}
#if 0
	if (m_minimiseLatency)
		for (uint offset = 1; offset < theBandwidth; offset++)
		{
			out[0][offset] = 0;
			float outOf = 0;
			for (int xy = theSize - 1; xy >= 0; xy--)
			{
				float am = pow((float)(xy / offset) / float((theSize - 1) / offset), m_alpha);
				out[0][offset] += in[0][(xy + (xy*theSize) + offset) % (theSize * theSize)] * am;
				outOf += am;
			}
			out[0][offset] /= outOf;
		}
	else
		for (uint offset = 1; offset < theBandwidth; offset++)
		{	out[0][offset] = 0;
			for (uint xy = 0; xy < theSize; xy++)
				out[0][offset] += in[0][(xy + (xy*theSize) + offset) % (theSize * theSize)];
			out[0][offset] /= theSize;
		}
#endif
	out[0][0] = in[0][0];
}

EXPORT_CLASS(DiagonalSum, 0,2,0, SubProcessor);
