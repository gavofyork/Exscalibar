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
#include "value.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

class Rectify: public SubProcessor
{
public:
	enum { HalfWaveRectification = 0, FullWaveRectification = 1, SquaredRectification = 2 };

	Rectify(): SubProcessor("Rectify") {}

private:
	virtual QString simpleText() const { return "½"; }
	virtual PropertiesInfo specifyProperties() const;
	virtual void updateFromProperties(const Properties &properties);
	virtual bool verifyAndSpecifyTypes(SignalTypeRefs const& _inTypes, SignalTypeRefs& _outTypes);
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _c) const;

	int m_type;
};

PropertiesInfo Rectify::specifyProperties() const
{
	return PropertiesInfo("Type", 0, "Type of rectification. { 0: Half-wave; 1: Full-wave; 2: Square }");
}

void Rectify::updateFromProperties(Properties const& _p)
{
	m_type = _p.get("Type").toInt();
}

bool Rectify::verifyAndSpecifyTypes(SignalTypeRefs const& _inTypes, SignalTypeRefs& _outTypes)
{
	if (_inTypes[0].scope() != 1)
		return false;
	_outTypes = Value(_inTypes[0].frequency(), _inTypes[0].asA<SignalType>().maxAmplitude(), _inTypes[0].asA<SignalType>().minAmplitude());
	return true;
}

void Rectify::processChunks(BufferDatas const& _in, BufferDatas& _out, uint _c) const
{
	if (m_type == HalfWaveRectification)
		for (uint i = 0; i < _c; i++)
			_out[0][i] = _in[0][i] < 0.f ? 0.f : _in[0][i];
	else if (m_type == FullWaveRectification)
		for (uint i = 0; i < _c; i++)
			_out[0][i] = _in[0][i] < 0.f ? -_in[0][i] : _in[0][i];
	else if (m_type == SquaredRectification)
		for (uint i = 0; i < _c; i++)
			_out[0][i] = _in[0][i] * _in[0][i];
}

EXPORT_CLASS(Rectify, 0,3,0, SubProcessor);
