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
using namespace std;

#include "geddei.h"
using namespace Geddei;

#include "coretypes.h"
using namespace Geddei;

#include "monitor.h"

class OneTwo: public CoProcessor
{
public:
	OneTwo(): CoProcessor("OneTwo"){}

	virtual void initFromProperties(const Properties &)
	{
		setupIO(0, 1);
	}
	virtual bool verifyAndSpecifyTypes(const Types &, Types &outTypes)
	{
		n = 0;
		outTypes[0] = Value();
		return true;
	}
	virtual int process()
	{
		BufferData d = output(0).makeScratchSample(true);
		d[0] = n;
		n = ++n % 65536;
		return CanWork;
	}
	virtual int canProcess()
	{
		return CanWork;
	}

private:
	int n;
};

class TSA: public SubProcessor
{
public:
	TSA() : SubProcessor("TSA") {}

	virtual void processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks)
	{
		for (uint i = 0; i < chunks; i++)
			out[0].sample(i).copyFrom(in[0].mid(i * 1024, 2048));
	}

	virtual bool verifyAndSpecifyTypes(Types const&, Types& _outTypes)
	{
		_outTypes[0] = FreqSteppedSpectrum(2048, 1);
		return true;
	}
	virtual void initFromProperties(Properties const&)
	{
		setupIO(1, 1);
		setupSamplesIO(2048, 1024, 1);
	}
};

class Transparent: public SubProcessor
{
public:
	Transparent() : SubProcessor("Transparent") {}

	virtual void processOwnChunks(const BufferDatas &in, BufferDatas &out, uint _ch)
	{
		for (uint c = 0; c < _ch; c++)
			for (uint i = 0; i < 64; i++)
				out[0](c, i) = in[0](c, i);
	}

	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
	{
		outTypes = inTypes;
		return true;
	}
	virtual void initFromProperties(const Properties &)
	{
		setupIO(1, 1);
		setupSamplesIO(64, 1, 1);
	}
};

void othertest()
{
	ProcessorGroup objects;
	(new OneTwo)->init("O", objects);
	(new DomProcessor(new Combination(new TSA, new Transparent)))->init("D", objects);
	Monitor *I = (new Monitor);
	I->init("I", objects);
	objects.get("O")[0] >>= objects.get("D")[0];
	objects.get("D")[0] >>= objects.get("I")[0];

	cout << "Confirming types..." << endl;
	objects.confirmTypes();

	cout << "Starting objects..." << endl;
	objects.go();

	cout << "Waiting for a little bit..." << endl;
	I->waitUntilDone();

	cout << "Stopping objects..." << endl;
	objects.stop();

	cout << "Disconnecting..." << endl;
	objects.disconnectAll();

	cout << "Monitor: " << I->elementsProcessed() << endl << endl << endl << endl;
	delete I;

	objects.deleteAll();
}

int main(int argc, char **argv)
{
	othertest();
/*	QStringList procs = ProcessorFactory::available();
	cout << "Processors:" << endl;
	for (QStringList::iterator i = procs.begin(); i != procs.end(); i++)
		cout << "   " << *i << " (" << ProcessorFactory::version(*i) << ")" << endl;
	procs = SubProcessorFactory::available();
	cout << "SubProcessors:" << endl;
	for (QStringList::iterator i = procs.begin(); i != procs.end(); i++)
		cout << "   " << *i << " (" << SubProcessorFactory::version(*i) << ")" << endl;
*/
	ProcessorGroup objects;
	ProcessorFactory::create("Player")->init("O", objects, Properties("Filename", argc < 2 ? "/tmp/test2.wav" : argv[1]));
	SubProcessorFactory::createDom("Mean")->init("M", objects, Properties("Multiplicity", 2));
	SubProcessorFactory::createDom("FFT")->init("F", objects, Properties("Debug", true));
	SubProcessorFactory::createDom("DownSample")->init("D", objects);

//	for (int i = 0; i < 10; i++)
	{
		Monitor *I = (new Monitor);
		I->init("I", objects);

		cout << "Connecting..." << endl;
		objects.get("O")[0] >>= objects.get("M")[0];
		objects.get("O")[1] >>= objects.get("M")[1];
		objects.get("M")[0] >>= objects.get("D")[0];
		objects.get("D")[0] >>= objects.get("F")[0];
		objects.get("F")[0] >>= objects.get("I")[0];

		cout << "Starting objects..." << endl;
		objects.go();

		cout << "Waiting for a little bit..." << endl;
		I->waitUntilDone();

		cout << "Stopping objects..." << endl;
		objects.stop();

		cout << "Disconnecting..." << endl;
		objects.disconnectAll();

		cout << "Monitor: " << I->elementsProcessed() << endl << endl << endl << endl;
		delete I;
	}

	objects.deleteAll();
}
