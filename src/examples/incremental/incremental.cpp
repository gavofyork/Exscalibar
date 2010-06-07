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

// I/O includes:
#include <iostream>
using namespace std;

// Geddei includes. If being built outside of Geddei, a #include
// <geddei/geddei.h> is needed. Don't forget to use Geddei's namespace.
#include "geddei.h"
using namespace Geddei;

// And the Geddei SignalTypes. We have to use the SignalTypes namespace here,
// too. Again outside Geddei, this is called <geddei/signaltypes.h>
#include "signaltypes.h"
using namespace TransmissionTypes;

// Our first custom processor. This has no inputs and one output, and it just
// produces loads of ones on that output.
class JustOnes: public HeavyProcessor
{
  // Here we setup our I/O. We want 0 inputs and 1 output:
  virtual void initFromProperties(const Properties &) { setupIO(0, 1); }
  // Here we specify our types, we just want a Wave output.
  virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes) { outTypes[0] = Value(); return true; }
  // Now we specify what this processor should output. We want it to infinitely produce 1s.
  virtual void processor() { while (true) output(0).makeScratchSamples(1, true)[0] = 1.; }
  // And a public constructor is necessary for use.
  public: JustOnes(): HeavyProcessor("JustOnes") {}
};

// Our second custom processor. This has one input and one output, and it just
// outputs the sum of the value from the input and the last output. For the
// summation of the first output we allow an initial value to be set in the
// properties under the key "First", which defaults to 0.
class Incremental: public HeavyProcessor
{
  // Member containing the initial value.
  float theFirst;
  // Here we specify the default properties; i.e. the default for First is
  // zero.
  virtual PropertiesInfo specifyProperties() { return PropertiesInfo("First", 0., "The first value."); }
  // And here we can populate our member form the given properties, then setup
  // the I/O.
  virtual void initFromProperties(const Properties &p) { theFirst = p["First"].toDouble(); setupIO(1, 1); }
  // Here we specify the output type. We trust that the input type is something
  // sensible and copy it blindly.
  virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { outTypes[0] = inTypes[0]; return true; }
  // Here we define the actual operation this processor does.
  // f(0) = theFirst
  // f(i | i > 0) = f(i - 1) + [readElement]
  // This is implemented in the for loop.
  virtual void processor() { for (float last = theFirst; true; last += input(0).readSample()[0]) output(0).makeScratchSamples(1, true)[0] = last; }
  // And a public constructor to actually use it.
  public: Incremental(): HeavyProcessor("Incremental") {}
};

// Our third and last custom processor. This will take one input and give no
// output. We simply print off whatever we receive, one sample at a time.
class Printer: public HeavyProcessor
{
  // Here we just setup the input. 1 input and no outputs.
  virtual void initFromProperties(const Properties &) { setupIO(1, 0); }
  // And here can verify our input type. As it happens, we dont care, so we
  // just return true.
  virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &) { return true; }
  // Here we define the operation of this processor. We just read a sample and
  // print off the (first) value. Easy.
  virtual void processor() { for (int i = 1; true; i++) { cout << "[" << qPrintable(name()) << "] " << i << "th element: " << input(0).readSamples(1)[0] << endl; sleep(1); } }
  // And a public constructor, as with the others.
  public: Printer(): HeavyProcessor("Printer") {}
};

int main()
{
	ProcessorGroup g;
	Processor *proc = ProcessorFactory::create("Player");
	if ( !proc )
	{
		g.deleteAll();
		return false;
	}
	proc->init("P", g, Properties("Filename", "/home/gav/Desktop/Music/ - ZEN RMX - Remix Retrospective (Disc 2)/Coldcut - 10 - Autumn Leaves [Irresistible Force mix].ogg"));
	SubProcessorFactory::createDom("Mean")->init("M", g);
	SubProcessorFactory::createDom("FFT")->init("F", g, Properties("Size", 1024)("Step", 512));
	SubProcessorFactory::createDom("MFCC")->init("B", g);
	SubProcessorFactory::createDom("Magnitude")->init("G", g);
	ProcessorFactory::create("Normalise")->init("N", g);
	g.get("P") >>= g.get("M");
	g.get("M")[0] >>= g.get("F")[0];
	g.get("F")[0] >>= g.get("B")[0];
	g.get("B")[0] >>= g.get("G")[0];
	g.get("G")[0] >>= g.get("N")[0];
	if (g.go())
	{
		qDebug("Going!");
		cin.get();
	}
	else
		qDebug("Not going.");

	return 0;
  // First we define a group to put all the processor objects in. This makes
  // management much easier. The argument of true means that it adopts the
  // processor objects, so it will look after deletion of them.
  ProcessorGroup group(true);
  // Next we need to create our processor objects. Our network will go like
  // this:
  // [JustOnes J] ---> [Incremental I1] ---> [Incremental I2] ---> [Printer P3]
  //            \.                    \.
  //             \---> [Printer P1]    \---> [Printer P2]
  // So we need 6 processor objects in total.
  // We dont need to keep track of them with variables since they are mapped
  // into the ProcessorGroup.
  (new JustOnes)->init("J", group);
  (new Incremental)->init("I1", group, Properties("First", 1.));
  (new Incremental)->init("I2", group, Properties("First", 0.));
  (new Printer)->init("P1", group);
  (new Printer)->init("P2", group);
  (new Printer)->init("P3", group);
  // Now we need to connect them all. Since J splits off into I1 and P1, we
  // have to split() or share() it. share() is far faster, so we do that...
  group.get("J")[0].share();
  // And connect it...
  group.get("J")[0] >>= group.get("I1")[0];
  group.get("J")[0] >>= group.get("P1")[0];
  // Then share() I1's output...
  group.get("I1")[0].share();
  // And connect it to I2 and P2...
  group.get("I1")[0] >>= group.get("I2")[0];
  group.get("I1")[0] >>= group.get("P2")[0];
  // Then finally connect I2 to P3...
  group.get("I2")[0] >>= group.get("P3")[0];
  // All connected - we just start them now; we use group.go(), which starts
  // all processor objects in the group at once.
  if (!group.go(true)) qFatal("Couldn't start processors.");
  // Now we wait for the user to press enter...
  cin.get();
  // Then we stop all the processors in the group...
  group.stop();
  // Then disconnect them all.
  group.disconnectAll();
  // The are automatically destroyed since group adopted them because we
  // specified true to its construction argument.
}
