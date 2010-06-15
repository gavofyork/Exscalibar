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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "geddei.h"
using namespace Geddei;

#include "coretypes.h"
using namespace Geddei;

class Fan: public SubProcessor
{
public:
	Fan();

private:
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const;

	uint m_outBins;
};

Fan::Fan(): SubProcessor("Fan", Out)
{
}

bool Fan::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	Typed<Spectrum> in = _inTypes[0];
	if (!in) return false;
	m_outBins = in->bins() / _outTypes.count();
	// Should be FreqSteppedSpectrum with offset if possible.
	// In general add a method to Spectrum to get a new type based on subset.
	_outTypes = Spectrum(m_outBins, in->frequency());
	return true;
}

void Fan::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for (uint i = 0, k = 0; i < outs.count(); i++)
		for (uint j = 0; j < m_outBins; j++,k++)
			outs[i][j] = ins[0][k];
}

EXPORT_CLASS(Fan, 0,1,0, SubProcessor);

/*
class CustomSegment: public SubProcessor
{
	QValueVector<uint> theBands;
	virtual PropertiesInfo specifyProperties() const { return PropertiesInfo("Band0", 8, "")("Band1", 8, "")("Band2", 8, ""); }
	virtual void initFromProperties (const Properties &p) { setupIO(1, 3, 1, 1, 1); theBands.resize(3); theBands[0] = p["Band0"].toInt(); theBands[1] = p["Band1"].toInt(); theBands[2] = p["Band2"].toInt(); }
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
	{
		if (!inTypes[0].isA<Spectrum>()) return false;
		for (uint i = 0; i < theBands.count(); i++)
			outTypes[i] = Spectrum(theBands[i], inTypes[0].frequency());
		return true;
	}
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const
	{
		uint from = 0;
		for (uint i = 0; i < theBands.count(); i++)
		{
			for (uint j = 0; j < theBands[i]; j++)
				outs[i][j] = ins[0][from + j];
			from += theBands[i];
		}
	}
public:
	CustomSegment(): SubProcessor("CustomSegment") {}
};
*/
