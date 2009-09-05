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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MAD
#undef SHRT_MAX
#include "madhelp.cpp"
#endif

#include "qfactoryexporter.h"

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace SignalTypes;

#include "player.h"

#define MESSAGES 0

Player::Player() : Processor("Player", OutConst, Guarded), thePath(""), theChannels(0), theRate(0), theLength(0), thePosition(0)
{
}

bool Player::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes)
{
	outTypes = Wave(theRate);
	return true;
}

void Player::specifyOutputSpace(Q3ValueVector<uint> &samples)
{
	for (uint i = 0; i < theChannels; i++)
		samples[i] = theReadFrames;
}

void Player::initFromProperties(const Properties &p)
{
	theReadFrames = p["Frames"].toInt();
	thePath = p["Filename"].toString();
	theMode = NoMode;
	theChannels = 0;

	qDebug("Opening file %s...", thePath.latin1());
#ifdef HAVE_VORBISFILE
	if (thePath.lower().contains(".ogg"))
	{
		QFile qfile(thePath);
		if (!qfile.open(QIODevice::ReadOnly))
			qWarning("*** WARNING: Cannot open file %s", thePath.latin1());
		else if (ov_open(fdopen(qfile.handle(), "r"), &theVorbisFile, NULL, 0) < 0)
			qWarning("*** WARNING: File %s does not appear to be an Ogg bitstream.", thePath.latin1());
		else
		{
			vorbis_info *vi = ov_info(&theVorbisFile, -1);
			theChannels = vi->channels;
			theRate = vi->rate;
			theLength = (long)ov_pcm_total(&theVorbisFile, -1);
			theMode = ModeVF;
			ov_clear(&theVorbisFile);
		}
	}
#endif
#ifdef HAVE_SNDFILE
	if (thePath.lower().contains(".wav"))
	{
		SF_INFO sfinfo;
		if (!(theSndFile = sf_open(thePath, SFM_READ, &sfinfo)))
			qWarning("*** WARNING: File %s cannot be read.", thePath.latin1());
		else
		{
			theLength = sfinfo.frames;
			theChannels = sfinfo.channels;
			theRate = sfinfo.samplerate;
			theMode = ModeSF;
			sf_close(theSndFile);
		}
	}
#endif
#ifdef HAVE_MAD
	if (thePath.lower().contains(".mp3"))
	{
	uint INPUT_BUFFER_SIZE = (theReadFrames / 4) * 4 * 5;
	unsigned char InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD];
	QFile qfile(thePath);
	if (!qfile.open(QIODevice::ReadOnly))
		qWarning("*** WARNING: Cannot open file %s", thePath.latin1());
	else if (!(theMadFile = fdopen(qfile.handle(), "r")))
		qWarning("*** WARNING: File %s cannot be opened.", thePath.latin1());
	else
	{
	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	if (!(BstdFile=NewBstdFile(theMadFile))) { qDebug("*** ERROR: Couldn't create bstdfile on file %s!", thePath.latin1()); return; }
	theRate = 0;
	while (!theRate || !theChannels)
	{
		if (Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
		{
			size_t ReadSize, Remaining;
			unsigned char *ReadStart;
			if (Stream.next_frame)
			{
				Remaining = Stream.bufend-Stream.next_frame;
				memmove(InputBuffer, Stream.next_frame, Remaining);
				ReadStart = InputBuffer + Remaining;
				ReadSize = INPUT_BUFFER_SIZE - Remaining;
			}
			else
				ReadSize=INPUT_BUFFER_SIZE, ReadStart=InputBuffer, Remaining=0;
			if ((ReadSize = BstdRead(ReadStart, 1, ReadSize, BstdFile)) <= 0) break;
			if (BstdFileEofP(BstdFile))
			{
				memset(ReadStart + ReadSize, 0, MAD_BUFFER_GUARD);
				ReadSize += MAD_BUFFER_GUARD;
			}
			mad_stream_buffer(&Stream, InputBuffer, ReadSize + Remaining);
			Stream.error = (mad_error)0;
		}

		if (mad_frame_decode(&Frame, &Stream))
		{
			if (MAD_RECOVERABLE(Stream.error) || Stream.error==MAD_ERROR_BUFLEN) continue; else break;
		}
		theChannels = MAD_NCHANNELS(&Frame.header);
		theRate = Frame.header.samplerate;
		theLength = 1;
	}
	BstdFileDestroy(BstdFile);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);
	fclose(theMadFile);
	theMode = ModeMAD;
	}
	}
#endif
	qDebug("Mode: %s, Length: %d, Channels: %d, Sampling: %d Hz", theMode==ModeSF ? "Soundfile" : theMode==ModeVF ? "Ogg/Vorbis" : theMode==ModeMAD ? "MP3" : "None", theLength, theChannels, theRate);
	thePosition = 0;
	if (theChannels) setupIO(0, theChannels);
}

PropertiesInfo Player::specifyProperties() const
{
	return PropertiesInfo("Filename", "/tmp/cdda.wav", "The name of the file to be played through the output(s)")
						 ("Frames", 8192, "The number of samples to output in each chunk.");
}

