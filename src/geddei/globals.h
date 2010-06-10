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

#include <limits>
#include <cmath>

#include <qthread.h>
#include <qstringlist.h>

#include <exscalibar.h>

namespace Geddei
{
	enum { FFTW = 1, GAT = 2, LIBSNDFILE = 4, ALSA = 8, LIBVORBISFILE = 16, LIBMAD = 32 };
	enum MultiplicityType { NotMulti = 0, In = 1, Out = 2, InOut = 3, Const = 4, InConst = 5, OutConst = 6, InOutConst = 7, Hetero = 8 };
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
	inline T sqr(T _x)
	{
		return _x * _x;
	}
	template<class T>
	inline T normalPDFN(T _x, T _m, T _o)
	{
		return exp(sqr(_x - _m)/(-2.0 * _o * _o));
	}
	template<class T>
	inline T normalPDF(T _x, T _m, T _o)
	{
		return exp(sqr(_x - _m)/(-2.0 * _o * _o)) / sqrt(2.0 * M_PI * _o * _o);
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

	DLLEXPORT const char *getVersion();
	DLLEXPORT uint getConfig();
	DLLEXPORT QStringList getPaths();
}
