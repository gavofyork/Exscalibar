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

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace SignalTypes;

#include "multiplayer.h"

#ifdef HAVE_SNDFILE

MultiPlayer::MultiPlayer() : Processor("MultiPlayer", OutConst, Guarded)
{
}

const bool MultiPlayer::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes)
{
	outTypes = Wave(theRate);
	return true;
}

const bool MultiPlayer::openFile()
{
	if(theFile) sf_close(theFile);
	SF_INFO sfinfo;
	for(; theIndex < thePaths.count(); thePaths.remove(thePaths.at(theIndex)))
	{	theFile = sf_open(thePaths[theIndex], SFM_READ, &sfinfo);
		if(theChannels == sfinfo.channels && theRate == sfinfo.samplerate) break;
		if(theFile) sf_close(theFile);
		theFile = 0;
	}
	if(theFile) theLength = sfinfo.frames;
	thePosition = 0;
	return theFile != 0;
}

void MultiPlayer::initFromProperties(const Properties &p)
{
	theFrames = p["Frames"].toInt();
	thePaths = p.get("Filenames").toStringList();
	theIndex = 0;
	theRate = 0;
	theChannels = 0;
	// A quick openFile(), that doesn't check the channels/rate, in fact it gets them.
	SF_INFO sfinfo;
	for(; theIndex < thePaths.count(); thePaths.remove(thePaths.at(theIndex)))
	{	theFile = sf_open(thePaths[theIndex], SFM_READ, &sfinfo);
		if(theFile)
		{	theChannels = sfinfo.channels;
			theRate = sfinfo.samplerate;
			break;
		}
		theFile = 0;
	}
	setupIO(0, theChannels);
}

PropertiesInfo MultiPlayer::specifyProperties() const
{
	return PropertiesInfo("Filenames", QStringList(), "The names of the files to be played through the outputs. They must all have the same number of channels.")
	                     ("Frames", 8192, "The number of samples to output in each chunk.");
}

void MultiPlayer::specifyOutputSpace(Q3ValueVector<uint> &samples)
{
	for(int i = 0; i < theChannels; i++)
		samples[i] = theFrames;
}

void MultiPlayer::processor()
{
	float buffer[theFrames * theChannels];

	int in = 0;
	while(true)
	{	if((in = sf_readf_float(theFile, buffer, theFrames)) > 0)
		{	thePosition += in;
			for(int i = 0; i < theChannels; i++)
			{	BufferData d = output(i).makeScratchSamples(in);
				for(int j = 0; j < in; j++) d[j] = buffer[j * theChannels + i];
				output(i) << d;
			}
		}
		else if(in == 0)
		{	plunge();
			theIndex++;
			if(!openFile()) return;
		}
		else
			sf_perror(theFile);
	}
}

void MultiPlayer::processorStopped()
{
	if(theFile) sf_close(theFile);
	thePosition = 0;
}

EXPORT_CLASS(MultiPlayer, 0,1,0, Processor);

#endif

