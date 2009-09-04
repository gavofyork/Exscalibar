// Filename: tutorial16.cpp
// Copyright (c)2005 Gav Wood.
//
// This file is public domain - you may do with it what you want.
//
// To be compiled with:
// make
// To be run with:
// nodeserver & ./tutorial16

#include <qtextra/qsubapp.h>

#include <rgeddei/rgeddei.h>
using namespace rGeddei;

LocalSession local("127.0.0.1");

class MySubApp: public QSubApp { void main(); } subApp;
void MySubApp::main()
{
	RemoteSession other("127.0.0.1");
	if(!other.isValid()) qFatal("Couldn't open remote session!");

	AbstractProcessorGroup g;

	AbstractProcessor::create(local, "Player")->
		init("player", g, Properties("Filename", "test.wav"));
	AbstractDomProcessor::create(local, "FFT")->
		init("fft", g, Properties("Size", 512)("Step", 256));
	AbstractDomProcessor::create(local, "ModeFrequency")->init("modefrequency", g);
	AbstractProcessor::create(local, "PrintAverage")->init("averagemode", g);
	AbstractDomProcessor::create(local, "MeanFrequency")->init("meanfrequency", g);
	AbstractProcessor::create(other, "PrintAverage")->init("averagemean", g);
	
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

