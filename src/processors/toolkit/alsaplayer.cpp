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

class ALSAPlayer: public Processor
{
	QString theDevice;
	uint theChannels;
	uint thePeriodSize;
	uint thePeriods;

	virtual void processor();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &_p)
	{
		theDevice = _p["device"].toString();
		theChannels = _p["channels"].toInt();
		thePeriodSize = _p["periodsize"].toInt();
		thePeriods = _p["periods"].toInt();
		setupIO(theChannels, 0);
	}
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo	("device", "hw:0,1", "The ALSA hardware device to open.")
								("channels", 2, "The number of channels to use.")
								("periodsize", 8192, "The number of frames in each period.")
								("periods", 2, "The number of periods in the outgoing buffer.");
	}
public:
	ALSAPlayer() : Processor("ALSAPlayer") {}
};

bool ALSAPlayer::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &)
{
	return true;
}

void ALSAPlayer::processor()
{
	snd_pcm_t *pcm_handle;
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
	snd_pcm_hw_params_t *hwparams;
	char *pcm_name;
	pcm_name = strdup(theDevice.toLatin1());
	snd_pcm_hw_params_alloca(&hwparams);
	if (snd_pcm_open(&pcm_handle, pcm_name, stream, thePeriodSize * thePeriods) < 0)
	{
		fprintf(stderr, "Error opening PCM device %s\n", pcm_name);
		return;
	}
	if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0)
	{
		fprintf(stderr, "Can not configure this PCM device.\n");
		return;
	}
	uint rate = input(0).type().frequency();
	if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
	{
		fprintf(stderr, "Error setting access.\n");
		return;
	}
	if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16_LE) < 0)
	{
		fprintf(stderr, "Error setting format.\n");
		return;
	}
	if (snd_pcm_hw_params_set_rate_resample(pcm_handle, hwparams, rate))
	{
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n", rate);
		return;
	}
	if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, theChannels) < 0)
	{
		fprintf(stderr, "Error setting channels.\n");
		return;
	}
	if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, thePeriods, 0) < 0)
	{
		fprintf(stderr, "Error setting periods.\n");
		return;
	}
	if (snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, (thePeriodSize * thePeriods)) < 0)
	{
		fprintf(stderr, "Error setting buffersize.\n");
		return;
	}
	if (snd_pcm_hw_params(pcm_handle, hwparams) < 0)
	{
		fprintf(stderr, "Error setting HW params.\n");
		return;
	}

	short outdata[thePeriodSize * theChannels];
	while (thereIsInputForProcessing(thePeriodSize))
	{
		BufferData d[theChannels];
		for (uint c = 0; c < theChannels; c++)
		{
			d[c] = input(c).readSamples(thePeriodSize, false);
			assert(d[c].elements() == thePeriodSize);
		}
		for (uint i = 0; i < thePeriodSize; i++)
			for (uint c = 0; c < theChannels; c++)
				outdata[i * theChannels + c] = short(d[c][i] * 32768.f);
		uint written = 0;
		while (written < thePeriodSize)
		{
			int pcmreturn = snd_pcm_writei(pcm_handle, outdata + written * theChannels, thePeriodSize - written);
			if (pcmreturn > 0)
				written += pcmreturn;
			else
				snd_pcm_prepare(pcm_handle);
		}
	}

}

EXPORT_CLASS(ALSAPlayer, 0,1,0, Processor);

#endif


