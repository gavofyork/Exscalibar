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

#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "bufferdata.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace TransmissionTypes;

class WaveGen: public CoProcessor
{
	enum WaveType { Sine, Square, Triangular } theWaveType;
	float theFrequency, theRate;
	int theChunk, theChunksPerPlunge, theStopAfterChunks;
	float m_phase;
	int m_chunksLeft;
	int m_hadChunks;

	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void initFromProperties(const Properties &properties);
	virtual PropertiesInfo specifyProperties() const;
	virtual void specifyOutputSpace(QVector<uint> &samples);
	virtual bool processorStarted();
	virtual int canProcess();
	virtual int process();
public:
	WaveGen(): CoProcessor("WaveGen", NotMulti) {}
};

bool WaveGen::processorStarted()
{
	m_phase = 0.f;
	m_chunksLeft = 0;
	m_hadChunks = 0;
	return true;
}

int WaveGen::canProcess()
{
	if (theStopAfterChunks && theStopAfterChunks > m_hadChunks)
		return WillNeverWork;
	return CanWork;
}

int WaveGen::process()
{
	BufferData d = output(0).makeScratchSamples(theChunk);
	for (int i = 0; i < theChunk; i++)
		d[i] = sin(2.0 * 3.14159265898 * (float(i) * theFrequency / theRate + m_phase));
	output(0) << d;
	m_phase += float(theChunk) * theFrequency / theRate;
	if (m_phase > 1.0) m_phase -= 1.0;
	if (theChunksPerPlunge && ++m_chunksLeft == theChunksPerPlunge)
	{	plunge();
		m_chunksLeft = 0;
	}
	m_hadChunks++;
	return DidWork;
}

bool WaveGen::verifyAndSpecifyTypes(const Types &, Types &outTypes)
{
	outTypes[0] = Wave(theRate);
	return true;
}

void WaveGen::specifyOutputSpace(QVector<uint> &samples)
{
	samples[0] = theChunk;
}

void WaveGen::initFromProperties(const Properties &properties)
{
	theChunksPerPlunge = 0;
	theFrequency = properties.get("Frequency").toDouble();
	theRate = properties.get("Sample Rate").toDouble();
	double pps = properties.get("PlungersPerSecond").toDouble();
	int spc = properties.get("SamplesPerChunk").toInt();
	if (pps > 0. && properties.get("ChunksPerPlunger").toInt())
	{	theChunksPerPlunge = properties.get("ChunksPerPlunger").toInt();
		theChunk = int(theRate / (pps * theChunksPerPlunge));
	}
	else if (spc)
		theChunk = spc;
	else
		theChunk = int(theRate / 2.);
	theStopAfterChunks = int(floor(properties["Stop After"].toDouble() * theRate / theChunk));
	theWaveType = Sine;
	setupIO(0, 1);
}

PropertiesInfo WaveGen::specifyProperties() const
{
	return PropertiesInfo("Frequency", 440., "Frequency of the wave to generate.")
						 ("Sample Rate", 44100., "Rate at which samples of the wave are produced.")
						 ("ChunksPerPlunger", 0, "Number of output chunks produced between each plunger. This is mutually exclusive with SamplesPerChunk.")
						 ("PlungersPerSecond", 0., "Number of plungers produced per second. This is mutually exclusive with SamplesPerChunk.")
						 ("SamplesPerChunk", 1024, "Number of samples in each chunk of output data. This is mutually exclusive with ChunksPerPlunger and PlungersPerSecond.")
						 ("Stop After", 0., "Number of plungers before this processors task has ended. If zero, continue indefinately. ");
}

EXPORT_CLASS(WaveGen, 0,1,0, Processor);
