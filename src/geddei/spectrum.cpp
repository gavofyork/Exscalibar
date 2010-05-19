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

#include "spectrum.h"

namespace SignalTypes
{

Spectrum::Spectrum(uint size, float frequency, float _max, float _min):
	SignalType(size, frequency, _max, _min)
{
}

bool FreqSteppedSpectrum::sameAsBE(const SignalType *cmp) const
{
	return SignalType::sameAsBE(cmp) && dynamic_cast<const FreqSteppedSpectrum *>(cmp)->theStep == theStep;
}

void FreqSteppedSpectrum::serialise(QSocketSession &sink) const
{
	SignalType::serialise(sink);
	sink.safeSendWord(theStep);
}

void FreqSteppedSpectrum::deserialise(QSocketSession &source)
{
	SignalType::deserialise(source);
	theStep = source.safeReceiveWord<float>();
}

bool PeriodSteppedSpectrum::sameAsBE(const SignalType *cmp) const
{
	return SignalType::sameAsBE(cmp) && dynamic_cast<const PeriodSteppedSpectrum *>(cmp)->theStep == theStep;
}

void PeriodSteppedSpectrum::serialise(QSocketSession &sink) const
{
	SignalType::serialise(sink);
	sink.safeSendWord(theStep);
}

void PeriodSteppedSpectrum::deserialise(QSocketSession &source)
{
	SignalType::deserialise(source);
	theStep = source.safeReceiveWord<float>();
}

}
