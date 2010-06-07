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

#if defined(HAVE_JACK) || 1

#include <jack/jack.h>

#include "qfactoryexporter.h"

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace TransmissionTypes;

class JackCapturer: public CoProcessor
{
public:
	JackCapturer(): CoProcessor("JackCapturer", OutConst), m_client(0) {}

	virtual bool processorStarted();
	virtual void processorStopped();
	virtual int canProcess();
	virtual int process();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);

	virtual void initFromProperties(const Properties &_p)
	{
		m_name = _p["Name"].toString();
		jackOpen();
		jack_client_close(m_client);
		m_client = 0;

		m_ports.clear();
		Port p;
		p.lastChunk.resize(m_bufferSize);
		for (int i = 0; i < _p["Channels"].toInt(); i++)
			m_ports.append(p);
		setupIO(0, m_ports.count());
	}
	virtual void updateFromProperties(Properties const&) {}
	virtual void specifyOutputSpace(QVector<uint>& _s) { for (int i = 0; i < _s.count(); i++) _s[i] = m_bufferSize; }
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo	("Name", "Geddei/" + name(), "The Jack device name.")
								("Channels", 2, "The number of channels to capture.", false, "#", AV("Channels", "#", 1, 6));
	}
//	virtual QColor specifyOutlineColour() const { return QColor(255 * m_error, 127 * (1.f - m_error), 0); }
	virtual QString simpleText() const { return QChar(0x2386); }

private:
	bool jackOpen()
	{
		JackStatus js;
		if ((m_client = jack_client_open(m_name.toLocal8Bit(), JackNullOption, &js)) == 0)
		{
			qWarning("*** ERROR: JACK server not running!");
			return false;
		}
		m_frequency = jack_get_sample_rate(m_client);
		m_bufferSize = jack_get_buffer_size(m_client);
		return true;
	}

	static int jackProcess(jack_nframes_t nframes, void *arg) { return static_cast<JackCapturer *>(arg)->processJack(nframes); }
	static void jackShutdown(void *arg) { static_cast<JackCapturer *>(arg)->shutdown(); }
	static void jackPortRegistration(jack_port_id_t _port, int _reg, void* _arg) { static_cast<JackCapturer *>(_arg)->portRegistration(_port, _reg); }

	inline int processJack(jack_nframes_t nframes)
	{
		for (int i = 0; i < m_ports.count(); i++)
			memcpy(m_ports[i].lastChunk.data(), (jack_default_audio_sample_t *)jack_port_get_buffer(m_ports[i].port, nframes), nframes * sizeof(jack_default_audio_sample_t));
		m_lastSize = nframes;
		return 0;
	}
	inline void shutdown()
	{
		m_client = 0;
	}
	inline void tryToConnect(jack_port_t* _p)
	{
		if (jack_port_is_mine(m_client, _p) != TRUE && jack_port_flags(_p) & JackPortIsOutput)
		{
			qDebug() << "Found acceptable output.";
			for (int i = 0; i < m_ports.count(); i++)
				if (!jack_port_connected(m_ports[i].port))
				{
					jack_connect(m_client, jack_port_name(_p), jack_port_name(m_ports[i].port));
					break;
				}
		}
	}
	inline void portRegistration(jack_port_id_t, int _reg)
	{
		if (_reg)
			m_retryConnect = true;
	}

	struct Port
	{
		QVector<float> lastChunk;
		jack_port_t* port;
	};

	float m_frequency;
	int m_bufferSize;
	int m_lastSize;
	QList<Port> m_ports;
	jack_client_t* m_client;

	bool m_retryConnect;
	QString m_name;
};

