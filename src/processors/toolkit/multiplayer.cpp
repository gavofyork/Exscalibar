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

#include "qfactoryexporter.h"

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace SignalTypes;

#include "multiplayer.h"

#ifdef HAVE_SNDFILE

MultiPlayer::MultiPlayer(): HeavyProcessor("MultiPlayer", OutConst, Guarded)
{
}

bool MultiPlayer::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes)
{
	outTypes = Wave(theRate);
	return true;
}

bool MultiPlayer::openFile()
{
	if (theFile) sf_close(theFile);
	SF_INFO sfinfo;
	for (; theIndex < (uint)thePaths.count(); thePaths.removeOne(thePaths.at(theIndex)))
	{	theFile = sf_open(thePaths[theIndex].toLatin1(), SFM_READ, &sfinfo);
		if (theChannels == sfinfo.channels && theRate == sfinfo.samplerate) break;
		if (theFile) sf_close(theFile);
		theFile = 0;
	}
	if (theFile) theLength = sfinfo.frames;
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
	for (; theIndex < (uint)thePaths.count(); thePaths.removeOne(thePaths.at(theIndex)))
	{	theFile = sf_open(thePaths[theIndex].toLatin1(), SFM_READ, &sfinfo);
		if (theFile)
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

void MultiPlayer::specifyOutputSpace(QVector<uint> &samples)
{
	for (int i = 0; i < theChannels; i++)
		samples[i] = theFrames;
}

void MultiPlayer::processor()
{
	float buffer[theFrames * theChannels];

	int in = 0;
	while (true)
	{	if ((in = sf_readf_float(theFile, buffer, theFrames)) > 0)
		{	thePosition += in;
			for (int i = 0; i < theChannels; i++)
			{	BufferData d = output(i).makeScratchSamples(in);
				for (int j = 0; j < in; j++) d[j] = buffer[j * theChannels + i];
				output(i) << d;
			}
		}
		else if (in == 0)
		{	plunge();
			theIndex++;
			if (!openFile()) return;
		}
		else
			sf_perror(theFile);
	}
}

void MultiPlayer::processorStopped()
{
	if (theFile) sf_close(theFile);
	thePosition = 0;
}

EXPORT_CLASS(MultiPlayer, 0,1,0, Processor);

#endif

