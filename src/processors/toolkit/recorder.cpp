/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#define __GEDDEI_BUILD

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "buffer.h"
#include "processor.h"
#include "processorforwarder.h"
#include "processorgroup.h"
using namespace Geddei;

#include "recorder.h"

#define MESSAGES 0

void Recorder::processor()
{
	if(MESSAGES) qDebug("> Recorder::processor(): Starting...");
	// Open file
	if(!theOutput.open(QIODevice::WriteOnly)) return;
	stream.setDevice(&theOutput);
	theCurrentSample = 0;
	theCurrentSection = 0;
	if(MESSAGES) qDebug("= Recorder::processor(): Enterring main loop...");
	while(thereIsInputForProcessing(1))
	{
		if(MESSAGES) qDebug("= Recorder::processor(): Priming row...");
		if(theCurrentSample == 0)
			for(; theCurrentSample < thePadBefore; theCurrentSample++)
			{
				if(theCurrentSample || theCurrentSection)
					stream << theRecordDelimiter;
				if(thePrintSection)
					stream << theCurrentSection << theFieldDelimiter;
				if(thePrintSample)
					stream << theCurrentSample << theFieldDelimiter;
				if(thePrintTime)
					stream << (float(theCurrentSample) / input(0).type().frequency()) << theFieldDelimiter;
				for(uint j = 0; j < numInputs(); j++)
					for(uint i = 0; i < input(i).type().scope(); i++)
						stream << "0" << theFieldDelimiter;
			}
		if(theCurrentSample || theCurrentSection)
			stream << theRecordDelimiter;
		if(thePrintSection)
			stream << theCurrentSection << theFieldDelimiter;
		if(thePrintSample)
			stream << theCurrentSample << theFieldDelimiter;
		if(thePrintTime)
			stream << (float(theCurrentSample) / input(0).type().frequency()) << theFieldDelimiter;
		for(uint i = 0; i < numInputs(); i++)
		{
			if(MESSAGES) qDebug("= Recorder::processor(): Reading from input %d...", i);
			const BufferData d = input(i).readSample();
			for(uint j = 0; j < d.elements(); j++)
			{	if(j || i) stream << theFieldDelimiter;
				stream << d[j];
			}
		}
		stream << flush;
		if(MESSAGES) qDebug("= Recorder::processor(): Done reading.");
		theCurrentSample++;
	}
	
	if(MESSAGES) qDebug("= Recorder::processor(): All done.");
	theOutput.close();
}

void Recorder::receivedPlunger()
{
	uint until = theCurrentSample + thePadAfter;
	for(; theCurrentSample < until; theCurrentSample++)
	{
		stream << theRecordDelimiter;
		if(thePrintSection)
			stream << theCurrentSection << theFieldDelimiter;
		if(thePrintSample)
			stream << theCurrentSample << theFieldDelimiter;
		if(thePrintTime)
			stream << (float(theCurrentSample) / input(0).type().frequency()) << theFieldDelimiter;
		for(uint j = 0; j < numInputs(); j++)
			for(uint i = 0; i < input(i).type().scope(); i++)
				stream << "0" << theFieldDelimiter;
	}
	theCurrentSection++;
	theCurrentSample = 0;
	// Skip to next file?
}

bool Recorder::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &)
{
//	theScope = inTypes[0].scope();
//	theFrequency = inTypes[0].frequency();
	return true;
}

PropertiesInfo Recorder::specifyProperties() const
{
	return PropertiesInfo("Inputs", 1, "The number of inputs to collect data from.")
	                     ("Output", "/tmp/out.dat", "The file into which the output data will be placed.")
	                     ("Pad Before", 0, "The number of 0-filled records to put before each section.")
	                     ("Pad After", 0, "The number of 0-filled records to put after each section.")
	                     ("Field Delimiter", " ", "The string to be inserted between each field of a record.")
	                     ("Record Delimiter", "\n", "The string to be inserted between each record.")
	                     ("Print Section", true, "If true, print the number of plungers that have come before the sample at the start of every record.")
	                     ("Print Sample", true, "If true, print the number of samples preceeding this one but after the last plunger. This will be printed at the start of the record, but after the section if there is one.")
	                     ("Print Time", true, "If true, print the number of seconds of signal data between this and the last plunger. This will be printed at the start of the record, but after the sample if there is one.");
}

void Recorder::initFromProperties(const Properties &p)
{
	// Check how many inputs we need...
	setupIO(p["Inputs"].toInt(), 0);
	
	// Set output file
	theOutput.setName(p["Output"].toString());
	
	// And the others
	theFieldDelimiter = p["Field Delimiter"].toString();
	theRecordDelimiter = p["Record Delimiter"].toString();
	thePrintSection = p["Print Section"].toBool();
	thePrintSample = p["Print Sample"].toBool();
	thePrintTime = p["Print Time"].toBool();
	thePadBefore = p["Pad Before"].toInt();
	thePadAfter = p["Pad After"].toInt();
}

EXPORT_CLASS(Recorder, 0,2,0, Processor);
