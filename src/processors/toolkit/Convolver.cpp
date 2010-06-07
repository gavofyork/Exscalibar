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

#include <stdint.h>

#include "qfactoryexporter.h"

#include "transmissiontype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

class Convolver: public StatefulSubProcessor
{
public:
	Convolver() : StatefulSubProcessor("Convolver") {}

private:
	inline void rejig()
	{
		if (m_function == 0)
		{
			for (int i = 0; i < m_convolution.size(); i++)
			{
				float c = (i - (m_convolution.size() - m_size / 2.f)) / (m_size / 2.f);
				m_convolution[i] = c;
			}
		}
		else if (m_function == 1)
		{
			for (int i = 0; i < m_convolution.size(); i++)
			{
				float c = (i - (m_convolution.size() - m_size / 2.f)) / (m_size / 2.f);
				m_convolution[i] = erff(c);
			}
		}
	}

	virtual void processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks);
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual QString simpleText() const { return "S"; }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(40, 96, 160); }

	int m_function;
	int m_size;
	uint m_arity;
	QVector<float> m_convolution;
};

void Convolver::processOwnChunks(BufferDatas const& _in, BufferDatas& _out, uint _ch)
{
	for (uint i = 0; i < _ch; i++)
	{
		for (uint s = 0; s < m_arity; s++)
		{
			_out[0](i, s) = 0.f;
			for (int c = 0; c < m_convolution.size(); c++)
				_out[0](i, s) += _in[0](i + c, s) * m_convolution[c];
		}
	}
}

bool Convolver::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	outTypes = inTypes;
	m_arity = inTypes[0].asA<TransmissionType>().arity();
	return true;
}

void Convolver::initFromProperties(Properties const& _p)
{
	m_convolution.resize(_p["Canvas"].toInt());
	setupIO(1, 1);
	setupSamplesIO(m_convolution.size(), 1, 1);
}

void Convolver::updateFromProperties(Properties const& _p)
{
	m_function = _p["Function"].toInt();
	m_size = _p["Size"].toInt();
	rejig();
}

PropertiesInfo Convolver::specifyProperties() const
{
	return PropertiesInfo
			("Canvas", 16, "The convolution size.", false, "S", AV(2, 256, AllowedValue::Log2))
			("Function", 0, "The convolution function.", true, "f", AV("Slash", "/", 0) AVand("Error", QChar(0x23B0), 1))
			("Size", 16, "Size of the function in samples.", true, "s", AV(1, 512, AllowedValue::Log2));
}

EXPORT_CLASS(Convolver, 0,2,0, SubProcessor);

