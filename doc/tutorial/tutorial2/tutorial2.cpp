// Filename: tutorial2.cpp
// Copyright (c)2005 Gav Wood.
//
// This file is public domain - you may do with it what you want.
//
// To be compiled with:
// g++ tutorial2.cpp -I$QTDIR/include -L$QTDIR/lib -DQT_THREAD_SUPPORT \
// -L$EXSCALIBAR_LOCATION/lib -I$EXSCALIBAR_LOCATION/include -lgeddei
// -o tutorial2

#include <geddei/geddei.h>
using namespace Geddei;

int main(int, char **)
{
	Processor &wavegen = *ProcessorFactory::create("WaveGen");
	DomProcessor &fft = *SubProcessorFactory::createDom("FFT");
	
	wavegen.init("wavegen1", Properties("Frequency", 1000.));
	fft.init("fft1", Properties("Size", 512)("Step", 256));
	
	wavegen[0] >>= fft[0];
	
	wavegen.go();
	fft.go();
	
	if(wavegen.waitUntilGoing() != Processor::NoError)
		qFatal("Error starting wavegen!");
	if(fft.waitUntilGoing() != Processor::NoError)
		qFatal("Error starting fft!");
	
	sleep(1);
	
	fft.stop();
	wavegen.stop();
	
	wavegen[0]--;
	
	delete &fft;
	delete &wavegen;
}

