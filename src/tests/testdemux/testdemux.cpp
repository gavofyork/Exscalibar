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

#include <QThread>

#include "geddei.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace SignalTypes;

#include "monitor.h"

#define MESSAGES 0

inline float abs(float a) { return a < 0 ? -a : a; }

class PlungeGenerator : public HeavyProcessor
{
	virtual void processor()
	{
		for (uint i = 0; i < 100; i++)
		{	BufferData s = output(0).makeScratchSample();
			s[0] = i;
			output(0) << s;
		}
		plunge();
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes) { outTypes[0] = Wave(22050); return true; }
	virtual void initFromProperties(const Properties &) { setupIO(0, 1); }

public:
	PlungeGenerator(): HeavyProcessor("PlungeGenerator") {}
};

class PlungeDetector : public HeavyProcessor
{
	void processor()
	{
		while (thereIsInputForProcessing())
		{
			BufferData s = input(0).readSample();
			cout << s[0] << " " << flush;
		}
	}

	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &) { return true; }
	virtual void initFromProperties(const Properties &) { setupIO(1, 0); }
	virtual void receivedPlunger()
	{
		cout << endl;
	}
public:
	PlungeDetector(): HeavyProcessor("PlungeDetector", NotMulti, true) {}
};

class Add: public SubProcessor
{
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const { out[0][0] = in[0][0] + in[0][2] + in[0][4]; out[0][1] = in[0][1] + in[0][3]; }
	virtual void initFromProperties(const Properties &) { setupIO(1, 1, 5, 3, 2); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes = inTypes[0]; return true; }
public:
	Add() : SubProcessor("Add") {}
};

class Sub: public SubProcessor
{
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const { out[0][0] = (in[0][0] - in[0][1]) + (in[0][2] - in[0][3]); }
	virtual void initFromProperties(const Properties &) { setupIO(1, 1, 4, 2, 1); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes = inTypes[0]; return true; }
public:
	Sub() : SubProcessor("Sub") {}
};

class Diff: public SubProcessor
{
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const { out[0][0] = ::abs(in[0][0] - in[1][0]); }
	virtual void initFromProperties(const Properties &) { setupIO(2, 1, 1, 1, 1); }
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes = inTypes[0]; return true; }
public:
	Diff() : SubProcessor("Diff", InConst) {}
};

int main()
{
	ProcessorGroup g;
	PlungeGenerator O;
	DomProcessor X("Log");
	DomProcessor Y("Exp");
	DomProcessor Z("DownSample");
	DomProcessor C("Log&Exp&DownSample");
	DomProcessor D(new Diff);
	PlungeDetector W;
	O.init("O", g);
	X.init("X", g);
	Y.init("Y", g);
	Z.init("Z", g);
	C.init("C", g);
	D.init("D", g);
	W.init("W", g);
	O[0].share();
	O[0] >>= X[0];
	X[0] >>= Y[0];
	Y[0] >>= Z[0];
	Z[0] >>= D[0];
	O[0] >>= C[0];
	C[0] >>= D[1];
	D[0] >>= W[0];

	cout << "Starting..." << std::endl;
	g.go();
	W.waitUntilDone();
	cout << "Stopping..." << std::endl;
	g.stop();
	g.disconnectAll();
}
