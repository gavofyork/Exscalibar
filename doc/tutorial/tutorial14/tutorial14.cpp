// Filename: tutorial14.cpp
// Copyright (c)2005 Gav Wood.
//
// This file is public domain - you may do with it what you want.
//
// To be compiled with:
// make

#include <geddei/geddei.h>
using namespace Geddei;

int main(int, char **)
{
	ProcessorGroup g;

	ProcessorFactory::create("Player")->
		init("player", g, Properties("Filename", "test.wav"));
	SubProcessorFactory::createDom("FFT")->
		init("fft", g, Properties("Size", 512)("Step", 256));
	SubProcessorFactory::createDom("ModeFrequency")->init("modefrequency", g);
	ProcessorFactory::create("PrintAverage")->init("averagemode", g);
	SubProcessorFactory::createDom("MeanFrequency")->init("meanfrequency", g);
	ProcessorFactory::create("PrintAverage")->init("averagemean", g);
	
	g["player"][0] >>= g["fft"][0];
	g["fft"][0].share();
	g["fft"][0] >>= g["modefrequency"][0];
	g["modefrequency"][0] >>= g["averagemode"][0];
	g["fft"][0] >>= g["meanfrequency"][0];
	g["meanfrequency"][0] >>= g["averagemean"][0];
	
	if(!g.go(true))
		qFatal("Error starting processors!");
	
	g["averagemode"].waitUntilDone();
	g["averagemean"].waitUntilDone();
	
	g.stop();
	g.disconnectAll();
	g.deleteAll();
}

