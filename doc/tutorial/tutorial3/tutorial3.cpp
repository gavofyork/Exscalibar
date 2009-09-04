// Filename: tutorial3.cpp
// Copyright (c)2005 Gav Wood.
//
// This file is public domain - you may do with it what you want.
//
// To be compiled with:
// g++ tutorial3.cpp -I$QTDIR/include -L$QTDIR/lib -DQT_THREAD_SUPPORT \
// -L$EXSCALIBAR_LOCATION/lib -I$EXSCALIBAR_LOCATION/include -lgeddei
// -o tutorial3

#include <geddei/geddei.h>
using namespace Geddei;

int main(int, char **)
{
	ProcessorGroup g;

	ProcessorFactory::create("WaveGen")->init("wavegen1", Properties("Frequency", 1000.));
	SubProcessorFactory::createDom("FFT")->init("fft", g, Properties("Size", 512)("Step", 256));
	
	g["wavegen"][0] >>= g["fft"][0];
	
	if(!g.go(true))
		qFatal("Error starting processors!");
	
	sleep(1);
	
	g.stop();
	g.disconnectAll();
	g.deleteAll();
}

