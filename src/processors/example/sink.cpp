/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

#include "qfactoryexporter.h"

#include "transmissiontype.h"
#include "bufferdata.h"
#include "buffer.h"
#include "processor.h"
using namespace Geddei;

class SignalSink: public HeavyProcessor
{
protected:
	virtual void processor();
	virtual void initFromProperties(const Properties &props);
	virtual bool verifyAndSpecifyTypes(const Types &/*inTypes*/, Types &/*outTypes*/) { return true; }
public:
	SignalSink(): HeavyProcessor("SignalSink") {}
};

void SignalSink::processor()
{
	while (thereIsInputForProcessing())
		input(0).readSamples();
}

void SignalSink::initFromProperties(const Properties &)
{
	setupIO(1, 0);
}

EXPORT_CLASS(SignalSink, 0,1,0, Processor);
