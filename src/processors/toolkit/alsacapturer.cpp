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

#ifdef HAVE_ALSA

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
	virtual void processor();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &) { setupIO(0, 2); }
public:
	ALSACapturer() : Processor("ALSACapturer") {}
};

bool ALSACapturer::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes)
{
	outTypes[0] = Wave(44100.0);
	outTypes[1] = Wave(44100.0);
	return true;
}

void ALSACapturer::processor()
{
	snd_pcm_t *pcm_handle;
	snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
	snd_pcm_hw_params_t *hwparams;
	char *pcm_name;
	pcm_name = strdup("hw:0,1");
	snd_pcm_hw_params_alloca(&hwparams);
	if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0) < 0)
	{
		fprintf(stderr, "Error opening PCM device %s\n", pcm_name);
		return;
	}
	if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0)
	{
		fprintf(stderr, "Can not configure this PCM device.\n");
		return;
	}
	uint rate = 44100;
	uint exact_rate;
	int dir;
	int periods = 2;
	int periodsize = 8192;
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
	exact_rate = snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &rate, &dir);
	if(exact_rate != rate)
	{
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n"
						"==> Using %d Hz instead.\n", rate, exact_rate);
	}
	if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 1) < 0)
	{
		fprintf(stderr, "Error setting channels.\n");
		return;
	}
	if (snd_pcm_hw_params_set_periods(pcm_handle, hwparams, periods, 0) < 0)
	{
		fprintf(stderr, "Error setting periods.\n");
		return;
	}
	if (snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, (periodsize * periods)>>2) < 0)
	{
		fprintf(stderr, "Error setting buffersize.\n");
		return;
	}
	if (snd_pcm_hw_params(pcm_handle, hwparams) < 0)
	{
		fprintf(stderr, "Error setting HW params.\n");
		return;
	}

	uint capdata[periodsize * 8];
	while(true)
	{
		int pcmreturn;
		if ((pcmreturn = snd_pcm_readi(pcm_handle, capdata, periodsize << 2)) > 0)
		{
			BufferData d0 = output(0).makeScratchSamples(pcmreturn), d1 = output(1).makeScratchSamples(pcmreturn);
			for(int i = 0; i < pcmreturn; i++)
			{	d0[i] = float(capdata[i]) / 32768.0;
				d1[i] = float(capdata[i + pcmreturn]) / 32768.0;
			}
		}
		else
			snd_pcm_prepare(pcm_handle);
	}

}

EXPORT_CLASS(ALSACapturer, 0,1,0, Processor);

#endif

