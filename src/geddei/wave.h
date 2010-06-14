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
#include "contiguous.h"
#else
#include <geddei/contiguous.h>
#endif
using namespace Geddei;

namespace Geddei
{

/** @ingroup SignalTypes
 * @brief A TransmissionType refinement for describing time-domain wave offset data.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be used to define a signal stream that comprises samples
 * of a single element, where each element represents an offset from a defined
 * zero.
 */
class DLLEXPORT Wave: public Contiguous
{
	TRANSMISSION_TYPE(Wave, Contiguous);

public:
	/**
	 * The constructor.
	 *
	 * @param frequency The sampling frequency of the Wave. That is, the
	 * inverse of the delay between each sample. e.g. CD audio has a sample
	 * frequency of 44100, i.e. each sample represents a reading in time of
	 * 1/44100th of a second later than the last.
	 */
	inline Wave(float _frequency = 0.f, float _max = 1.f, float _min = -1.f) : Contiguous(1, _frequency, _max, _min) {}

	virtual QString info() const { return QString("<div><b>Wave</b></div>") + Contiguous::info(); }

	TT_NO_MEMBERS;
};

class DLLEXPORT WaveChunk: public Contiguous
{
	TRANSMISSION_TYPE(WaveChunk, Contiguous);

public:
	/**
	 * The constructor.
	 *
	 * @param frequency The sampling frequency of the Wave. That is, the
	 * inverse of the delay between each sample. e.g. CD audio has a sample
	 * frequency of 44100, i.e. each sample represents a reading in time of
	 * 1/44100th of a second later than the last.
	 */
	inline WaveChunk(uint _length = 0, float _frequency = 1.f, float _rate = 1.f, float _max = 1.f, float _min = -1.f) : Contiguous(_length, _frequency, _max, _min), m_rate(_rate) {}

	virtual QString info() const { return QString("<div><b>WaveChunk</b></div><div>Rate: %1 Hz</div>").arg(m_rate) + Contiguous::info(); }

	inline uint length() const { return arity(); }
	inline void setLength(uint _l) { setArity(_l); }

	inline float rate() const { return m_rate; }
	inline void setRate(float _r) { m_rate = _r; }

private:
	float m_rate;

	TT_1_MEMBER(m_rate);
};

}
