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

#pragma once

#include <exscalibar.h>

#include <QVector>
#include <QColor>

#ifdef __GEDDEI_BUILD
#include "contiguous.h"
#else
#include <geddei/contiguous.h>
#endif
using namespace Geddei;

namespace Geddei
{

/** @ingroup SignalTypes
 * @brief A TransmissionType refinement for describing generic single-value data.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be used to define a signal stream that comprises samples
 * of a single element. The meaning of each element is undefined.
 *
 * It is the simplest TransmissionType-derived class.
 */
class DLLEXPORT MultiValue: public Contiguous
{
	TRANSMISSION_TYPE(MultiValue, Contiguous);

public:
	struct Config
	{
		int index;
		QColor fore;
		QColor back;
		float min;
		float max;
		float conversion;
		QString units;

		Config(bool): conversion(0.f) {}
		Config(float _max, float _min = 0.f, int _i = -1): index(_i), fore(Qt::black), back(Qt::transparent), min(std::min(_min, _max)), max(std::max(_min, _max)), conversion(1.f), units(QString::null) {}
		Config(QColor _f = Qt::black, QColor _b = Qt::transparent, float _max = 1.f, float _min = 0.f, int _i = -1, float _c = 1.f, QString const& _u = QString::null): index(_i), fore(_f), back(_b), min(std::min(_min, _max)), max(std::max(_min, _max)), conversion(_c), units(_u) {}
		bool operator==(Config const&) const { return true; }
	};

	/**
	 * The constructor.
	 *
	 * @param frequency The sampling frequency of the values. That is, the
	 * inverse of the delay (in signal time) between each value (sample).
	 *
	 * If there is no clear way of defining this, you may choose to use the
	 * default value of zero, which will serve the purpose of "not applicable".
	 */
	MultiValue(uint _scope = 1, float _frequency = 1, float _max = 1.f, float _min = 0.f, QVector<Config> const& _c = QVector<Config>(), int _l = 0);
	MultiValue(uint _scope, float _frequency, QVector<Config> const& _c, int _l = 0);

/*	inline void setConfig(int _e, Config const& _c) { m_config[_e] = _c; if (_c.index == -1) m_config[_e].index = _e; }
	inline Config config(int _e) const { return m_config[_e]; }
	inline void setConfig(QVector<Config> const& _c) { m_config = _c; normalise(); }
*/	inline QVector<Config> const& config() const { return m_config; }

	inline int labeled() const { return m_labeled; }

	virtual QString info() const { return QString("<div><b>MultiValue</b></div>") + Contiguous::info(); }

protected:
	void updateMM();
	void normalise();

	int m_labeled;
	QVector<Config> m_config;

	TT_2_MEMBERS(m_labeled, m_config);
};

/** @ingroup SignalTypes
 * @brief A TransmissionType refinement for describing generic single-value data.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be used to define a signal stream that comprises samples
 * of a single element. The meaning of each element is undefined.
 *
 * It is the simplest TransmissionType-derived class.
 */
class DLLEXPORT Value: public MultiValue
{
	TRANSMISSION_TYPE(Value, MultiValue);

public:
	typedef MultiValue::Config Config;

	/**
	 * The constructor.
	 *
	 * @param frequency The sampling frequency of the values. That is, the
	 * inverse of the delay (in signal time) between each value (sample).
	 *
	 * If there is no clear way of defining this, you may choose to use the
	 * default value of zero, which will serve the purpose of "not applicable".
	 */
	Value(float _f = 1.f, Config _c = Config()) : MultiValue(1u, _f, QVector<Config>() << _c, 0) {}

	virtual QString info() const { return QString("<div><b>Value</b></div>") + Contiguous::info(); }

	TT_NO_MEMBERS;
};

}
