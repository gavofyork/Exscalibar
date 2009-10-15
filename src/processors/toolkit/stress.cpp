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

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

class Stress: public SubProcessor
{
	uint theSize;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);

public:
	Stress() : SubProcessor("Stress") {}
};

void Stress::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for (uint i = 0; i < theSize; i++)
	{
		outs[0][i] = 1.;
		uint v = static_cast<int>(round(ins[0][i]));
		for (uint j = 2; j < min(v, (uint)70); j++)
			outs[0][i] *= v;
	}
}

bool Stress::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	outTypes[0] = inTypes[0];
	theSize = inTypes[0].scope();
	return true;
}

EXPORT_CLASS(Stress, 0,2,0, SubProcessor);
