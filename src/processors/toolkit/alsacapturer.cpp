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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(HAVE_ALSA)

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

#include "qfactoryexporter.h"
#include "qring.h"

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace Geddei;

class ALSACapturer: public CoProcessor
{
	QString theDevice;
	uint theChannels;
	uint thePeriodSize;
	uint thePeriods;
	uint theFrequency;
	enum { NormDisabled = 0, NormMax, NormRMS };
	snd_pcm_t *thePcmHandle;
	QVector<short> m_inData;

	float m_error;

	int m_normAlgorithm;
	QRing<float> m_normalisation;

	float m_dcOffsetLearnRate;
	QVector<float> m_inAvg;

	float m_max;

	virtual bool processorStarted();
	virtual void processorStopped();
	virtual int canProcess();
	virtual int process();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void initFromProperties(const Properties &_p)
	{
		theDevice = _p["Device"].toString();
		theChannels = _p["Channels"].toInt();
		theFrequency = _p["Frequency"].toInt();
		thePeriodSize = _p["Period Size"].toInt();
		thePeriods = _p["Periods"].toInt();
		updateFromProperties(_p);
		m_inAvg.resize(theChannels);
		setupIO(0, theChannels);
		setupVisual(30, 30, 30);
		m_normalisation.resize(theFrequency / thePeriodSize * _p["Normalisation Length"].toFloat());
		m_error = 0.f;
	}
	virtual void updateFromProperties(const Properties &_p)
	{
		m_dcOffsetLearnRate = _p["DC Offset Learn Rate"].toDouble();
		m_normAlgorithm = _p["Normalisation Algorithm"].toInt();
	}
	virtual void specifyOutputSpace(QVector<uint>& _s) { for (int i = 0; i < _s.count(); i++) _s[i] = thePeriodSize; }
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo	("Device", "hw:0,0", "The ALSA hardware device to open.", false, "D")
								("Channels", 2, "The number of channels to capture.", false, "#", AV("Channels", "#", 1, 6))
								("Frequency", 44100, "The frequency with which to sample at. { Hz }")
								("DC Offset Learn Rate", 0.7f, "How quickly changes in the DC offset are learned.", true, QChar(0x03B1), AV("Learning rate", QChar(0x03B1), 0.1f, 1.f, AllowedValue::Log2))
								("Normalisation Algorithm", 1, "How volume is normalised. { 0: Disabled; 1: Max; 2: RMS }", true, QChar(0x03BB), AV("Max", QString(QChar(0x2308)) + QChar(0x2309), 0) AVand("RMS", QChar(0x223F), 1))
								("Normalisation Length", 60.f, "The length of history for normalisation. { s }")
								("Period Size", 1024, "The number of frames in each period.")
								("Periods", 4, "The number of periods in the outgoing buffer.");
	}
	virtual QColor specifyOutlineColour() const { return QColor(255 * m_error, 127 * (1.f - m_error), 0); }
	virtual QString simpleText() const { return QChar(0x2386); }

public:
	ALSACapturer(): CoProcessor("ALSACapturer", OutConst), thePcmHandle(0) {}
};

bool ALSACapturer::verifyAndSpecifyTypes(const Types &, Types &outTypes)
{
	for (uint i = 0; i < theChannels; i++)
		outTypes[i] = Wave(theFrequency);
	return true;
}

bool ALSACapturer::processorStarted()
{
	m_max = 0.f;
	m_error = 0.f;

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
		qDebug() << "*** ACTUAL FREQUENCY" << f;
		m_inData.resize(thePeriodSize * theChannels);
		assert(!snd_pcm_nonblock(thePcmHandle, 1));
		snd_pcm_prepare(thePcmHandle);
		m_normalisation.clear();
		for (uint c = 0; c < numOutputs(); c++)
			m_inAvg[c] = 0.f;
		return true;
	}
	if (thePcmHandle)
		snd_pcm_close(thePcmHandle);
	thePcmHandle = 0;
	return false;
}

int ALSACapturer::canProcess()
{
	forever
	{
		int av = snd_pcm_readi(thePcmHandle, m_inData.data(), thePeriodSize);
		if (av > 0)
		{
			qDebug() << "REWINDING" << av;
			if (snd_pcm_rewindable(thePcmHandle) < av)
				qDebug() << "*** ALSA CAPTURER: CAN'T REWIND FRAMES!";
			int x = snd_pcm_rewind(thePcmHandle, av);
			if (x < av)
				qDebug() << "*** ALSA CAPTURER: CAN'T REWIND FRAMES!";
			if (x != av)
				av = x;
		}
	//	int av = snd_pcm_avail_update(thePcmHandle);
		if (av >= (int)thePeriodSize)
			return CanWork;
		else if (av >= 0)
		{
			int r = -(int)max(1u, (thePeriodSize - av) * 1000 / theFrequency);
			return r;
		}
		m_error = .99999f;
		snd_pcm_recover(thePcmHandle, av, 0);
	}
}

int ALSACapturer::process()
{
	m_error *= m_error;
	int count = snd_pcm_readi(thePcmHandle, m_inData.data(), thePeriodSize);
	static int totalCount = 0;
	totalCount += count;
	qDebug() <<  totalCount / theFrequency;
	if (count > 0)
	{
		BufferData d[theChannels];
		float avg[theChannels];
		for (uint c = 0; c < theChannels; c++)
			d[c] = output(c).makeScratchSamples(count), avg[c] = 0.f;

		if (count > thePeriodSize / 2.f)
		{
			float m = 0.f;
			if (m_normAlgorithm == NormMax)
				for (int i = 0; i < count; i++)
					m = max(m, fabsf(m_inData[i]));
			else if (m_normAlgorithm == NormRMS)
			{
				for (int i = 0; i < count; i++)
					m += m_inData[i] * m_inData[i];
				m = sqrt(m / count);
			}
			m_normalisation.shift(m);
		}
		float divisor = 0.f;
		if (m_normAlgorithm == NormMax && m_normalisation.count())
			for (int i = 0; i < m_normalisation.count(); i++)
				divisor = max(divisor, fabsf(m_normalisation[i]));
		else if (m_normAlgorithm == NormRMS && m_normalisation.count())
		{
			for (int i = 0; i < m_normalisation.count(); i++)
				divisor += m_normalisation[i] * m_normalisation[i];
			divisor = sqrt(divisor / m_normalisation.count());
		}
		else
			divisor = 32768.f;

		for (int i = 0; i < count; i++)
			for (uint c = 0; c < theChannels; c++)
			{
				d[c][i] = float(m_inData[i * theChannels + c]) / divisor;
				avg[c] += d[c][i];
				d[c][i] -= m_inAvg[c];
			}
		for (uint c = 0; c < theChannels; c++)
			output(c) << d[c], m_inAvg[c] = m_inAvg[c] * m_dcOffsetLearnRate + avg[c] / (float)count * (1.f - m_dcOffsetLearnRate);
		return DidWork;
	}
	else if (count < 0)
	{
		snd_pcm_recover(thePcmHandle, count, 0);
		m_error = .99999f;
		return DidWork;
	}
	return NoWork;
}

void ALSACapturer::processorStopped()
{
	snd_pcm_close(thePcmHandle);
	thePcmHandle = 0;
}

EXPORT_CLASS(ALSACapturer, 0,1,0, Processor);

#endif
