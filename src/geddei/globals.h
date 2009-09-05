/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_GLOBALS_H
#define _GEDDEI_GLOBALS_H

#include <qthread.h>
#include <qstringlist.h>

#include <exscalibar.h>

namespace Geddei
{
	
	enum { FFTW = 1, GAT = 2, LIBSNDFILE = 4, ALSA = 8, LIBVORBISFILE = 16, LIBMAD = 32 };
	enum MultiplicityType { NotMulti = 0, In = 1, Out = 2, InOut = 3, Const = 4, InConst = 5, OutConst = 6, InOutConst = 7 };
	static uint Undefined = (uint)-1;
	
	DLLEXPORT void sleep(uint secs);
	DLLEXPORT void usleep(uint usecs);
	
	inline uint ceillog2(uint i)
	{
		for (uint l = 0; ; l++)
			if (i <= uint(1 << l))
				return l;
	}
	
	DLLEXPORT const char *getVersion();
	DLLEXPORT uint getConfig();
	DLLEXPORT const QStringList getPaths();
	
}

#endif
