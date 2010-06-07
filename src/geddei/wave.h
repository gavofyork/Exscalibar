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
 * @brief A TransmissionType refinement for describing time-domain wave offset data.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be used to define a signal stream that comprises samples
 * of a single element, where each element represents an offset from a defined
 * zero.
 */
class DLLEXPORT Wave: public Signal
{
	TRANSMISSION_TYPE(Wave, Signal);

public:
	/**
	 * The constructor.
	 *
	 * @param frequency The sampling frequency of the Wave. That is, the
	 * inverse of the delay between each sample. e.g. CD audio has a sample
	 * frequency of 44100, i.e. each sample represents a reading in time of
	 * 1/44100th of a second later than the last.
	 */
	Wave(float frequency = 0) : Signal(1, frequency, 1.f, -1.f) {}

	virtual QString info() const { return QString("<div><b>Wave</b></div>") + Signal::info(); }

	TT_NO_MEMBERS;
};

}
