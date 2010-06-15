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

class ToneSplit: public SubProcessor
{
public:
	ToneSplit(): SubProcessor("ToneSplit") {}

private:
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void initFromProperties() { setupIO(1, 2); setupSamplesIO(10, 1, 1); }
	virtual PropertiesInfo specifyProperties() const;
	virtual QString simpleText() const { return QChar(0x266D); }

	Typed<FreqSteppedSpectrum> m_signal;

	float m_tail;

	DECLARE_1_PROPERTY(ToneSplit, m_tail);
};

void ToneSplit::processChunk(BufferDatas const& _ins, BufferDatas& o_outs) const
{
	for (uint s = 0; s < m_signal->bins(); s++)
	{
		o_outs[0](0, s) = _ins[0](9, s);
		for (int i = 10 - m_tail; i < 9; i++)
			if (o_outs[0](0, s) > _ins[0](i, s))
				o_outs[0](0, s) = _ins[0](i, s);
		o_outs[1](0, s) = _ins[0](9, s) - o_outs[0](0, s);
	}
}

bool ToneSplit::verifyAndSpecifyTypes(Types const& _inTypes, Types& o_outTypes)
{
	if (!(m_signal = _inTypes[0]))
		return false;
	o_outTypes = _inTypes[0];
	return true;
}

PropertiesInfo ToneSplit::specifyProperties() const
{
	return PropertiesInfo
			("Tail", 3, "The tail.", true, "t", AV(1, 10));
}

EXPORT_CLASS(ToneSplit, 0,3,0, SubProcessor);

