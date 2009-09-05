/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <cstdlib>

#include <qapplication.h>

#include "exscalibar.h"

#include "globals.h"

namespace Geddei
{

static const char *theVersion = EXSCALIBAR_VERSION;

class Sleeper : public QThread
{
public:
	static void sleep(uint secs) { QThread::sleep(secs); }
	static void usleep(uint usecs) { QThread::usleep(usecs); }
};

void sleep(uint secs)
{
	Sleeper::sleep(secs);
}

void usleep(uint usecs)
{
	Sleeper::usleep(usecs);
}

const char *getVersion()
{
	return theVersion;
}

uint getConfig()
{
	return 0
#ifdef HAVE_FFTW
	|FFTW
#endif
#ifdef HAVE_GAT
	|GAT
#endif
#ifdef HAVE_SNDFILE
	|LIBSNDFILE
#endif
#ifdef HAVE_ALSA
	|ALSA
#endif
#ifdef HAVE_MAD
	|LIBMAD
#endif
#ifdef HAVE_VORBISFILE
	|LIBVORBISFILE
#endif
	;
}

const QStringList getPaths()
{
	QStringList ret;
#ifdef DEBUG
	ret += ".";
#endif
	if(std::getenv("GEDDEI_PLUGINS"))
		ret += QStringList::split(":", std::getenv("GEDDEI_PLUGINS"));
	if(std::getenv("EXSCALIBAR_LOCATION"))
		ret += QString(std::getenv("EXSCALIBAR_LOCATION")) + "/plugins/geddei";
	QStringList thePaths(QApplication::libraryPaths());
	for(QStringList::Iterator i = thePaths.begin(); i != thePaths.end(); i++)
		ret += ((*i) + "/geddei");
	return ret;
}

};
