/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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
#include "coretypes.h"
using namespace Geddei;

class Magnitude: public SubProcessor
{
public:
	Magnitude(): SubProcessor("Magnitude") {}

private:
	virtual bool verifyAndSpecifyTypes(const Types &in, Types &out);
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;

	uint m_arity;
};

bool Magnitude::verifyAndSpecifyTypes(Types const& _in, Types& _out)
{
	Typed<Spectrum> i(_in[0]);
	if (!i) return false;
	_out = Value(i->frequency());
	m_arity = i->arity();
	return true;
}

void Magnitude::processChunk(BufferDatas const& _in, BufferDatas& _out) const
{
	_out[0][0] = 0.;
	for (uint i = 0; i < m_arity; i++)
		_out[0][0] += _in[0][i] * _in[0][i];
	_out[0][0] = sqrt(_out[0][0]);
}

EXPORT_CLASS(Magnitude, 0,1,0, SubProcessor);
