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

#include "signaltype.h"
#include "qsocketsession.h"
using namespace Geddei;

#include "value.h"
#include "wave.h"
#include "spectrum.h"
#include "matrix.h"
using namespace SignalTypes;

namespace Geddei
{

SignalType::SignalType(uint scope, float frequency, float _max, float _min): theMin(min(_min, _max)), theMax(max(_min, _max))
{
	theScope = scope;
	theFrequency = frequency;
}

void SignalType::send(QSocketSession &sink) const
{
	sink.safeSendWord((uint32_t)id());
	serialise(sink);
}

SignalType *SignalType::receive(QSocketSession &source)
{
	SignalType *s = create(source.safeReceiveWord<uint32_t>());
	s->deserialise(source);
	return s;
}

void SignalType::serialise(QSocketSession &sink) const
{
	sink.safeSendWord((uint32_t)theScope);
	sink.safeSendWord(theFrequency);
	sink.safeSendWord(theMin);
	sink.safeSendWord(theMax);
}

void SignalType::deserialise(QSocketSession &source)
{
	theScope = source.safeReceiveWord<int32_t>();
	theFrequency = source.safeReceiveWord<float>();
	theMin = source.safeReceiveWord<float>();
	theMax = source.safeReceiveWord<float>();
}

SignalType *SignalType::create(uint id)
{
	switch (id)
	{
		case 0: return new Value;
		case 1: return new Wave;
		case 2: return new Spectrum;
		case 3: return new Matrix;
		case 4: return new SquareMatrix;
		default: return 0;
	}
}

/*ostream &operator<<(ostream &out, const SignalType &me)
{
	if (me == SignalType::null)
		return out << "null";
	out << "[ " << me.theScope << "x" << me.theFrequency << "Hz - ";
	switch (me.theFormat)
	{	case SignalType::NoFormat: out << "NoFormat"; break;
		case SignalType::Wave: out << "Wave"; break;
		case SignalType::Spectrum: out << "Spectrum"; break;
		case SignalType::Matrix: out << "Matrix"; break;
		case SignalType::Unknown: out << "Unknown"; break;
	}
	return out << " ]";
}*/

}
