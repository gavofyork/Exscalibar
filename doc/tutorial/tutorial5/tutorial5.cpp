// Filename: tutorial5.cpp
// Copyright (c)2005 Gav Wood.
//
// This file is public domain - you may do with it what you want.
//
// To be compiled with:
// g++ tutorial5.cpp -I$QTDIR/include -L$QTDIR/lib -DQT_THREAD_SUPPORT \
// -L$EXSCALIBAR_LOCATION/lib -I$EXSCALIBAR_LOCATION/include -lgeddei \
// -o tutorial5

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

class Print: public Processor
{
	virtual void initFromProperties(const Properties &)
	{
		setupIO(1, 0);
	}
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &)
	{
		if(!inTypes[0].isA<Value>()) return false;
		return true;
	}
	virtual void processor()
	{
		while(true)
		{
			const BufferData d = input(0).readSample();
			qDebug("%s: Got frequency: %fHz", name().latin1(), d[0]);
		}
	}
public:
	Print(): Processor("Print") {}
};

int main(int, char **)
{
	ProcessorGroup g;

	ProcessorFactory::create("WaveGen")->
		init("wavegen", g, Properties("Frequency", 1000.));
	SubProcessorFactory::createDom("FFT")->
		init("fft", g, Properties("Size", 512)("Step", 256));
	(new DomProcessor(new ModeFrequency))->init("modefrequency", g);
	(new Print)->init("print", g);
	
	g["wavegen"][0] >>= g["fft"][0];
	g["fft"][0] >>= g["modefrequency"][0];
	g["modefrequency"][0] >>= g["print"][0];
	
	if(!g.go(true))
		qFatal("Error starting processors!");
	
	Geddei::sleep(1);
	
	g.stop();
	g.disconnectAll();
	g.deleteAll();
}

