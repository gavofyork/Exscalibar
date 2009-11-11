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

#pragma once

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "signaltype.h"
#else
#include <geddei/signaltype.h>
#endif
using namespace Geddei;

namespace SignalTypes
{

/** @ingroup SignalTypes
 * @brief A SignalType refinement for describing generic single-value data.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be used to define a signal stream that comprises samples
 * of a single element. The meaning of each element is undefined.
 *
 * It is the simplest SignalType-derived class.
 */
class DLLEXPORT Value: public SignalType
{
	virtual uint id() const { return 0; }
	virtual SignalType *copyBE() const { return new Value(theFrequency); }

public:
	/**
	 * The constructor.
	 *
	 * @param frequency The sampling frequency of the values. That is, the
	 * inverse of the delay (in signal time) between each value (sample).
	 *
	 * If there is no clear way of defining this, you may choose to use the
	 * default value of zero, which will serve the purpose of "not applicable".
	 */
	Value(float frequency = 0, float _max = 1.f, float _min = 0.f) : SignalType(1, frequency, _max, _min) {}
};

}
