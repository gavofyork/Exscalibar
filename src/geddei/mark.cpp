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

#include "mark.h"
#include "source.h"
using namespace Geddei;

namespace TransmissionTypes
{

TRANSMISSION_TYPE_CPP(Mark);

void Mark::polishData(BufferData& _d, Source* _s, uint) const
{
	setTimestamp(_d, _s->secondsPassed());
}

void Mark::setTimestamp(BufferData& _d, double _ts)
{
	union { double d; float f[2]; } ts;
	ts.d = _ts;
	for (uint s = _d.sampleSize() - 2; s < _d.elements(); s += _d.sampleSize())
		(_d[s] = ts.f[0]), (_d[s + 1] = ts.f[1]);
}

double Mark::timestamp(BufferData const& _data)
{
	union { double d; float f[2]; } ts;
	ts.f[0] = _data[_data.sampleSize() - 2];
	ts.f[1] = _data[_data.sampleSize() - 1];
	return ts.d;
}

QString Mark::info() const
{
	return QString("<div><b>Mark</b></div>") + TransmissionType::info();
}

}
