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
#include "transmissiontype.h"
#else
#include <geddei/transmissiontype.h>
#endif
using namespace Geddei;

namespace Geddei
{

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
	Mark(uint _arity = 0, QVector<float> const& _maxs = QVector<float>(), QVector<float> const& _mins = QVector<float>());

	/**
	 * Virtual destructor.
	 */
	virtual ~Mark() {}

	virtual QString info() const;

	virtual uint reserved() const { return 2u; }
	virtual void polishData(BufferData&, Source*, uint) const;
	static void setTimestamp(BufferData& _data, double _ts);
	static double timestamp(BufferData const& _data);

	float min(uint _i) const { return (_i < arity()) ? m_mins[_i] : 0.f; }
	float max(uint _i) const { return (_i < arity()) ? m_maxs[_i] : 1.f; }
	void setRange(uint _i, float _one, float _other) { if (_i < arity()) { m_mins[_i] = ::min(_one, _other); m_maxs[_i] = ::max(_one, _other); } }

	QVector<float> m_mins;
	QVector<float> m_maxs;

	TT_2_MEMBERS(m_mins, m_maxs);
};

}
