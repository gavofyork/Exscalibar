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
using namespace SignalTypes;

class Delta: public SubProcessor
{
	virtual void initFromProperties (const Properties &) { setupIO(1, 1, 2, 1, 1); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes[0] = inTypes[0]; return true; }
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const
	{
		for (uint i = 0; i < ins[0].scope(); i++)
			outs[0](0, i) = ins[0](1, i) - ins[0](0, i);
	}
public:
	Delta(): SubProcessor("Delta") {}
};

EXPORT_CLASS(Delta, 0,1,0, SubProcessor);

class Extract: public SubProcessor
{
	uint m_index;
	virtual PropertiesInfo specifyProperties() const { return PropertiesInfo("Element Index", 0, "Index of the element to extract."); }
	virtual void updateFromProperties(Properties const& _p) { m_index = _p["Element Index"].toInt(); }
	virtual void initFromProperties (const Properties & _p) { setupIO(1, 1, 1, 1, 1); updateFromProperties(_p); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes[0] = Value(inTypes[0].frequency(), inTypes[0].asA<SignalType>().maxAmplitude(), inTypes[0].asA<SignalType>().minAmplitude()); return m_index < inTypes[0].scope(); }
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const
	{
		outs[0][0] = ins[0][m_index];
	}
public:
	Extract(): SubProcessor("Extract") {}
};

EXPORT_CLASS(Extract, 0,1,0, SubProcessor);

class Invert: public SubProcessor
{
	uint m_min;
	uint m_max;
	uint m_scope;
	virtual void initFromProperties (Properties const&) { setupIO(1, 1, 1, 1, 1); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes = inTypes; m_min = inTypes[0].asA<SignalType>().minAmplitude(); m_scope = inTypes[0].scope(); m_max = inTypes[0].asA<SignalType>().maxAmplitude(); return true; }
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const
	{
		for (uint i = 0; i < m_scope; i++)
			outs[0][i] = m_max - ins[0][i] + m_min;
	}
public:
	Invert(): SubProcessor("Invert") {}
};

EXPORT_CLASS(Invert, 0,1,0, SubProcessor);


class Sum: public SubProcessor
{
	virtual void initFromProperties (const Properties &);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void processChunk(const BufferDatas &ins, BufferDatas &outs) const;
public:
	Sum();
};

Sum::Sum(): SubProcessor("Sum")
{
}

void Sum::initFromProperties (const Properties &)
{
	setupIO(1, 1, 1, 1, 1);
}

bool Sum::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	outTypes = Value(inTypes[0].frequency());
	return true;
}

void Sum::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	outs[0][0] = 0.;
	for (uint i = 0; i < ins[0].elements(); i++)
		outs[0][0] += ins[0][i];
}

EXPORT_CLASS(Sum, 0,1,0, SubProcessor);
