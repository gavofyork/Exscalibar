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

#if defined(HAVE_JACK) || 1

#include <jack/jack.h>

#include <Plugin>
using namespace Geddei;

class JackCapturer: public CoProcessor
{
public:
	JackCapturer(): CoProcessor("JackCapturer", OutConst), m_client(0) {}

	virtual bool processorStarted();
	virtual void processorStopped();
	virtual int canProcess();
	virtual int process();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);

	virtual void initFromProperties(const Properties &_p)
	{
		m_name = _p["Name"].toString();
		m_ports.clear();
		if (jackOpen())
		{
			jack_client_close(m_client);
			m_client = 0;

			Port p;
			p.lastChunk.resize(m_bufferSize);
			for (int i = 0; i < _p["Channels"].toInt(); i++)
				m_ports.append(p);
		}
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
	virtual QString simpleText() const { return "J"; }

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
		if (m_lastSize)
			qDebug() << "*** JACKCAPTURER: DISCARDING: " << nframes;
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
	return (m_lastSize > 0) ? CanWork : -1;//-(int)round((m_bufferSize / m_frequency * 1000.f) / 2.f);
}

int JackCapturer::process()
{
	if (!m_client)
		return WillNeverWork;
	if (!m_lastSize)
		return -1;//(int)round((m_bufferSize / m_frequency * 1000.f) / 2.f);
	for (int i = 0; i < m_ports.count(); i++)
	{
		int ls = min<int>(m_lastSize, m_bufferSize);
		BufferData b = output(i).makeScratchSamples(ls);
		if (b.isNull())
			continue;
		for (int s = 0; s < ls; s++)
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

bool JackCapturer::verifyAndSpecifyTypes(const Types &, Types &outTypes)
{
	outTypes = Wave(m_frequency);
	return true;
}

EXPORT_CLASS(JackCapturer, 2,0,0, Processor);

#endif

