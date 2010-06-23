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

class PhonToSone: public SubProcessor
{
public:
	PhonToSone(): SubProcessor("PhonToSone") {}

private:
	virtual QString simpleText() const { return "S"; }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(30, 160, 128); }
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void processChunk(BufferDatas const& _ins, BufferDatas& _outs) const;

	uint m_bins;
};

void PhonToSone::processChunk(BufferDatas const& _ins, BufferDatas& _outs) const
{
	for (uint i = 0; i < m_bins; i++)
	{
		float dBSPL = _ins[0][i];
		if (dBSPL < 0.f) dBSPL = 0.f;
		_outs[0][i] = dBSPL > 40.f ? pow(2.f, (dBSPL - 40.f) / 10.f) : pow(dBSPL / 40.f, 2.642f);
	}
}

bool PhonToSone::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	Typed<Spectrum> in = _inTypes[0];
	if (!in) return false;

	m_bins = in->bins();
	in->setRange(0, 60);
	_outTypes[0] = in;

	return true;
}

EXPORT_CLASS(PhonToSone, 1,0,1, SubProcessor);


