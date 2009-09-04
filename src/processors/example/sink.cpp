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

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "buffer.h"
#include "processor.h"
using namespace Geddei;

class SignalSink: public Processor
{
protected:
	virtual void processor();
	virtual void initFromProperties(const Properties &props);
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &/*inTypes*/, SignalTypeRefs &/*outTypes*/) { return true; }
public:
	SignalSink() : Processor("SignalSink") {}
};

void SignalSink::processor()
{
	while(thereIsInputForProcessing())
		input(0).readSamples();
}

void SignalSink::initFromProperties(const Properties &)
{
	setupIO(1, 0);
}

EXPORT_CLASS(SignalSink, 0,1,0, Processor);
