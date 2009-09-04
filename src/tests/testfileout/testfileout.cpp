#define __GEDDEI_BUILD

#include <iostream>

#include "geddei.h"
using namespace Geddei;

#include "recorder.h"

#include "wave.h"
using namespace SignalTypes;

class Trivial: public Processor
{
	virtual void processor()
	{
		for(uint i = 0; i < 10; i++)
		{	for(uint j = 0; j < 10; j++)
			{	BufferData s = output(0).makeScratchSample();
				s[0] = j + i*10;
				output(0) << s;
			}
			plunge();
		}
	}
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes) { outTypes[0] = Wave(22050); return true; }
	virtual void initFromProperties(const Properties &) { setupIO(0, 1); }
	public: Trivial() : Processor("Trivial") {}
};

class Trivial2: public Processor
{
	virtual void processor()
	{
		for(int i = 0;; i++)
			output(0).makeScratchSample(true)[0] = i;
	}
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes) { outTypes[0] = Wave(22050); return true; }
	virtual void initFromProperties(const Properties &) { setupIO(0, 1); }
	public: Trivial2() : Processor("Trivial2") {}
};

class Multiply: public SubProcessor
{
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const { out[0][0] = in[0][0] * in[1][0]; }
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes[0] = inTypes[0]; return true; }
	virtual void initFromProperties(const Properties &) { setupIO(2, 1, 1, 1, 1); }
	public: Multiply(): SubProcessor("Multiply") {}
};

int main()
{
	Processor	&O = *(new Trivial),
				&P = *(new Trivial2),
				&M = *(new DomProcessor(new Multiply)),
				&E = *(SubProcessorFactory::createDom("Mean")),
				&I = *(new Recorder),
				&D = *(SubProcessorFactory::createDom("DownSample")),
				&J = *(new Recorder);

	ProcessorGroup objects;
	O.init("O", objects);
	P.init("P", objects);
	M.init("M", objects);
	E.init("E", objects, Properties("Multiplicity", 2));
	I.init("I", objects, Properties("Inputs", 3)("Field Delimiter", " ")("Record Delimiter", "\n")("Print Time", false)("Print Sample", true)("Print Section", true)("Output", "/tmp/out.dat"));
	D.init("D", objects, Properties("Divisor", 3)("Spread", 3));
	J.init("J", objects, Properties("Inputs", 1)("Field Delimiter", " ")("Record Delimiter", "\n")("Print Time", false)("Print Sample", true)("Print Section", true)("Output", "/tmp/out2.dat"));

	std::cout << "Connecting..." << std::endl;
	O[0].share();
	P[0].share();
	P[0] >>= D[0];
	D[0] >>= J[0];
	O[0] >>= I[0];
	O[0] >>= M[0];
	P[0] >>= M[1];
	M[0].share();
	M[0] >>= I[1];
	O[0] >>= E[0];
	M[0] >>= E[1];
	E[0] >>= I[2];

	std::cout << "Starting objects..." << std::endl;
	if(!objects.go()) qFatal("Couldn't start!");

	std::cout << "Waiting till done..." << std::endl;
	I.waitUntilDone();

	std::cout << "Stopping objects..." << std::endl;
	objects.stop();

	std::cout << "Disconnecting..." << std::endl;
	O[0] --;
}
