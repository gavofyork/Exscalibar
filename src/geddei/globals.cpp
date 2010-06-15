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

QStringList getPaths()
{
	QStringList ret;
#ifdef DEBUG
	ret += ".";
#endif
	if (std::getenv("GEDDEI_PLUGINS"))
		ret += QString(std::getenv("GEDDEI_PLUGINS")).split(":");
	if (std::getenv("EXSCALIBAR_LOCATION"))
		ret += QString(std::getenv("EXSCALIBAR_LOCATION")) + "/plugins/geddei";
	QStringList thePaths(QApplication::libraryPaths());
	for (QStringList::Iterator i = thePaths.begin(); i != thePaths.end(); i++)
		ret += ((*i) + "/geddei");
	return ret;
}

};
