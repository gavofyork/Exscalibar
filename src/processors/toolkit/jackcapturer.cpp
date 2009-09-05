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

#ifdef HAVE_JACK

#include <jack/jack.h>

#include "qfactoryexporter.h"

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace SignalTypes;

class JackCapturer: public Processor
{
	QFastWaitCondition theTransfer;
	QMutex theTransferLock;
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
		QMutexLocker lock(&theTransferLock);
		theWantFrames = nframes;
		while (theBuffer.isNull()) theTransfer.wait(&theTransferLock);
		for (uint i = 0; i < nframes; i++)
			theBuffer[i] = in[i];
		theWantFrames = 0;
		theDoneTransfer = true;
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
	
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &p);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &out);
	virtual bool processorStarted();
	virtual void processor();
	virtual void processorStopped();
	virtual void specifyOutputSpace(Q3ValueVector<uint> &samples);
	
public:
	JackCapturer(): Processor("JackCapturer", NotMulti, Guarded) {}
};

PropertiesInfo JackCapturer::specifyProperties() const
{
	return PropertiesInfo();
}

void JackCapturer::specifyOutputSpace(Q3ValueVector<uint> &samples)
{
	samples[0] = 8192;
}

void JackCapturer::initFromProperties(const Properties &)
{
	if ((theClient = jack_client_new(name())) == 0)
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
	if ((theClient = jack_client_new(name())) == 0)
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
		QMutexLocker lock(&theTransferLock);
		while (!theWantFrames) theTransfer.wait(&theTransferLock);
		theDoneTransfer = false;
		theBuffer = output(0).makeScratchSamples(theWantFrames);
		while (!theDoneTransfer) theTransfer.wait(&theTransferLock);
		output(0) << theBuffer;
		theBuffer.nullify();
		if (++theCount == 80)
		{	plunge();
			theCount = 0;
		}
	}
}

void JackCapturer::processorStopped()
{
	jack_client_close(theClient);
}

EXPORT_CLASS(JackCapturer, 0,1,0, Processor);

#endif

