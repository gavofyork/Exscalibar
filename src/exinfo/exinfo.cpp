/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <cstdlib>
#include <iostream>
using namespace std;

#include "geddei.h"
using namespace Geddei;

int main(int argc, char **argv)
{
	if(argc < 2 || (QString(argv[1]) != "list" && QString(argv[1]) != "version" && QString(argv[1]) != "paths"))
	{	cout << "Usage: exinfo <command> [options]" << endl;
		cout << "Where <command> is one of:" << endl
			 << "   list      List all *Processor plugins found." << endl
			 << "   version   Show version information and build configuration." << endl
			 << "   paths     List all paths used for searching plugins." << endl
			 << endl
			 << "Copyright (c)2005 Gav Wood. This utility is part of Exscalibar." << endl;
		return 1;
	}

	if(QString(argv[1]) == "version")
	{
		cout << "Exscalibar version: " << Geddei::getVersion() << endl;
		cout << "Exscalibar built with features:"
			 << (Geddei::getConfig()&FFTW ? " fftw" : "")
			 << (Geddei::getConfig()&GAT ? " gat" : "")
			 << (Geddei::getConfig()&LIBSNDFILE ? " sndfile" : "")
			 << (Geddei::getConfig()&ALSA ? " alsa" : "")
			 << (Geddei::getConfig()&LIBVORBISFILE ? " vorbisfile" : "")
			 << (Geddei::getConfig()&LIBMAD ? " mad" : "")
			 << endl;
		cout << "(as reported by Geddei)" << endl;
	}
	if(QString(argv[1]) == "paths")
	{
		cout << "Plugin paths searched: " << endl << "   " << qPrintable(getPaths().join("\n   ")) << endl;
	}
	if(QString(argv[1]) == "list")
	{
		if(argc == 3 && QString(argv[2]) != "all" && QString(argv[2]) != "proc" && QString(argv[2]) != "sub")
		{	cout << "Usage: exinfo list [proc|sub|all]" << endl;
			return 1;
		}
		if(argc < 3 || QString(argv[2]) == "all" || QString(argv[2]) == "proc")
		{
			QStringList procs = ProcessorFactory::available();
			cout << procs.count() << " Processors available to Exscalibar:" << endl;
			for(QStringList::iterator i = procs.begin(); i != procs.end(); i++)
				cout << "   " << qPrintable(*i) << " (" << qPrintable(ProcessorFactory::version(*i)) << ")" << endl;
		}
		if(argc < 3 || QString(argv[2]) == "all" || QString(argv[2]) == "sub")
		{
			QStringList procs = SubProcessorFactory::available();
			cout << procs.count() << " SubProcessors available to Exscalibar:" << endl;
			for(QStringList::iterator i = procs.begin(); i != procs.end(); i++)
				cout << "   " << qPrintable(*i) << " (" << qPrintable(SubProcessorFactory::version(*i)) << ")" << endl;
		}
	}
}