bool JackCapturer::processorStarted()
{
	if (!numOutputs()) return false;

	if (!jackOpen()) return false;

	for (int i = 0; i < m_ports.size(); i++)
		m_ports[i].port = jack_port_register(m_client, QString("input%1").arg(i).toLocal8Bit(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

	jack_set_process_callback(m_client, JackCapturer::jackProcess, this);
	jack_on_shutdown(m_client, JackCapturer::jackShutdown, this);
	jack_set_port_registration_callback(m_client, JackCapturer::jackPortRegistration, this);

	if (jack_activate(m_client))
	{
		qWarning("*** ERROR: Cannot activate JACK client!");
		return false;
	}

	m_retryConnect = true;

	return true;
}

int JackCapturer::canProcess()
{
	if (!m_client)
		return WillNeverWork;
	if (m_retryConnect)
	{
		for (const char** p = jack_get_ports(m_client, 0, 0, 0); *p; p++)
			tryToConnect(jack_port_by_name(m_client, *p));
		m_retryConnect = false;
	}
	return (m_lastSize > 0) ? CanWork : -(int)round((m_bufferSize / m_frequency * 1000.f) / 2.f);
}

int JackCapturer::process()
{
	if (!m_client)
		return WillNeverWork;
	if (!m_lastSize)
		return -(int)round((m_bufferSize / m_frequency * 1000.f) / 2.f);
	for (int i = 0; i < m_ports.count(); i++)
	{
		BufferData b = output(i).makeScratchSamples(m_lastSize);
		if (b.isNull())
			return DidWork;
		for (int s = 0; s < m_lastSize; s++)
			b[s] = m_ports[i].lastChunk[s];
		output(i) << b;
	}
	m_lastSize = 0;
	return DidWork;
}

void JackCapturer::processorStopped()
{
	if (m_client)
		jack_client_close(m_client);
	m_client = 0;
}

bool JackCapturer::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes)
{
	outTypes = Wave(m_frequency);
	return true;
}

EXPORT_CLASS(JackCapturer, 2,0,0, Processor);

#if 0
class JackCapturer: public HeavyProcessor
{
	QFastWaitCondition theTransfer;
	QFastMutex theTransferLock;
	bool theKeepGoing, theDoneTransfer;
	jack_nframes_t theWantFrames;
	BufferData theBuffer;

	jack_port_t *theInputPort;
	jack_client_t *theClient;
	uint theFreq;
	uint theCount;

	int process(jack_nframes_t nframes)
	{
		jack_default_audio_sample_t *in = (jack_default_audio_sample_t *)jack_port_get_buffer(theInputPort, nframes);

		if (!isRunning())
			return 0;

//		QFastMutexLocker lock(&theTransferLock);
//		theWantFrames = nframes;
//		while (theBuffer.isNull()) theTransfer.wait(&theTransferLock);
		theBuffer = output(0).makeScratchSamples(nframes);
		if (theBuffer.isNull())
			return 0;
		for (uint i = 0; i < nframes; i++)
			theBuffer[i] = in[i];
		output(0) << theBuffer;
//		theWantFrames = 0;
//		theDoneTransfer = true;
		return 0;
	}

	void shutdown()
	{
		theKeepGoing = false;
	}

	static int jackProcess(jack_nframes_t nframes, void *arg)
	{
		return static_cast<JackCapturer *>(arg)->process(nframes);
	}

	static void jackShutdown(void *arg)
	{
		static_cast<JackCapturer *>(arg)->shutdown();
	}

	void JackCapturer::processorStarted()
	{
	}

	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &p);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &out);
	virtual bool processorStarted();
	virtual void processor();
	virtual void processorStopped();
	virtual void specifyOutputSpace(QVector<uint> &samples);
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(240, 0, 160); }

public:
	JackCapturer(): HeavyProcessor("JackCapturer", NotMulti, Guarded) {}
};

PropertiesInfo JackCapturer::specifyProperties() const
{
	return PropertiesInfo();
}

void JackCapturer::specifyOutputSpace(QVector<uint> &samples)
{
	samples[0] = 8192;
}

void JackCapturer::initFromProperties(const Properties &)
{
	JackStatus js;
	if ((theClient = jack_client_open(name().toLocal8Bit(), JackNullOption, &js)) == 0)
		qWarning("*** WARNING: JACK server not running!");
	else
	{	setupIO(0, theClient ? 1 : 0);
		theFreq = jack_get_sample_rate(theClient);
		jack_client_close(theClient);
	}
}

bool JackCapturer::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &out)
{
	out = Wave(theFreq);
	return true;
}

bool JackCapturer::processorStarted()
{
	if (!numOutputs()) return false;
	theBuffer.nullify();
	theKeepGoing = true;
	theCount = 0;
	JackStatus js;
	if ((theClient = jack_client_open(name().toLocal8Bit(), JackNullOption, &js)) == 0)
	{
		qWarning("*** ERROR: JACK server not running!");
		return false;
	}
	jack_set_process_callback(theClient, JackCapturer::jackProcess, this);
	jack_on_shutdown(theClient, JackCapturer::jackShutdown, this);
	theInputPort = jack_port_register(theClient, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	if (jack_activate(theClient))
	{
		qWarning("*** ERROR: Cannot activate JACK client!");
		return false;
	}
	return true;
}

void JackCapturer::processor()
{
	while (theKeepGoing)
	{
		sleep(1);
/*		QFastMutexLocker lock(&theTransferLock);
		while (!theWantFrames) theTransfer.wait(&theTransferLock);
		theDoneTransfer = false;
		theBuffer = output(0).makeScratchSamples(theWantFrames);
		while (!theDoneTransfer) theTransfer.wait(&theTransferLock);
		output(0) << theBuffer;
		theBuffer.nullify();
		if (++theCount == 80)
		{	plunge();
			theCount = 0;
		}*/
	}
}

void JackCapturer::processorStopped()
{
	jack_client_close(theClient);
}

EXPORT_CLASS(JackCapturer, 0,1,0, Processor);
#endif
#endif

