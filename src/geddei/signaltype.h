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

#include <stdint.h>
#include <iostream>
using namespace std;

#include <qglobal.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "transmissiontype.h"
#else
#include <geddei/transmissiontype.h>
#endif

class QSocketSession;

namespace Geddei
{

class DLLEXPORT Signal: public TransmissionType
{
	TRANSMISSION_TYPE(Signal, TransmissionType);

public:
	/**
	 * Basic constructor.
	 *
	 * @param sampleSize The sampleSize of the signal. That is, how many elements (single
	 * values) is takes to adequatly define a single reading, or sample. For a
	 * simple time-based offset single, this will be 1, but for more complex
	 * signals such as spectra, matrices or whatever, this will be more.
	 * @param frequency The sampling frequency of the signal. That is, the
	 * inverse of the delay between each sample. e.g. CD audio has a sample
	 * frequency of 44100, i.e. each sample represents a reading in time of
	 * 1/44100th of a second later than the last.
	 */
	Signal(uint sampleSize = 1u, float frequency = 0.f, float _max = 1.f, float _min = 0.f);

	/**
	 * Virtual destructor.
	 */
	virtual ~Signal() {}

	virtual QString info() const;

	float minAmplitude() const { return theMin; }
	float maxAmplitude() const { return theMax; }
	void setRange(float _max, float _min) { theMin = min(_min, _max); theMax = max(_min, _max); }

	/**
	 * @return The frequency, or number of samples per Signal-time second of
	 * the data represented by this TransmissionType.
	 */
	float frequency() const { return theFrequency; }

	/**
	 * Sets the frequency of this TransmissionType.
	 *
	 * @param frequency The new frequency.
	 */
	void setFrequency(float frequency) { theFrequency = frequency; }

	/**
	 * Get the number of seconds represented by the given number of elements.
	 *
	 * @param elements The number of elements.
	 * @return The number of Signal-time seconds that @a elements constitute.
	 */
	float seconds(uint elements) const { return float(elements / size()) / theFrequency; }

	/**
	 * Get the number of elements a Signal-time length of seconds represents.
	 *
	 * @param seconds The length of Signal-time.
	 * @return The number of elements.
	 */
	uint elementsFromSeconds(float seconds) const { return (uint)(seconds * theFrequency) * size(); }

protected:
	/**
	 * Reciprocal of the amount of time between this sample and the next.
	 */
	float theFrequency;

	float theMin;
	float theMax;

	TT_3_MEMBERS(theFrequency, theMin, theMax);
};

/**
 * All Mark samples are of sampleSize 2 + the size.
 *
 * The two samples on the front are coalesced to form a double which gives the number of
 * seconds since the last plunger that this Mark corresponds to. These two are automatically
 * filled by the near side Connection.
 */
class DLLEXPORT Mark: public TransmissionType
{
	TRANSMISSION_TYPE(Mark, TransmissionType);

public:
	Mark(uint _arity = 0): TransmissionType(_arity + 2) {}

	/**
	 * Virtual destructor.
	 */
	virtual ~Mark() {}

	virtual QString info() const;

	virtual uint reserved() const { return 2u; }
	virtual void polishData(BufferData&, Source*, uint) const;
	static void setTimestamp(BufferData& _data, double _ts);
	static double timestamp(BufferData const& _data);

	TT_NO_MEMBERS;
};

}
