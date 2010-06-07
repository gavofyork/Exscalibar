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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "geddei.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace TransmissionTypes;

class Delta: public SubProcessor
{
public:
	Delta(): SubProcessor("Delta") {}

private:
	virtual void initFromProperties (const Properties &) { setupSamplesIO(2, 1, 1); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
	{
		outTypes[0] = inTypes[0];
		m_arity = inTypes[0].asA<TransmissionType>().arity();
		return true;
	}
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const
	{
		for (uint i = 0; i < m_arity; i++)
			outs[0](0, i) = ins[0](1, i) - ins[0](0, i);
	}
	virtual QString simpleText() const { return QChar(0x2206); }

	uint m_arity;
};

EXPORT_CLASS(Delta, 0,1,0, SubProcessor);

class Extract: public SubProcessor
{
	uint m_index;
	virtual PropertiesInfo specifyProperties() const { return PropertiesInfo("Element Index", 0, "Index of the element to extract."); }
	virtual void updateFromProperties(Properties const& _p) { m_index = _p["Element Index"].toInt(); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { if (!inTypes[0].isA<Contiguous>()) return false; outTypes[0] = Value(inTypes[0].asA<Contiguous>().frequency(), inTypes[0].asA<Contiguous>().maxAmplitude(), inTypes[0].asA<Contiguous>().minAmplitude()); return m_index < inTypes[0].asA<Contiguous>().arity(); }
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const
	{
		outs[0][0] = ins[0][m_index];
	}
	virtual QString simpleText() const { return QChar(0x2198); }
public:
	Extract(): SubProcessor("Extract") {}
};

EXPORT_CLASS(Extract, 0,1,0, SubProcessor);

class Invert: public SubProcessor
{
	uint m_min;
	uint m_max;
	uint m_arity;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { if (!inTypes[0].isA<Contiguous>()) return false; outTypes = inTypes; m_min = inTypes[0].asA<Contiguous>().minAmplitude(); m_arity = inTypes[0].asA<Contiguous>().arity(); m_max = inTypes[0].asA<Contiguous>().maxAmplitude(); return true; }
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const
	{
		for (uint i = 0; i < m_arity; i++)
			outs[0][i] = m_max - ins[0][i] + m_min;
	}
	virtual QString simpleText() const { return QChar(0x21F5); }
public:
	Invert(): SubProcessor("Invert") {}
};

EXPORT_CLASS(Invert, 0,1,0, SubProcessor);


class Sum: public SubProcessor
{
public:
	Sum();

private:
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const;
	virtual QString simpleText() const { return QChar(0x2211); }

	uint m_arity;
};

Sum::Sum(): SubProcessor("Sum")
{
}

bool Sum::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<Contiguous>())
		return false;
	m_arity = inTypes[0].asA<Contiguous>().arity();
	outTypes = Value(inTypes[0].asA<Contiguous>().frequency());
	return true;
}

void Sum::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	outs[0][0] = 0.;
	for (uint i = 0; i < m_arity; i++)
		outs[0][0] += ins[0][i];
}

EXPORT_CLASS(Sum, 0,1,0, SubProcessor);
