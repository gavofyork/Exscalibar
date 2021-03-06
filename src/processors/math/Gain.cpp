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

#include <Plugin>
using namespace Geddei;

class Gain: public SubProcessor
{
public:
	Gain() : SubProcessor("Gain", NotMulti, SubInplace) {}

private:
	virtual QString simpleText() const { return ""; }

	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties() { setupVisual(0, 0); }
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& o_outTypes);
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _c) const;

	float m_gain;
	DECLARE_1_PROPERTY(Gain, m_gain);

	bool m_isMark;
	uint m_arity;
};

PropertiesInfo Gain::specifyProperties() const
{
	return PropertiesInfo("Gain", 1.f, "The gain.", true, "x", AV("-", "-", -1.f) AVand(0.001f, 1000.f, AllowedValue::Log2));
}

bool Gain::verifyAndSpecifyTypes(Types const& _inTypes, Types& o_outTypes)
{
	m_isMark = _inTypes[0].isA<Mark>();
	o_outTypes = _inTypes;
	m_arity = _inTypes[0].arity();
	return true;
}

void Gain::processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _c) const
{
	if (m_gain == 1)
		return;
	if (m_isMark)
	{
		for (uint c = 0; c < _c; c++)
			for (uint i	= 0; i < m_arity; i++)
				_outs[0](c, i) = i ? _ins[0](c, i) : (m_gain * _ins[0](c, i));
//		Mark::setTimestamp(_outs[0], Mark::timestamp(_ins[0]));
	}
	else
		for (uint c = 0; c < _c; c++)
		{
			BufferData d = _outs[0].samples(c);
			for (uint i	= 0; i < m_arity; i++)
				d[i] *= m_gain;// _ins[0](c, i);
		}
}

EXPORT_CLASS(Gain, 0,2,0, SubProcessor);

