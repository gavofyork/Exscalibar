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

class Slur: public StatefulSubProcessor
{
public:
	Slur() : StatefulSubProcessor("Slur") {}

private:
	inline float maxAround(BufferData const& _b, uint i, uint s, uint sc) const
	{
		float ret = _b(i, s);
		for (int p = -m_precision / 2; p <= m_precision / 2; p++)
			ret = max(ret, _b(i, clamp<int>(p+s, 0, sc)));
		return ret;
	}

	virtual void processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks);
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual QString simpleText() const { return "S"; }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(40, 96, 160); }

	float m_quantity;
	int m_precision;
	QVector<float> m_current;
};

void Slur::processOwnChunks(BufferDatas const& _in, BufferDatas& _out, uint _ch)
{
	for (uint i = 0; i < _ch; i++)
		for (int s = 0; s < m_current.count(); s++)
			_out[0](i, s) = m_current[s] = lerp(m_current[s], maxAround(_in[0], i, s, m_current.size()), m_quantity);
}

bool Slur::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	outTypes = inTypes;
	m_current.resize(inTypes[0].arity());
	for (int i = 0; i < m_current.size(); i++)
		m_current[i] = 0.f;
	return true;
}

void Slur::initFromProperties(Properties const&)
{
	setupIO(1, 1);
	setupSamplesIO(1, 1, 1);
}

void Slur::updateFromProperties(Properties const& _p)
{
	m_quantity = _p["Quantity"].toFloat();
	m_precision = _p["Precision"].toInt();
}

PropertiesInfo Slur::specifyProperties() const
{
	return PropertiesInfo
			("Quantity", .5, "The slur quantity.", true, "Q", AVunity)
			("Precision", 0, "Precision", true, "P", AV(0, 10));
}

EXPORT_CLASS(Slur, 0,2,0, SubProcessor);
