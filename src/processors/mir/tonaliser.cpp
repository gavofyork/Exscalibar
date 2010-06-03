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

#include "spectrum.h"
using namespace SignalTypes;

class Tonaliser: public SubProcessor
{
public:
	Tonaliser(): SubProcessor("Tonaliser") {}

private:
	virtual QString simpleText() const { return "T"; }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(Properties const&);
	virtual void updateFromProperties(Properties const&properties);
	virtual bool verifyAndSpecifyTypes(SignalTypeRefs const& _inTypes, SignalTypeRefs& _outTypes);
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _c) const;

	QVector<int> m_bands;
	QVector<int> m_bc;
};

PropertiesInfo Tonaliser::specifyProperties() const
{
	return PropertiesInfo("Bands", 12, "Number of bands.", false, "#", AV(12, 12 * 128, AllowedValue::Log2));
}

void Tonaliser::initFromProperties(Properties const& _p)
{
	int bands = _p["Bands"].toInt();
	m_bc.resize(bands);
	setupIO(1, 1);
	setupSamplesIO(1, 1, 1);
}

void Tonaliser::updateFromProperties(Properties const&)
{
}

bool Tonaliser::verifyAndSpecifyTypes(SignalTypeRefs const& _inTypes, SignalTypeRefs& _outTypes)
{
	if (!_inTypes[0].isA<Spectrum>())
		return false;

	static const float middleC = 261.626f;
	m_bands.resize(_inTypes[0].scope());
	for (int ob = 0; ob < m_bc.count(); ob++)
		m_bc[ob] = 0;
	for (uint b = 0; b < _inTypes[0].scope(); b++)
	{
		int ob = -1;
		float bf = _inTypes[0].asA<Spectrum>().bandFrequency(b);
		if (!isInf(bf) && bf > 0.f)
		{
			float lnote = log2(_inTypes[0].asA<Spectrum>().bandFrequency(b) / middleC);
			ob = (int)floor((lnote - floor(lnote)) * m_bc.count());
			m_bc[ob]++;
		}
		m_bands[b] = ob;
	}
	_outTypes[0] = Spectrum(m_bc.count(), _inTypes[0].asA<Spectrum>().frequency(), _inTypes[0].asA<Spectrum>().maxAmplitude(), _inTypes[0].asA<Spectrum>().minAmplitude());
	return true;
}

void Tonaliser::processChunks(BufferDatas const& _in, BufferDatas& _out, uint _ch) const
{
	for (uint i = 0; i < _ch; i++)
	{
		for (int ob = 0; ob < m_bc.count(); ob++)
			_out[0](i, ob) = 0.f;
		for (uint b = 0; b < _in[0].scope(); b++)
			if (m_bands[b] != -1)
				_out[0](i, m_bands[b]) += _in[0](i, b);
		for (int ob = 0; ob < m_bc.count(); ob++)
			_out[0](i, ob) /= m_bc[ob];
	}
}

EXPORT_CLASS(Tonaliser, 0,3,0, SubProcessor);


class Deaverage: public SubProcessor
{
public:
	Deaverage(): SubProcessor("Deaverage") {}

private:
	virtual QString simpleText() const { return QString("-")+QChar(0x03BC); }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &) { setupIO(1, 1); setupSamplesIO(1, 1, 1); }
	virtual void updateFromProperties(const Properties &properties);
	virtual bool verifyAndSpecifyTypes(SignalTypeRefs const& _inTypes, SignalTypeRefs& _outTypes);
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _c) const;
};

PropertiesInfo Deaverage::specifyProperties() const
{
	return PropertiesInfo();
}

void Deaverage::updateFromProperties(Properties const&)
{
}

bool Deaverage::verifyAndSpecifyTypes(SignalTypeRefs const& _inTypes, SignalTypeRefs& _outTypes)
{
	if (_inTypes[0].scope() == 1)
		return false;
	_outTypes = _inTypes;
	return true;
}

void Deaverage::processChunks(BufferDatas const& _in, BufferDatas& _out, uint _ch) const
{
	for (uint i = 0; i < _ch; i++)
	{
		float avg = 0.f;
		for (uint j = 0; j < _in[0].scope(); j++)
			avg += _in[0](i, j);
		avg /= _in[0].scope();
		for (uint j = 0; j < _in[0].scope(); j++)
			_out[0](i, j) = max(0.f, _in[0](i, j) - avg);
	}
}

EXPORT_CLASS(Deaverage, 0,3,0, SubProcessor);

