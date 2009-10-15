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

class Magnitude: public SubProcessor
{
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &in, SignalTypeRefs &out);
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;

public:
	Magnitude();
};

Magnitude::Magnitude(): SubProcessor("Magnitude")
{
}

bool Magnitude::verifyAndSpecifyTypes(const SignalTypeRefs &in, SignalTypeRefs &out)
{
	if (!in[0].isA<Spectrum>()) return false;
	out = Value(in[0].frequency());
	return true;
}

void Magnitude::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	out[0][0] = 0.;
	for (uint i = 0; i < in[0].elements(); i++)
		out[0][0] += in[0][i] * in[0][i];
	out[0][0] = sqrt(out[0][0]);
}

EXPORT_CLASS(Magnitude, 0,1,0, SubProcessor);
