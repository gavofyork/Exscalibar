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
#include <Plugin>
using namespace Geddei;

class DeciBel : public SubProcessor
{
public:
	DeciBel() : SubProcessor("DeciBel") {}

private:
	virtual QString simpleText() const { return "dB"; }
	virtual PropertiesInfo specifyProperties() const;
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void processChunk(BufferDatas const& _in, BufferDatas& _out) const;

	float m_spl;
	DECLARE_1_PROPERTY(DeciBel, m_spl);

	uint m_bins;
	float m_min;
	float m_delta;
};

bool DeciBel::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	Typed<Spectrum> in = _inTypes[0];
	if (!in) return false;

	m_min = in->min();
	m_delta = in->max() - m_min;
	m_bins = in->bins();
	in->setRange(0, 60);
	_outTypes[0] = in;

	return true;
}

void DeciBel::processChunk(BufferDatas const& _ins, BufferDatas& _outs) const
{
	for (uint i = 0; i < m_bins; i++)
		_outs[0](0, i) = max<float>(0.f, 10.f * log10((_ins[0](0, i) - m_min) / m_delta) + m_spl);
}

PropertiesInfo DeciBel::specifyProperties() const
{
	return PropertiesInfo("Spl", 90.f, "The SPL value.", true, ":", AV(30.f, 120.f));
}

EXPORT_CLASS(DeciBel, 1,0,1, SubProcessor);
