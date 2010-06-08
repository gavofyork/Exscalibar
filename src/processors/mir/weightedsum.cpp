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

#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "value.h"
using namespace TransmissionTypes;

class WeightedSum: public SubProcessor
{
public:
	WeightedSum(): SubProcessor("WeightedSum") {}

private:
	virtual QString simpleText() const { return QChar(0x2140); }
	virtual PropertiesInfo specifyProperties() const;
	virtual void updateFromProperties(const Properties &properties);
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _c) const;

	int m_type;
	uint m_arity;
};

PropertiesInfo WeightedSum::specifyProperties() const
{
	return PropertiesInfo("Type", 0, "Type of summation. { 0: Normal; 1: Linear weighted on index }");
}

void WeightedSum::updateFromProperties(Properties const& _p)
{
	m_type = _p.get("Type").toInt();
}

bool WeightedSum::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	if (!_inTypes[0].isA<Contiguous>())
		return false;
	_outTypes = Value(_inTypes[0].asA<Contiguous>().frequency());
	m_arity = _inTypes[0].asA<Contiguous>().arity();
	return true;
}

void WeightedSum::processChunks(BufferDatas const& _in, BufferDatas& _out, uint _c) const
{
	if (m_type == 0)
		for (uint i = 0; i < _c; i++)
		{
			_out[0][i] = 0.f;
			for (uint s = 0; s < m_arity; s++)
				_out[0][i] += _in[0](i, s);
		}
	else if (m_type == 1)
		for (uint i = 0; i < _c; i++)
		{
			_out[0][i] = 0.f;
			for (uint s = 0; s < m_arity; s++)
				_out[0][i] += _in[0](i, s) * s;
		}
}

EXPORT_CLASS(WeightedSum, 0,3,0, SubProcessor);
