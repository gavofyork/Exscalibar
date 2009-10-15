/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
 *
 * This file is part of Exscalibar.
 *
 * Exscalibar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Exscalibar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Exscalibar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>

#include "geddei.h"
using namespace Geddei;

#include "recorder.h"

#include "wave.h"
using namespace SignalTypes;

class Trivial: public HeavyProcessor
{
	virtual void processor()
	{
		for (uint i = 0; i < 10; i++)
		{	for (uint j = 0; j < 10; j++)
			{	BufferData s = output(0).makeScratchSample();
				s[0] = j + i*10;
				output(0) << s;
			}
			plunge();
		}
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes) { outTypes[0] = Wave(22050); return true; }
	virtual void initFromProperties(const Properties &) { setupIO(0, 1); }
	public: Trivial(): HeavyProcessor("Trivial") {}
};

class Trivial2: public HeavyProcessor
{
	virtual void processor()
	{
		for (int i = 0;; i++)
			output(0).makeScratchSample(true)[0] = i;
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes) { outTypes[0] = Wave(22050); return true; }
	virtual void initFromProperties(const Properties &) { setupIO(0, 1); }
	public: Trivial2(): HeavyProcessor("Trivial2") {}
};

class Multiply: public SubProcessor
{
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const { out[0][0] = in[0][0] * in[1][0]; }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes[0] = inTypes[0]; return true; }
	virtual void initFromProperties(const Properties &) { setupIO(2, 1); }
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
	if (!objects.go()) qFatal("Couldn't start!");

	std::cout << "Waiting till done..." << std::endl;
	I.waitUntilDone();

	std::cout << "Stopping objects..." << std::endl;
	objects.stop();

	std::cout << "Disconnecting..." << std::endl;
	O[0] --;
}
