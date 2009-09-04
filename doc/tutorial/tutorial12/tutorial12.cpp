// Filename: tutorial12.cpp
// Copyright (c)2005 Gav Wood.
//
// This file is public domain - you may do with it what you want.
//
// To be compiled with:
// make

#include <geddei/geddei.h>
using namespace Geddei;

#include <geddei/signaltypes.h>
using namespace SignalTypes;

class ModeFrequency: public SubProcessor
{
	virtual void initFromProperties(const Properties &)
	{
		setupIO(1, 1, 1, 1, 1);
	}
	Spectrum theSpectrum;
	virtual const bool verifyAndSpecifyTypes(
		const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
	{
		if(!inTypes[0].isA<Spectrum>()) return false;
		theSpectrum = inTypes[0].asA<Spectrum>();
		outTypes[0] = Value(theSpectrum.frequency());
		return true;
	}
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const
	{
		uint maxBand = 0;
		for(uint i = 1; i < in[0].elements(); i++)
			if(in[0][i] > in[0][maxBand]) maxBand = i;
		out[0][0] = theSpectrum.bandFrequency(maxBand);
	}
public:
	ModeFrequency(): SubProcessor("ModeFrequency") {}
};

class MeanFrequency: public SubProcessor
{
	virtual void initFromProperties(const Properties &)
	{
		setupIO(1, 1, 1, 1, 1);
	}
	Spectrum theSpectrum;
	virtual const bool verifyAndSpecifyTypes(
		const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
	{
		if(!inTypes[0].isA<Spectrum>()) return false;
		theSpectrum = inTypes[0].asA<Spectrum>();
		outTypes[0] = Value(theSpectrum.frequency());
		return true;
	}
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const
	{
		float sum = 0., magnitude = 0.;
		for(uint i = 0; i < in[0].elements(); i++)
		{	float e = in[0][i] < 0. ? -in[0][i] : in[0][i];
			sum += theSpectrum.bandFrequency(i) * e;
			magnitude += e;
		}
		out[0][0] = (magnitude > 0.) ? sum / magnitude : 0.;
	}
public:
	MeanFrequency(): SubProcessor("MeanFrequency") {}
};

class PrintAverage: public Processor
{
	virtual void initFromProperties(const Properties &)
	{
		setupIO(1, 0);
	}
	virtual const bool verifyAndSpecifyTypes(
		const SignalTypeRefs &inTypes, SignalTypeRefs &)
	{
		if(!inTypes[0].isA<Value>()) return false;
		return true;
	}
	float sum;
	uint count;
	virtual void processor()
	{
		sum = 0.;
		count = 0;
		while(thereIsInputForProcessing(1))
		{
			sum += input(0).readSample()[0];
			count++;
		}
	}
	virtual void receivedPlunger()
	{
		qDebug("%s: Average frequency %fHz (from %d samples)", name().latin1(), sum / count, count);
		count = 0;
		sum = 0.;
	}
public:
	PrintAverage(): Processor("PrintAverage", NotMulti, Guarded) {}
};

int main(int, char **)
{
	ProcessorGroup g;

	ProcessorFactory::create("Player")->
		init("player", g, Properties("Filename", "test.wav"));
	SubProcessorFactory::createDom("FFT")->
		init("fft", g, Properties("Size", 512)("Step", 256));
	(new DomProcessor(new ModeFrequency))->init("modefrequency", g);
	(new PrintAverage)->init("averagemode", g);
	(new DomProcessor(new MeanFrequency))->init("meanfrequency", g);
	(new PrintAverage)->init("averagemean", g);
	
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

