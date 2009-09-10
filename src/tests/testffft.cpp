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

#include <q3valuevector.h>
#include <qthread.h>

#include "processor.h"
#include "processorgroup.h"
#include "subprocessorfactory.h"
#include "subprocessor.h"
#include "domprocessor.h"
#include "bufferdatas.h"
#include "signaltypeptrs.h"
using namespace geddei;

#include "player.h"
#include "monitor.h"

int main(int argc, char **argv)
{
	int w = 0;
	double c = .5;
	if (argc > 1) w = QString(argv[1]).toInt();
	if (argc > 2) c = QString(argv[2]).toDouble();
	ProcessorGroup g;
	Player P;
	P.init("P", g, Properties("Filename", "/tmp/test.wav"));
	DomProcessor &F = *(SubProcessorFactory::createDom("FFT"));
	for (int i = 0; i < w; i++) F.addWorker(SubProcessorFactory::create("FFT"));
	F.init("F", g, Properties("Size", 4096)("Step", 512)("Latency/Throughput", c));
	Monitor M;
	M.init("M", g);

	P[0] >>= F[0];
	F[0] >>= M[0];

	for (int i = 0; i < 2; i++)
	{
		g.go();
		M.waitForPlunger();
		g.stop();
	}
	std::cout << M.samplesProcessed() << " samples processed in " << M.elapsedTime() << " seconds" << std::endl;

	P[0] --;
	F[0] --;
}



