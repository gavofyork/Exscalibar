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

#include <limits>
#include <cmath>

#include <qthread.h>
#include <qstringlist.h>

#include <exscalibar.h>

namespace Geddei
{
	enum { FFTW = 1, GAT = 2, LIBSNDFILE = 4, ALSA = 8, LIBVORBISFILE = 16, LIBMAD = 32 };
	enum MultiplicityType { NotMulti = 0, In = 1, Out = 2, InOut = 3, Const = 4, InConst = 5, OutConst = 6, InOutConst = 7 };
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
	inline float lerp(float _a, float _b, float _x)
	{
		return (_b - _a) * _x + _a;
	}
	inline float sqr(float _x)
	{
		return _x * _x;
	}
	inline float normalPDFN(float _x, float _m, float _o)
	{
		return exp(sqr(_x - _m)/(-2.f * _o * _o));
	}
	inline float normalPDF(float _x, float _m, float _o)
	{
		return exp(sqr(_x - _m)/(-2.f * _o * _o)) / sqrtf(2.f * M_PI * _o * _o);
	}
	inline float normalCDF(float _x, float _m, float _o)
	{
		return .5f * (1 + erff((_x - _m) / (M_SQRT2 * _o)));
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

	DLLEXPORT const char *getVersion();
	DLLEXPORT uint getConfig();
	DLLEXPORT QStringList getPaths();
}
