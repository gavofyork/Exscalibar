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

#include "qsocketsession.h"

#include "value.h"

namespace Geddei
{

TRANSMISSION_TYPE_CPP(Value);
TRANSMISSION_TYPE_CPP(MultiValue);

static const Qt::GlobalColor s_colours[] = {
	Qt::black,
	Qt::darkRed,
	Qt::darkGreen,
	Qt::darkBlue,
	Qt::darkCyan,
	Qt::darkMagenta,
	Qt::darkYellow,
	Qt::darkGray,
	Qt::red,
	Qt::green,
	Qt::blue,
	Qt::cyan,
	Qt::magenta,
	Qt::yellow,
	Qt::gray
};

MultiValue::MultiValue(uint _scope, float _frequency, float _max, float _min, QVector<MultiValue::Config> const& _c, int _l):
	Contiguous(_scope, _frequency, _max, _min), m_labeled(_l)
{
	if (_c.isEmpty())
	{
		m_config.resize(_scope);
		for (uint c = 0; c < _scope; c++)
		{
			m_config[c].back = Qt::transparent;
			m_config[c].fore = s_colours[c % 15];
			m_config[c].max = ::max(_min, _max);
			m_config[c].min = ::min(_min, _max);
		}
	}
	else
	{
		m_config = _c;
		updateMM();
	}
	normalise();
}

MultiValue::MultiValue(uint _scope, float _frequency, QVector<MultiValue::Config> const& _c, int _l):
	Contiguous(_scope, _frequency), m_labeled(_l)
{
	m_config = _c;
	updateMM();
	normalise();
}

void MultiValue::normalise()
{
	for (int c = 0; c < m_config.count(); c++)
		m_config[c].index = (m_config[c].index == -1) ? c : m_config[c].index;
}

void MultiValue::updateMM()
{
	theMax = m_config.size() ? m_config[0].max : 1.f;
	theMin = m_config.size() ? m_config[0].min : 1.f;
	for (int i = 1; i < m_config.size(); i++)
	{
		theMax = ::max(theMax, m_config[i].max);
		theMin = ::min(theMin, m_config[i].min);
	}
}
/*
void MultiValue::serialise(QSocketSession &sink) const
{
	TransmissionType::serialise(sink);
	foreach (Config c, m_config)
	{
		sink.safeSendWord((uint32_t)c.fore.rgba());
		sink.safeSendWord((uint32_t)c.back.rgba());
		sink.safeSendWord(c.max);
		sink.safeSendWord(c.min);
		sink.safeSendWord(c.index);
		sink.safeSendWord(c.conversion);
		sink.sendString(c.units.toUtf8());
	}
}

void MultiValue::deserialise(QSocketSession &source)
{
	TransmissionType::deserialise(source);
	foreach (Config c, m_config)
	{
		c.fore.setRgba(source.safeReceiveWord<QRgb>());
		c.back.setRgba(source.safeReceiveWord<QRgb>());
		c.max = source.safeReceiveWord<float>();
		c.min = source.safeReceiveWord<float>();
		c.index = source.safeReceiveWord<int>();
		c.conversion = source.safeReceiveWord<float>();
		c.units = QString::fromUtf8(source.receiveString());
	}
}
*/
}

