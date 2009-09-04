// Filename: tutorial1.cpp
// Copyright (c)2005 Gav Wood.
//
// This file is public domain - you may do with it what you want.
//
// To be compiled with:
// g++ tutorial1.cpp -I$QTDIR/include -L$QTDIR/lib -DQT_THREAD_SUPPORT \
// -L$EXSCALIBAR_LOCATION/lib -I$EXSCALIBAR_LOCATION/include -lgeddei \
// -o tutorial1

#include <geddei/geddei.h>
using namespace Geddei;

int main(int, char **)
{
	Processor &wavegen = *ProcessorFactory::create("WaveGen");
	
	wavegen.init("wavegen1", Properties("Frequency", 1000.));
	
	wavegen.go();
	
	if(wavegen.waitUntilGoing() != Processor::NoError)
		qFatal("Error starting wavegen!");
	
	sleep(1);
	
	wavegen.stop();
	
	delete &wavegen;
}

