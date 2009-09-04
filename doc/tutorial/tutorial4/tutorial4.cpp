// Filename: tutorial4.cpp
// Copyright (c)2005 Gav Wood.
//
// This file is public domain - you may do with it what you want.
//
// To be compiled with:
// g++ tutorial4.cpp -I$QTDIR/include -L$QTDIR/lib -DQT_THREAD_SUPPORT \
// -L$EXSCALIBAR_LOCATION/lib -I$EXSCALIBAR_LOCATION/include -lgeddei \
// -o tutorial4

#include <geddei/geddei.h>
using namespace Geddei;

#include <geddei/signaltypes.h>
using namespace SignalTypes;

class PrintModeBand: public Processor
{
	virtual void initFromProperties(const Properties &)
	{
		setupIO(1, 0);
	}
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &)
	{
		if(!inTypes[0].isA<Spectrum>()) return false;
		return true;
	}
	virtual void processor()
	{
		while(true)
		{
			const BufferData d = input(0).readSample();
			uint maxBand = 0;
			for(uint i = 1; i < d.elements(); i++)
				if(d[i] > d[maxBand]) maxBand = i;
			qDebug("Mode frequency: %fHz", input(0).type().asA<Spectrum>().bandFrequency(maxBand));
		}
	}
public:
	PrintModeBand(): Processor("PrintModeBand") {}
};

int main(int, char **)
{
	ProcessorGroup g;

	ProcessorFactory::create("WaveGen")->
		init("wavegen", g, Properties("Frequency", 1000.));
	SubProcessorFactory::createDom("FFT")->
		init("fft", g, Properties("Size", 512)("Step", 256));
	(new PrintModeBand)->init("modeband", g);
	
	g["wavegen"][0] >>= g["fft"][0];
	g["fft"][0] >>= g["modeband"][0];
	
	if(!g.go(true))
		qFatal("Error starting processors!");
	
	Geddei::sleep(1);
	
	g.stop();
	g.disconnectAll();
	g.deleteAll();
}

