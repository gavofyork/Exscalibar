/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@kde.org                                                     *
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
	snd_pcm_t *thePcmHandle;
	QVector<short> m_outData;

	virtual bool processorStarted();
	virtual void processorStopped();
	virtual void process();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(240, 0, 160); }
	virtual void initFromProperties(const Properties &_p)
	{
		theDevice = _p["device"].toString();
		theChannels = _p["channels"].toInt();
		thePeriodSize = _p["periodsize"].toInt();
		thePeriods = _p["periods"].toInt();
		setupIO(theChannels, 0);
	}
	virtual void specifyInputSpace(QVector<uint>& _s) { for (int i = 0; i < _s.count(); i++) _s[i] = thePeriodSize; }
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo	("device", "hw:0,0", "The ALSA hardware device to open.")
								("channels", 2, "The number of channels to use.")
								("periodsize", 1024, "The number of frames in each period.")
								("periods", 4, "The number of periods in the outgoing buffer.");
	}
public:
	ALSAPlayer() : Processor("ALSAPlayer", NotMulti, Cooperative), thePcmHandle(0) {}
};

bool ALSAPlayer::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &)
{
	return true;
}

bool ALSAPlayer::processorStarted()
{
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_hw_params_alloca(&hwparams);
	thePcmHandle = 0;
	if (snd_pcm_open(&thePcmHandle, theDevice.toLatin1(), SND_PCM_STREAM_PLAYBACK, thePeriodSize * thePeriods) < 0)
		fprintf(stderr, "Error opening PCM device %s\n", qPrintable(theDevice));
	else if (snd_pcm_hw_params_any(thePcmHandle, hwparams) < 0)
		fprintf(stderr, "Can not configure this PCM device.\n");
	else if (snd_pcm_hw_params_set_access(thePcmHandle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
		fprintf(stderr, "Error setting access.\n");
	else if (snd_pcm_hw_params_set_format(thePcmHandle, hwparams, SND_PCM_FORMAT_S16_LE) < 0)
		fprintf(stderr, "Error setting format.\n");
	else if (snd_pcm_hw_params_set_rate_resample(thePcmHandle, hwparams, (uint)input(0).type().frequency()))
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n", (uint)input(0).type().frequency());
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
		m_outData.resize(thePeriodSize * theChannels);
		return true;
	}
	if (thePcmHandle)
		snd_pcm_close(thePcmHandle);
	thePcmHandle = 0;
	return false;
}

void ALSAPlayer::process()
{
	BufferData d[theChannels];
	for (uint c = 0; c < theChannels; c++)
	{
		d[c] = input(c).readSamples(thePeriodSize, false);
		assert(d[c].elements() == thePeriodSize);
	}
	for (uint i = 0; i < thePeriodSize; i++)
		for (uint c = 0; c < theChannels; c++)
			m_outData[i * theChannels + c] = short(d[c][i] * 32768.f);
	uint written = 0;
	while (written < thePeriodSize)
	{
		int pcmreturn = snd_pcm_writei(thePcmHandle, m_outData.data() + written * theChannels, thePeriodSize - written);
		if (pcmreturn > 0)
			written += pcmreturn;
		else
			snd_pcm_prepare(thePcmHandle);
	}
}

void ALSAPlayer::processorStopped()
{
	snd_pcm_close(thePcmHandle);
	thePcmHandle = 0;
}

EXPORT_CLASS(ALSAPlayer, 0,1,0, Processor);

#endif


