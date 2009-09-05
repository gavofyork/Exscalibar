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

#include <iostream>
using namespace std;

#include "geddei.h"
using namespace Geddei;

#include "monitor.h"

int main(int argc, char **argv)
{
/*	QStringList procs = ProcessorFactory::available();
	cout << "Processors:" << endl;
	for (QStringList::iterator i = procs.begin(); i != procs.end(); i++)
		cout << "   " << *i << " (" << ProcessorFactory::version(*i) << ")" << endl;
	procs = SubProcessorFactory::available();
	cout << "SubProcessors:" << endl;
	for (QStringList::iterator i = procs.begin(); i != procs.end(); i++)
		cout << "   " << *i << " (" << SubProcessorFactory::version(*i) << ")" << endl;
*/
	ProcessorGroup objects;
	ProcessorFactory::create("Player")->init("O", objects, Properties("Filename", argc < 2 ? "/tmp/test2.wav" : argv[1]));
	SubProcessorFactory::createDom("Mean")->init("M", objects, Properties("Multiplicity", 2));
	SubProcessorFactory::createDom("FFT")->init("F", objects, Properties("Debug", true));
	SubProcessorFactory::createDom("DownSample")->init("D", objects);

//	for (int i = 0; i < 10; i++)
	{
		Monitor *I = (new Monitor);
		I->init("I", objects);
		
		cout << "Connecting..." << endl;
		objects["O"][0] >>= objects["M"][0];
		objects["O"][1] >>= objects["M"][1];
		objects["M"][0] >>= objects["D"][0];
		objects["D"][0] >>= objects["F"][0];
		objects["F"][0] >>= objects["I"][0];

		cout << "Starting objects..." << endl;
		objects.go();

		cout << "Waiting for a little bit..." << endl;
		I->waitUntilDone();

		cout << "Stopping objects..." << endl;
		objects.stop();

		cout << "Disconnecting..." << endl;
		objects.disconnectAll();
		
		cout << "Monitor: " << I->elementsProcessed() << endl << endl << endl << endl;
		delete I;
	}
	
	objects.deleteAll();
}
