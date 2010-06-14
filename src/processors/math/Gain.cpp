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

#include <Plugin>
using namespace Geddei;

class Gain: public SubProcessor
{
public:
	Gain() : SubProcessor("Gain") {}

private:
	virtual QString simpleText() const { return ""; }

	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties() { setupVisual(0, 0); }
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;

	float m_gain;
	DECLARE_1_PROPERTY(Gain, m_gain);

	uint m_arity;
};

PropertiesInfo Gain::specifyProperties() const
{
	return PropertiesInfo("Gain", 1.f, "The gain.", true, "x", AV(0.00001f, 100000.f, AllowedValue::Log10));
}

void Gain::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for (uint i = 0; i < m_arity; i++)
		outs[0][i] = ins[0][i] * m_gain;
}

bool Gain::verifyAndSpecifyTypes(Types const& _inTypes, Types& o_outTypes)
{
	o_outTypes = _inTypes;
	m_arity = _inTypes[0].arity();
	return true;
}

EXPORT_CLASS(Gain, 0,2,0, SubProcessor);

