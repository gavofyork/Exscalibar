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

#include "contiguous.h"
using namespace Geddei;

namespace Geddei
{

TRANSMISSION_TYPE_CPP(Contiguous);

Contiguous::Contiguous(uint _arity, float _frequency, float _max, float _min):
	TransmissionType	(_arity),
	theFrequency		(_frequency),
	theMin				(::min(_min, _max)),
	theMax				(::max(_min, _max))
{
}

QString Contiguous::info() const
{
	return QString("<div><b>Contiguous</b></div><div>Frequency: %1 Hz</div><div>Range: %3-%4</div>").arg(theFrequency).arg(theMin).arg(theMax) + TransmissionType::info();
}

}
