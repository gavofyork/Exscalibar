/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@kde.org                                                     *
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

#include <qdatetime.h>
#include "qsubapp.h"

#include "abstractprocessorgroup.h"
#include "remoteprocessor.h"
#include "remotesession.h"
#include "localsession.h"
#include "localprocessor.h"
#include "localdomprocessor.h"
#include "remotedomprocessor.h"
using namespace rGeddei;

#include "monitor.h"

// Should be up here since we want it running in the main thread along with the QApplication.
LocalSession local("192.168.0.2");

class MySubApp: public QSubApp { void main(); } subApp;

void MySubApp::main()
{
	RemoteSession strength("192.168.0.2");

	AbstractProcessorGroup g;

	AbstractProcessor::create(local, "Player")->
		init("O", g, Properties("Filename", "/tmp/test.wav"));

	AbstractDomProcessor::create(local, "FFT")->
		init("F", g, Properties("Size", 2048)("Step", 1024));
//	g.dom("F").spawnWorker(local);

	AbstractDomProcessor::create(local, "SelfSimilarity")->
		init("X", g);
	g.dom("X").spawnWorker(local);
	g.dom("X").spawnWorker(local);

/*	AbstractProcessor::create(local, "DiagonalSum")->
		init("D", g);
*/
	Monitor monitor;
	AbstractProcessor::create(local, monitor)->
		init("I", g);

	g["O"][0] >>= g["F"][0];
	g["F"][0] >>= /*g["B"][0];
	g["B"][0] >>= */g["X"][0];
	g["X"][0] >>= /*g["D"][0];
	g["D"][0] >>= */g["I"][0];

	if (!g.go()) qFatal("Problem starting processors. Bailing.");
	monitor.waitForPlunger();
	g.stop();

	g.disconnectAll();
	g.deleteAll();
	
	cerr << "Total signal processed: " << monitor.signalProcessed() << " seconds (" << monitor.samplesProcessed() << " samples)." << endl;
	cerr << "Time elapsed: " << monitor.elapsedTime() << " seconds." << endl;
	cerr << "Averge processing speed: " << monitor.timesFasterThanReal() << " xRT." << endl;
	cerr << "Average throughput: " << monitor.averageThroughput() << " Mbps." << endl;
}

