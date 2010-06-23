
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
using namespace std;

#include <Plugin>
using namespace Geddei;

// NOTE this class is deprecated. Use ISO226 instead.
class Terhardt : public SubProcessor
{
public:
	Terhardt() : SubProcessor("Terhardt") {}

private:
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual QString simpleText() const { return "T"; }

	uint m_bins;

	QVector<float> theMult;
};

void Terhardt::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for (int i = 0; i < theMult.size(); i++)
		outs[0][i] = ins[0][i] * theMult[i];
}

bool Terhardt::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	Typed<Spectrum> in(_inTypes[0]);
	if (!in) return false;

	theMult.resize(in->bins());
	for (int i = 0; i < theMult.size(); i++)
	{
		float f = in->bandFrequency(i);
		float Adb = -3.64f * pow(f * 0.001f, -0.8f) + 6.5f * exp(-0.6 * pow(0.001f * f - 3.3f, 2.f)) - .001f * pow(.001f * f, 4.f);
		theMult[i] = f > 0.f ? exp(Adb / 10.f) : 0.f;
	}

	// TODO: Set range to what it really is given band range and in's range.
	//in->setRange();

	_outTypes[0] = in;
	return true;
}

EXPORT_CLASS(Terhardt, 1,0,1, SubProcessor);
