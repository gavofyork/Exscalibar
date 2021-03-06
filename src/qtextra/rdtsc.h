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

namespace QtExtra
{

typedef unsigned long long realTime;

inline realTime rdtsc()
{
	unsigned long low;
	unsigned long high;
	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
	return ((unsigned long long)high << 32) | low;
}

extern realTime g_rdtscTPS;

inline double rdtscElapsed(realTime _start)
{
	return double(rdtsc() - _start) / double(g_rdtscTPS);
}

}
