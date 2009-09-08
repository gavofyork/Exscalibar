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

#if defined(HAVE_ALSA) || 1

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

#include "qfactoryexporter.h"

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace SignalTypes;

class ALSACapturer: public Processor
{
	QString theDevice;
	uint theChannels;
	uint thePeriodSize;
	uint thePeriods;
	uint theFrequency;
	snd_pcm_t *thePcmHandle;

	virtual bool processorStarted();
	virtual void processorStopped();
	virtual void processor();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &_p)
	{
		theDevice = _p["Device"].toString();
		theChannels = _p["Channels"].toInt();
		theFrequency = _p["Frequency"].toInt();
		thePeriodSize = _p["Period Size"].toInt();
		thePeriods = _p["Periods"].toInt();
		setupIO(0, theChannels);
	}
	virtual void specifyOutputSpace(Q3ValueVector<uint>& _s) { for (int i = 0; i < _s.count(); i++) _s[i] = thePeriodSize; }
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo	("Device", "hw:0,0", "The ALSA hardware device to open.")
								("Channels", 2, "The number of channels to capture.")
								("Frequency", 44100, "The frequency with which to sample at [in Hz].")
								("Period Size", 1024, "The number of frames in each period.")
								("Periods", 4, "The number of periods in the outgoing buffer.");
	}
public:
	ALSACapturer() : Processor("ALSACapturer"), thePcmHandle(0) {}
};

bool ALSACapturer::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes)
{
	for (uint i = 0; i < theChannels; i++)
		outTypes[i] = Wave(theFrequency);
	return true;
}

bool ALSACapturer::processorStarted()
{
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_hw_params_alloca(&hwparams);
	thePcmHandle = 0;
	if (snd_pcm_open(&thePcmHandle, theDevice.toLatin1(), SND_PCM_STREAM_CAPTURE, thePeriodSize * thePeriods) < 0)
		fprintf(stderr, "Error opening PCM device %s\n", qPrintable(theDevice));
	else if (snd_pcm_hw_params_any(thePcmHandle, hwparams) < 0)
		fprintf(stderr, "Can not configure this PCM device.\n");
	else if (snd_pcm_hw_params_set_access(thePcmHandle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
		fprintf(stderr, "Error setting access.\n");
	else if (snd_pcm_hw_params_set_format(thePcmHandle, hwparams, SND_PCM_FORMAT_S16_LE) < 0)
		fprintf(stderr, "Error setting format.\n");
	else if (snd_pcm_hw_params_set_rate_resample(thePcmHandle, hwparams, theFrequency))
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n", theFrequency);
	else if (snd_pcm_hw_params_set_channels(thePcmHandle, hwparams, theChannels) < 0)
		fprintf(stderr, "Error setting channels.\n");
	else if (snd_pcm_hw_params_set_periods(thePcmHandle, hwparams, thePeriods, 0) < 0)
		fprintf(stderr, "Error setting periods.\n");
	else if (snd_pcm_hw_params_set_buffer_size(thePcmHandle, hwparams, (thePeriodSize * thePeriods)) < 0)
		fprintf(stderr, "Error setting buffersize.\n");
	else if (snd_pcm_hw_params(thePcmHandle, hwparams) < 0)
		fprintf(stderr, "Error setting HW params.\n");
	else
	{
		uint f;
		snd_pcm_hw_params_get_rate_resample(thePcmHandle, hwparams, &f);
		qDebug() << "Using rate " << f;
		return true;
	}
	if (thePcmHandle)
		snd_pcm_close(thePcmHandle);
	thePcmHandle = 0;
	return false;
}

void ALSACapturer::processor()
{
	short indata[thePeriodSize * theChannels];
	while (guard())
	{
		int count;
		if ((count = snd_pcm_readi(thePcmHandle, indata, thePeriodSize)) > 0)
		{
			BufferData d[theChannels];
			for (uint c = 0; c < theChannels; c++)
				d[c] = output(c).makeScratchSamples(count);
			for (int i = 0; i < count; i++)
				for (uint c = 0; c < theChannels; c++)
					d[c][i] = float(indata[i * theChannels + c]) / 32768.f;
			for (uint c = 0; c < theChannels; c++)
				output(c) << d[c];
		}
		else
			snd_pcm_prepare(thePcmHandle);
	}
}

void ALSACapturer::processorStopped()
{
	snd_pcm_close(thePcmHandle);
	thePcmHandle = 0;
}

EXPORT_CLASS(ALSACapturer, 0,1,0, Processor);

#endif
