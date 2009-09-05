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

#include <qdatetime.h>
#include "qsubapp.h"

#include "rgeddei.h"
using namespace rGeddei;

#include "monitor.h"

class MySubApp: public QSubApp { void main(); } subApp;

// Should be up here since we want it running in the main thread along with the QApplication.
LocalSession local("192.168.0.85");

void MySubApp::main()
{
	RemoteSession strength("192.168.0.2");

	Monitor monitor;
	AbstractProcessorGroup group;
	AbstractProcessor::create(local, "Player")->
		init("O", group, Properties("Filename", "/home/gav/Stuff/audio/short.wav"));
	AbstractDomProcessor::create(local, "FFT")->
		init("F", group, Properties("Size", 512)("Step", 256));
	AbstractDomProcessor::create(local, "SelfSimilarity")->
		init("S", group, Properties("Size", 32)("Step", 16));
	AbstractDomProcessor::create(local, "DiagonalSum")->
		init("D", group);
	AbstractProcessor::create(local, monitor)->
		init("I", group);
	AbstractProcessor::create(local, "Recorder")->
		init("R", group, Properties("Output", "/home/gav/analysis.csv"));

	group["O"][0] >>= group["F"][0];
	group["F"][0] >>= group["S"][0];
	group["S"][0] >>= group["I"][0];
	group["I"][0] >>= group["D"][0];
	group["D"][0] >>= group["R"][0];

	if (!group.go()) qFatal("Problem starting processors. Bailing.");
	monitor.waitUntilDone();
	
	group.stop();
	group.disconnectAll();
	group.deleteAll();
	
	cerr << "Total signal processed: " << monitor.signalProcessed() << " seconds (" << monitor.samplesProcessed() << " samples)." << endl;
	cerr << "Time elapsed: " << monitor.elapsedTime() << " seconds." << endl;
	cerr << "Averge processing speed: " << monitor.timesFasterThanReal() << " xRT." << endl;
	cerr << "Average throughput: " << monitor.averageThroughput() << " Mbps." << endl;
}

