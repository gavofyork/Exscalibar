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
		objects.get("O")[0] >>= objects.get("M")[0];
		objects.get("O")[1] >>= objects.get("M")[1];
		objects.get("M")[0] >>= objects.get("D")[0];
		objects.get("D")[0] >>= objects.get("F")[0];
		objects.get("F")[0] >>= objects.get("I")[0];

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
