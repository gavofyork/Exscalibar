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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(HAVE_ALSA)

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

#include "qfactoryexporter.h"

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace TransmissionTypes;

class ALSAPlayer: public CoProcessor
{
	QString theDevice;
	uint theChannels;
	uint thePeriodSize;
	uint thePeriods;
	snd_pcm_t *thePcmHandle;
	QVector<short> m_outData;

	virtual bool processorStarted();
	virtual void processorStopped();
	virtual int canProcess();
	virtual int process();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(240, 0, 160); }
	virtual void initFromProperties(const Properties &_p)
	{
		theDevice = _p["Device"].toString();
		theChannels = _p["Channels"].toInt();
		thePeriodSize = _p["Period Size"].toInt();
		thePeriods = _p["Periods"].toInt();
		setupIO(theChannels, 0);
	}
	virtual void specifyInputSpace(QVector<uint>& _s) { for (int i = 0; i < _s.count(); i++) _s[i] = thePeriodSize; }
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo	("Device", "hw:0,0", "The ALSA hardware device to open.")
								("Channels", 2, "The number of channels to use.")
								("Period Size", 1024, "The number of frames in each period.")
								("Periods", 4, "The number of periods in the outgoing buffer.");
	}
public:
	ALSAPlayer(): CoProcessor("ALSAPlayer", InConst), thePcmHandle(0) {}
};

QMutex g_alsaLock;

bool ALSAPlayer::verifyAndSpecifyTypes(const Types &, Types &)
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
	else if (snd_pcm_hw_params_set_rate_resample(thePcmHandle, hwparams, (uint)input(0).type().asA<Contiguous>().frequency()))
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n", (uint)input(0).type().asA<Contiguous>().frequency());
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
		snd_pcm_prepare(thePcmHandle);
		return true;
	}
	if (thePcmHandle)
		snd_pcm_close(thePcmHandle);
	thePcmHandle = 0;
	return false;
}

int ALSAPlayer::canProcess()
{
	int av = snd_pcm_avail(thePcmHandle);
	if (av >= (int)thePeriodSize)
		return CanWork;
	else if (av < 0)
		snd_pcm_recover(thePcmHandle, av, 0);
	return NoWork;
}

int ALSAPlayer::process()
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
		else if (pcmreturn < 0)
			snd_pcm_recover(thePcmHandle, written, 0);
	}
	return DidWork;
}

void ALSAPlayer::processorStopped()
{
	snd_pcm_close(thePcmHandle);
	thePcmHandle = 0;
}

EXPORT_CLASS(ALSAPlayer, 0,1,0, Processor);

#endif


