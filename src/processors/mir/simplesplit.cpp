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

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
using namespace Geddei;

#include "spectrum.h"
using namespace SignalTypes;

class SimpleSplit: public SubProcessor
{
	int theWidth;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &properties);

public:
	SimpleSplit();
};

SimpleSplit::SimpleSplit(): SubProcessor("SimpleSplit", Out)
{
}

void SimpleSplit::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	for (uint i = 0; i < multiplicity(); i++)
		out[i].copyFrom(in[0].mid(i * theWidth, theWidth));
}

bool SimpleSplit::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<Spectrum>()) return false;
	Spectrum s = inTypes[0].asA<Spectrum>();
	theWidth = s.scope() / multiplicity();
	s.setScope(theWidth);
	outTypes = s;
	return true;
}

void SimpleSplit::initFromProperties(const Properties &)
{
	setupIO(1, Undefined, 1, 1, 1);
}

EXPORT_CLASS(SimpleSplit, 0,1,0, SubProcessor);