void Player::processor()
{
#ifdef HAVE_SNDFILE
	if (theMode == ModeSF)
	{
		SF_INFO sfinfo;
		theSndFile = sf_open(thePath, SFM_READ, &sfinfo);
		if (!theSndFile) return;
		float buffer[theReadFrames * theChannels];
		int in = 0;
		while (true)
		{
			if ((in = sf_readf_float(theSndFile, buffer, theReadFrames)) > 0)
			{	thePosition += in;
				for (uint i = 0; i < theChannels; i++)
				{	BufferData d = output(i).makeScratchSamples(in);
					if (!d.isNull())
						for (int j = 0; j < in; j++)
							d[j] = buffer[j * theChannels + i];
					output(i).push(d);
				}
			}
			else if (in == 0)
				break;
			else
				sf_perror(theSndFile);
		}
	}
#endif
#ifdef HAVE_VORBISFILE
	if (theMode == ModeVF)
	{
		QFile qfile(thePath);
		if (!qfile.open(QIODevice::ReadOnly)) return;
		if (ov_open(fdopen(qfile.handle(), "r"), &theVorbisFile, NULL, 0) < 0) return;
		float **buffer;
		int in = 0, current_section = 0;
		while (true)
		{
			in = ov_read_float(&theVorbisFile, &buffer, theReadFrames, &current_section);
			if (in == 0)
				break;
			else if (in < 0)
				qWarning("*** WARNING: Error in bitstream.");
			else
			{
				thePosition += in;
				for (uint i = 0; i < (uint)theChannels; i++)
				{	BufferData d = output(i).makeScratchSamples(in/* / theChannels*/);
					if (!d.isNull())
						for (uint j = 0; j < (uint)in/* / theChannels*/; j++)
							d[j] = buffer[i][j];
					output(i).push(d);
				}
			}
		}
	}
#endif
#ifdef HAVE_MAD
	if (theMode == ModeMAD)
	{
	QFile qfile(thePath);
	if (!qfile.open(QIODevice::ReadOnly)) return;
	theMadFile = fdopen(qfile.handle(), "r");
	uint INPUT_BUFFER_SIZE = (theReadFrames / 4) * 4 * 5;
	unsigned char InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD], *GuardPtr=NULL;

	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);
	mad_timer_reset(&Timer);

	if (!(BstdFile=NewBstdFile(theMadFile))) { qDebug("*** ERROR: Couldn't create bstdfile on file %s!", thePath.latin1()); return; }
	while (1)
	{
		if (Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
		{
			size_t			ReadSize, Remaining;
			unsigned char	*ReadStart;

			if (Stream.next_frame!=NULL)
			{
				Remaining=Stream.bufend-Stream.next_frame;
				memmove(InputBuffer,Stream.next_frame,Remaining);
				ReadStart=InputBuffer+Remaining;
				ReadSize=INPUT_BUFFER_SIZE-Remaining;
			}
			else
				ReadSize=INPUT_BUFFER_SIZE, ReadStart=InputBuffer, Remaining=0;

			ReadSize=BstdRead(ReadStart,1,ReadSize,BstdFile);
			if (ReadSize<=0)
			{
				if (ferror(theMadFile)) qDebug("*** WARNING: Read error on bit-stream from file %s (%s)", thePath.latin1(), strerror(errno));
				break;
			}

			if (BstdFileEofP(BstdFile))
			{
				GuardPtr=ReadStart+ReadSize;
				memset(GuardPtr,0,MAD_BUFFER_GUARD);
				ReadSize+=MAD_BUFFER_GUARD;
			}

			mad_stream_buffer(&Stream,InputBuffer,ReadSize+Remaining);
			Stream.error=(mad_error)0;
		}

		if (mad_frame_decode(&Frame,&Stream))
		{
			if (MAD_RECOVERABLE(Stream.error))
			{
				if (Stream.error!=MAD_ERROR_LOSTSYNC || Stream.this_frame!=GuardPtr)
					qDebug("*** WARNING: Recoverable frame level error in file %s (%s)", thePath.latin1(), mad_stream_errorstr(&Stream));
				continue;
			}
			else
				if (Stream.error==MAD_ERROR_BUFLEN)
					continue;
				else
				{
					qDebug("*** WARNING: Unrecoverable frame level error in file %s (%s)", thePath.latin1(), mad_stream_errorstr(&Stream));
					break;
				}
		}

		mad_timer_add(&Timer,Frame.header.duration);
		mad_synth_frame(&Synth,&Frame);
		thePosition++;
		for (uint i = 0; i < theChannels; i++)
		{	BufferData d = output(i).makeScratchSamples(Synth.pcm.length);
			if (!d.isNull())
				for (uint j = 0; j < Synth.pcm.length; j++)
					d[j] = float(MadFixedToSshort(Synth.pcm.samples[i][j])) / 32768.f;
			output(i).push(d);
		}
	}
	}
#endif
	plunge();
	if (MESSAGES) qDebug("Player (%s): Outahere", name().latin1());
}

void Player::processorStopped()
{
#ifdef HAVE_SNDFILE
	if (theMode == ModeSF)
		sf_close(theSndFile);
#endif
#ifdef HAVE_VORBISFILE
	if (theMode == ModeVF)
		ov_clear(&theVorbisFile);
#endif
#ifdef HAVE_MAD
	if (theMode == ModeMAD)
	{
	BstdFileDestroy(BstdFile);
	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);
	fclose(theMadFile);
	}
#endif
	thePosition = 0;
}

EXPORT_CLASS(Player, 0,2,0, Processor);
