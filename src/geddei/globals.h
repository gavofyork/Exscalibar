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

#include <stdint.h>

#include <limits>
#include <cmath>
#include <cassert>

#include <QtAlgorithms>
#include <QThread>
#include <QVector>
#include <QStringList>

#include <exscalibar.h>

namespace Geddei
{
	enum { FFTW = 1, GAT = 2, LIBSNDFILE = 4, ALSA = 8, LIBVORBISFILE = 16, LIBMAD = 32 };
	enum MultiplicityType { NotMulti = 0, In = 1, Out = 2, InOut = 3, Const = 4, InConst = 5, OutConst = 6, InOutConst = 7, Hetero = 8 };
	enum { SubNonInplace = 0, SubInplace = 1 };
	static uint Undefined = (uint)-1;

	static const float StreamFalse = -std::numeric_limits<float>::infinity();
	static const float StreamTrue = std::numeric_limits<float>::infinity();

	inline void StreamDummy()
	{
		(void)StreamFalse;
		(void)StreamTrue;
	}

	DLLEXPORT void sleep(uint secs);
	DLLEXPORT void usleep(uint usecs);

	inline uint ceillog2(uint i)
	{
		for (uint l = 0; ; l++)
			if (i <= uint(1 << l))
				return l;
	}
	template<class T>
	inline T quantised(T _i, T _s)
	{
		return floor(_i / _s) * _s;
	}
	template<class T>
	inline T clamp(T _a, T _l, T _u)
	{
		return _a > _u ? _u : _a < _l ? _l : _a;
	}
	template<class T>
	inline T lerp(T _a, T _b, T _x)
	{
		return (_b - _a) * _x + _a;
	}
	template<class T>
	inline T lerp(T _da, T _db, T _sa, T _sb)
	{
		return (_db - _da) * _sb / (_sa + _sb) + _da;
	}
	template<class T>
	inline T sqr(T _x)
	{
		return _x * _x;
	}
	template<class T>
	inline T cube(T _x)
	{
		return _x * _x * _x;
	}
	template<class T>
	inline T normalPDFN(T _x, T _m, T _o)
	{
		return exp(sqr(_x - _m)/(-2.0 * sqr(_o)));
	}
	template<class T>
	inline T normalPDFN(T _x)
	{
		return exp(-0.5*sqr(_x));
	}
	template<class T>
	inline T normalPDF(T _x, T _m, T _o)
	{
		return exp(sqr(_x - _m)/(-2.0 * sqr(_o))) / sqrt(2.0 * M_PI * sqr(_o));
	}
	template<class T>
	inline T normalCDF(T _x, T _m, T _o)
	{
		return 0.5 * (1 + erf((_x - _m) / (M_SQRT2 * _o)));
	}
	inline int isInf(float _x)
	{
		union { float f; int i; } u;
		u.f = _x;
		if (u.i == 2139095040)
			return 1;
		if (u.i == -8388608)
			return -1;
		return 0;
	}
	inline bool isFinite(float _x)
	{
		return (*(uint32_t const*)(&_x)) != 0x7fc00000 && (*(uint32_t const*)(&_x)) != 0xff800000 && (*(uint32_t const*)(&_x)) != 0x7f800000;
	}

	template<class T>
	bool divides(T _n, T _d, T _e)
	{
		T dummy;
		T x = modf(_n / _d, &dummy);
		if (x > .5)
			return 1.0 - x < _e;
		else
			return x < _e;
	}
	template <typename RandomAccessIterator, typename LessThan>
	inline bool isSorted(RandomAccessIterator _start, RandomAccessIterator _end, LessThan _lessThan)
	{
		if (_start != _end)
		{
			RandomAccessIterator lit = _start;
			RandomAccessIterator it = lit;
			for (it++; it != _end; ++it, ++lit)
				if (!_lessThan(*lit, *it))
					return false;
		}
		return true;
	}
	template<class T>
	static T graphParameters(T _min, T _max, T _divisions, T* o_from = 0, T* o_delta = 0, bool _forceMinor = false)
	{
		T uMin = _min;
		T uMax = _max;
		T l10 = log10((uMax - uMin) / _divisions * 5.5f);
		T mt = pow(10.f, l10 - floor(l10));
		T ep = pow(10.f, floor(l10));
		T inc = _forceMinor
				? ((mt > 6.f) ? ep / 2.f : (mt > 3.f) ? ep / 5.f : (mt > 1.2f) ? ep / 10.f : ep / 20.f)
				: ((mt > 6.f) ? ep * 2.f : (mt > 3.f) ? ep : (mt > 1.2f) ? ep / 2.f : ep / 5.f);
		if (o_delta && o_from)
		{
			(*o_from) = floor(uMin / inc) * inc;
			(*o_delta) = (ceil(uMax / inc) - floor(uMin / inc)) * inc;
		}
		else if (o_from)
		{
			(*o_from) = ceil(uMin / inc) * inc;
		}
		return inc;
	}

	template<class T>
	inline T interpolateIndex(QVector<T> const& _l, float _i)
	{
		if (_i <= 0) return _l.first();
		if (_i >= _l.count() - 1) return _l.last();
		return lerp<T>(_l[(int)floor(_i)], _l[(int)floor(_i) + 1], _i - floor(_i));
	}
	template<class T>
	inline T interpolateIndex(T const* _d, int _len, float _i)
	{
		if (_i <= 0) return _d[0];
		if (_i >= _len - 1) return _d[_len - 1];
		return lerp<T>(_d[(int)floor(_i)], _d[(int)floor(_i) + 1], _i - floor(_i));
	}

	template<class T>
	static inline T cubicInterpolateIndex(T const* _data, int _len, T _index)
	{
		int fi = (int)floor(_index);
		T pi = _index - floor(_index);
		if (pi == 0)
			return _data[fi];
		if (fi >= _len - 1)
			return _data[_len - 1];
		if (fi <= 0)
			return _data[0];

		T m = _data[fi + 1] - _data[fi];
		T mi = fi ? (_data[fi + 1] - _data[fi - 1]) / 2.f : m;
		T mia1 = (fi < _len - 2) ? (_data[fi + 2] - _data[fi]) / 2.f : m;
		T pi2 = sqr(pi);
		T pi3 = cube(pi);

		return	_data[fi] * (1.f - 3.f * pi2 + 2.f * pi3) + mi * (pi - 2.f * pi2 + pi3) + _data[fi + 1] * (3.f * pi2 - 2.f * pi3) - mia1 * (pi2 - pi3);
	}

	template<class T>
	inline T interpolateValue(T const* _d, int _len, float _v)
	{
		if (_v <= _d[0])
			return 0;
		if (_v >= _d[_len - 1])
			return _len - 1;
		T const* i = qLowerBound(_d, _d + _len, _v);
		if (*i == _v)
			return i - _d;
		assert(*i > _v);
		assert(i != _d);
		return (i - _d) + (_v - *i) / (*i - *(i - 1));
	}
	template<class T>
	inline T interpolateValue(QVector<T> const& _l, float _v)
	{
		if (_v <= _l.first())
			return 0;
		if (_v >= _l.last())
			return _l.count() - 1;
		typename QVector<T>::ConstIterator i = qLowerBound(_l.begin(), _l.end(), _v);
		if (*i == _v)
			return i - _l.begin();
		assert(*i > _v);
		assert(i != _l.begin());
		return (i - _l.begin()) + (_v - *i) / (*i - *(i - 1));
	}

	DLLEXPORT const char *getVersion();
	DLLEXPORT uint getConfig();
	DLLEXPORT QStringList getPaths();
}
