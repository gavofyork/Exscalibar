// Filename: tutorial3.cpp
// Copyright (c)2005 Gav Wood.
//
// This file is public domain - you may do with it what you want.
//
// To be compiled with:
// g++ tutorial3.cpp -I$QTDIR/include -L$QTDIR/lib -DQT_THREAD_SUPPORT \
// -L$EXSCALIBAR_LOCATION/lib -I$EXSCALIBAR_LOCATION/include -lgeddei

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

class PrintAverage: public Processor
{
	virtual const PropertiesInfo specifyProperties()
	{
		return PropertiesInfo("AveragesUntilDone", 10,
			"The number of averages this Processor should carry out.");
	}
	int theAveragesUntilDone;
	virtual void initFromProperties(const Properties &p)
	{
		theAveragesUntilDone = p["AveragesUntilDone"].toInt();
		setupIO(1, 0);
	}
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &)
	{
		if(!inTypes[0].isA<Value>()) return false;
		return true;
	}
	float sum;
	uint count, plungers;
	virtual void processor()
	{
		sum = 0.;
		count = 0;
		plungers = 0;
		while(plungers < theAveragesUntilDone)
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
		plungers++;
	}
public:
	PrintAverage(): Processor("PrintAverage", NotMulti, Guarded) {}
};

int main(int, char **)
{
	ProcessorGroup g;

	ProcessorFactory::create("WaveGen")->
		init("wavegen", g, Properties("Frequency", 1000.)("PlungersPerSecond", 1.));
	SubProcessorFactory::createDom("FFT")->
		init("fft", g, Properties("Size", 512)("Step", 256));
	(new DomProcessor(new ModeFrequency))->init("modefrequency", g);
	(new PrintAverage)->
		init("printaverage", g, Properties("AveragesUntilDone", 5));
	
	g["wavegen"][0] >>= g["fft"][0];
	g["fft"][0] >>= g["modefrequency"][0];
	g["modefrequency"][0] >>= g["printaverage"][0];
	
	if(!g.go(true))
		qFatal("Error starting processors!");
	
	g["printaverage"].waitUntilDone();
	
	g.stop();
	g.disconnectAll();
	g.deleteAll();
}

