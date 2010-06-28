/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

namespace Geddei
{

TRANSMISSION_TYPE_CPP(Mark);
TRANSMISSION_TYPE_CPP(SpectralPeak);

Mark::Mark(uint _arity, QVector<float> const& _maxs, QVector<float> const& _mins):
	TransmissionType(_arity + 2), m_mins(_arity), m_maxs(_arity)
{
	for (uint i = 0; i < _arity; i++)
	{
		m_maxs[i] = 1.f;
		m_mins[i] = 0.f;
		if ((int)i < _maxs.size())
			if ((int)i < _mins.size())
			{
				m_mins[i] = ::min(_mins[i], _maxs[i]);
				m_maxs[i] = ::max(_mins[i], _maxs[i]);
			}
			else
			{
				((_maxs[i] > 0.f) ? m_maxs[i] : m_mins[i]) = _maxs[i];
			}
		else if ((int)i < _mins.size())
		{
			((_mins[i] > 0.f) ? m_maxs[i] : m_mins[i]) = _mins[i];
		}
	}
}

void Mark::initData(BufferData const& _d, Source*, uint) const
{
	setTimestamp(const_cast<BufferData&>(_d), numeric_limits<double>::infinity());
}

void Mark::polishData(BufferData const& _d, Source* _s, uint) const
{
/*	for (uint i = 0; i < _d.samples(); i++)
		if (_d(i, 0) > 800)
			qDebug() << "POLISHDATA: CRAZY SAMPLE" << _d(i, 0) << _d(i, 1);*/
	for (uint i = 0; i < _d.samples(); i++)
		if (isInf(timestamp(_d.sample(i))) == 1)
			setTimestamp(const_cast<BufferData&>(_d).sample(i), _s->secondsPassed());
}

void Mark::setTimestamp(BufferData _d, double _ts)
{
	if (_d.isNull())
		return;
	union { double d; float f[2]; } ts;
	ts.d = _ts;
	for (uint s = _d.sampleSize() - 2; s < _d.elements(); s += _d.sampleSize())
		(_d[s] = ts.f[0]), (_d[s + 1] = ts.f[1]);
}

bool Mark::isEndOfTime(BufferData const& _d)
{
	return isInf(timestamp(_d)) == -1;
}

void Mark::setEndOfTime(BufferData _d)
{
	if (_d.isNull())
		return;
	for (uint s = 0; s < _d.elements(); s++)
		_d[s] = 0;
	setTimestamp(_d, -numeric_limits<double>::infinity());
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
	return QString("<div><b>Mark</b></div><div>Range: %s - %s</div>").arg(arity() ? m_mins[0] : 0).arg(arity() ? m_maxs[0] : 0) + TransmissionType::info();
}

}
