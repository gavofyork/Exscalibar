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

#include <sys/time.h>

#include "rdtsc.h"
using namespace QtExtra;

namespace QtExtra
{

static realTime getRdtscTPS()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	unsigned long long tsn = ts.tv_sec * 1000000000ull + ts.tv_nsec;
	unsigned long long ret = rdtsc();
	while ((ts.tv_sec * 1000000000ull + ts.tv_nsec) - tsn < 100000000ull)
		clock_gettime(CLOCK_MONOTONIC, &ts);
	return (rdtsc() - ret) * 10ull;
}

DLLEXPORT realTime g_rdtscTPS = getRdtscTPS();

}
