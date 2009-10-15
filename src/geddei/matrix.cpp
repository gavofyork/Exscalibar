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

#include "qsocketsession.h"

#include "matrix.h"

namespace SignalTypes
{

bool Matrix::sameAsBE(const SignalType *cmp) const
{
	return SignalType::sameAsBE(cmp) &&
		theWidth == dynamic_cast<const Matrix *>(cmp)->theWidth &&
		theHeight == dynamic_cast<const Matrix *>(cmp)->theHeight &&
		thePitchWidth == dynamic_cast<const Matrix *>(cmp)->thePitchWidth &&
		thePitchHeight == dynamic_cast<const Matrix *>(cmp)->thePitchHeight;
}

void Matrix::serialise(QSocketSession &sink) const
{
	SignalType::serialise(sink);
	sink.safeSendWord((uint32_t)theWidth);
	sink.safeSendWord((uint32_t)theHeight);
}

void Matrix::deserialise(QSocketSession &source)
{
	SignalType::deserialise(source);
	theWidth = source.safeReceiveWord<uint32_t>();
	theHeight = source.safeReceiveWord<uint32_t>();
}

}
