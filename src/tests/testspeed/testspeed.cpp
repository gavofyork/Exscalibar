/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <cmath>
using namespace std;

#include <qdatetime.h>

#include "geddei.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace SignalTypes;

class MySource: public Processor
{
	virtual void processor()
	{
		for (int i = 0; i < 200000; i++)
		{
			{	BufferData s = output(0).makeScratchSamples(10);
				for (int j = 0; j < 10; j++)
					s[j] = j;
				output(0) << s;
			}
		}
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes) { outTypes[0] = Value(); return true; }
	virtual void initFromProperties(const Properties &) { setupIO(0, 1); }
	virtual void specifyOutputSpace(QVector<uint> &o) { o[0] = 10; }
public:
	MySource() : Processor("MySource") {}
};

class MySink: public Processor
{
	virtual void processor()
	{
		float f;
		while (thereIsInputForProcessing(10))
		{
			{	const BufferData s = input(0).readSamples(10);
				for (int j = 0; j < 10; j++)
					f = s[j];
			}
//	sched_yield();
		}
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &) { return true; }
	virtual void initFromProperties(const Properties &) { setupIO(1, 0); }
public:
	MySink() : Processor("MySink", NotMulti, Guarded) {}
};

class LatencySource: public Processor
{
	virtual void processor()
	{
		for (int i = 0; i < 200000; i++)
		{
			QTime *t = new QTime;
			t->start();
			output(0).makeScratchSamples(1, true)[0] = *((float *)&t);
		}
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &out) { out[0] = Value(); return true; }
	virtual void initFromProperties(const Properties &) { setupIO(0, 1); }
public:
	LatencySource() : Processor("LatencySource") {}
};

class LatencySink: public Processor
{
	int theElapsedSum, theSamples;
	virtual void processor()
	{
		theElapsedSum = theSamples = 0;
		while (thereIsInputForProcessing(1))
		{
			float f = input(0).readSample()[0];
			QTime *t = *((QTime **)&f);
			theElapsedSum += t->elapsed();
			theSamples++;
			delete t;
		}
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &) { return true; }
	virtual void initFromProperties(const Properties &) { setupIO(1, 0); }
public:
	float latency() const { return float(theElapsedSum) / float(theSamples); }
	LatencySink() : Processor("LatencySink", NotMulti, Guarded) {}
};

class PassThrough: public SubProcessor
{
	virtual void initFromProperties(const Properties &)
	{
		setupIO(1, 1, 1, 1);
	}
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &in, SignalTypeRefs &out)
	{
		out = in;
		return true;
	}
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const
	{
		out[0].copyFrom(in[0]);
	}
public:
	PassThrough(): SubProcessor("PassThrough") {}
};

int main(int argc, char **argv)
{
	ProcessorGroup g;

	int mode = argc > 1 ? QString(argv[1]).toInt() : 0;

	if (mode == 0)
	{
		(new MySource)->init("o", g);
		(new MySink)->init("i", g);
		g["o"][0] >>= g["i"][0];
		g.go(true);
		g["i"].waitUntilDone();
		g.stop();
	}
	else if (mode == 1)
	{
		(new MySource)->init("o", g);
		(new MySource)->init("p", g);
		g.go(true);
		g["o"].waitUntilDone();
		g["p"].waitUntilDone();
		g.stop();
	}
	else if (mode == 2)
	{
		float weight = argc > 2 ? QString(argv[2]).toDouble() : 0.5;
		int links = argc > 3 ? QString(argv[3]).toInt() : 1;
		(new LatencySource)->init("o", g);
		for (int i = 0; i < links; i++)
			(new DomProcessor(new PassThrough))->init("p" + QString::number(i), g,
				Properties("Latency/Throughput", weight));
		LatencySink *sink = new LatencySink;
		sink->init("i", g);
		g["o"][0] >>= g[links > 0 ? "p0" : "i"][0];
		for (int i = 0; i < links; i++)
			g["p" + QString::number(i)][0] >>=
				g[i == links - 1 ? "i" : ("p" + QString::number(i+1))][0];

		uint count = 3;
		float times[count], ltimes[count];

		for (uint t = 0; t < count; t++)
		{
			QTime clock;
			g.go(true);
			clock.start();
			g["i"].waitUntilDone();
			times[t] = clock.elapsed() / 1000.;
			ltimes[t] = sink->latency() / 1000.;
			g.stop();
		}

		float mean = 0., sd = 0., lmean = 0., lsd = 0.;
		for (uint t = 0; t < count; t++) mean += float(times[t]) / float(count);
		for (uint t = 0; t < count; t++) sd += ((float(times[t]) - mean) * (float(times[t]) - mean));
		for (uint t = 0; t < count; t++) lmean += float(ltimes[t]) / float(count);
		for (uint t = 0; t < count; t++) lsd += ((float(ltimes[t]) - lmean) * (float(ltimes[t]) - lmean));
		sd = sqrt(sd / float(count));
		lsd = sqrt(lsd / float(count));

		cout << mean << " " << sd << " " << lmean << " " << lsd << endl;
	}
	else if (mode == 3)
	{
		float weight = argc > 2 ? QString(argv[2]).toDouble() : 0.5;
		int links = argc > 3 ? QString(argv[3]).toInt() : 1;
		(new LatencySource)->init("o", g);
		SubProcessor *sub = 0;
		for (int i = 0; i < links; i++)
			if (!sub) sub = new PassThrough; else sub = new Combination(sub, new PassThrough);
		LatencySink *sink = new LatencySink;
		if (sub)
			(new DomProcessor(sub))->init("p", g, Properties("Latency/Throughput", weight));
		sink->init("i", g);
		if (sub)
		{
			g["o"][0] >>= g["p"][0];
			g["p"][0] >>= g["i"][0];
		}
		else
			g["o"][0] >>= g["i"][0];
		uint count = 3;
		float times[count], ltimes[count];

		for (uint t = 0; t < count; t++)
		{
			QTime clock;
			g.go(true);
			clock.start();
			g["i"].waitUntilDone();
			times[t] = clock.elapsed() / 1000.;
			ltimes[t] = sink->latency() / 1000.;
			g.stop();
		}

		float mean = 0., sd = 0., lmean = 0., lsd = 0.;
		for (uint t = 0; t < count; t++) mean += float(times[t]) / float(count);
		for (uint t = 0; t < count; t++) sd += ((float(times[t]) - mean) * (float(times[t]) - mean));
		for (uint t = 0; t < count; t++) lmean += float(ltimes[t]) / float(count);
		for (uint t = 0; t < count; t++) lsd += ((float(ltimes[t]) - lmean) * (float(ltimes[t]) - lmean));
		sd = sqrt(sd / float(count));
		lsd = sqrt(lsd / float(count));

		cout << mean << " " << sd << " " << lmean << " " << lsd << endl;
	}
	g.disconnectAll();
	g.deleteAll();
}
