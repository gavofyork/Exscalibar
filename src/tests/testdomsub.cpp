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

#include <cassert>
#include <iostream>
using namespace std;

#include <q3valuevector.h>
#include <qthread.h>

#include "processor.h"
#include "subprocessorfactory.h"
#include "domprocessor.h"
#include "processorgroup.h"
#include "bufferdatas.h"
#include "signaltypeptrs.h"
using namespace geddei;

#include "wave.h"
using namespace signaltypes;

#include "monitor.h"
#include "multiplayer.h"

int main()
{
#ifdef HAVE_LIBSNDFILE1

	ProcessorGroup group;

	MultiPlayer O;
	DomProcessor &X = *SubProcessorFactory::createDom("DownSample");
	X.createAndAddWorker();
	Monitor M;

	Properties p;
	p["Filenames"].asStringList() += "/tmp/1.wav";
	p["Filenames"].asStringList() += "/tmp/2.wav";
	O.init("O", group, p);
	X.init("X", group, Properties("Divisor", 2));
	M.init("M", group);

	O.connect(0, &X, 0, 262144);
	X.connect(0, &M, 0, 262144);

	group.go();
	M.waitForPlunger();
	std::cout << "Signal Processed: " << M.signalProcessed() << "s" << std::endl;
	M.waitForPlunger();
	std::cout << "Signal Processed: " << M.signalProcessed() << "s" << std::endl;
	group.stop();
	std::cout << "Average Throughput: " << M.averageThroughput() << "e/s" << std::endl;
#endif
}


