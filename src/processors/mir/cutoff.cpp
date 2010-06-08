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

class CutOff : public SubProcessor
{
	float theFreqFrom, theFreqTo;
	uint theFrom, theTo, theSize;

	virtual void processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const;
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual QString simpleText() const { return QChar(0x2702); }

	void processSamples(const BufferData &in, BufferData &out, uint chunks);

public:
	CutOff() : SubProcessor("CutOff") {}
};

void CutOff::processChunks(const BufferDatas &ins, BufferDatas &outs, uint chunks) const
{
	for (uint i = 0; i < chunks; i++)
		outs[0].sample(i).copyFrom(ins[0].mid(i * theSize + theFrom, theTo - theFrom));
}

bool CutOff::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (!inTypes[0].isA<FreqSteppedSpectrum>()) return false;
	const FreqSteppedSpectrum &s = inTypes[0].asA<FreqSteppedSpectrum>();

	float base = 0;
	theSize = s.size();
	theFrom = ::min(s.size(), (uint)::max(0, (int)round((theFreqFrom - base) / s.step())));
	theTo = ::min(s.size(), (uint)::max(0, (int)round((theFreqTo - base) / s.step())));

	outTypes[0] = FreqSteppedSpectrum(theTo - theFrom, s.frequency(), s.step());
	return true;
}

void CutOff::initFromProperties(const Properties &properties)
{
	theFreqFrom = properties.get("Frequency From").toDouble();
	theFreqTo = properties.get("Frequency To").toDouble();
}

PropertiesInfo CutOff::specifyProperties() const
{
	return PropertiesInfo("Frequency From", 0., "The lower bound of bands allowed to pass.")
						 ("Frequency To", 4000., "The uppoer bound of bands allowed to pass.");
}

EXPORT_CLASS(CutOff, 0,1,0, SubProcessor);
