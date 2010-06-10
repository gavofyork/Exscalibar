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

#include <cstdlib>
#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "bufferdata.h"
#include "processor.h"
#include "buffer.h"
using namespace Geddei;

#include "value.h"
#include "spectrum.h"
#include "matrix.h"
using namespace Geddei;

class Pulser : public CoProcessor
{
public:
	Pulser();

private:
	float getExpectation();	// returns float value (0.f, 1.f] according to prior likelihood of imminent beat.

	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual QString simpleText() const { return QChar(0x21FB); }

	float m_periodPriorMean;
	float m_periodPriorSD;
	float m_sampleWidth;
	float m_frequency;
	int m_percentile;
	float m_factor;
	float m_mean;
	float m_priorFactor;
	float m_priorOffset;

	bool m_dynamicPrior;

	bool m_lastOutput;
	float m_threshold;
	float m_lastPulse;
	QList<float> m_sample;
	QList<float> m_sampleSorted;

	struct Pulse { float in; float out; float certainty; Pulse(float i, float o, float c): in(i), out(o), certainty(c){}};
	QList<Pulse> m_last;
};

Pulser::Pulser(): CoProcessor("Pulser")
{
}

bool Pulser::processorStarted()
{
	m_sample.clear();
	m_sampleSorted.clear();
	m_threshold = 1.f;
	m_mean = 0.f;
	m_lastOutput = true;
	return true;
}

float Pulser::getExpectation()
{
	return normalCDF(m_lastPulse, m_periodPriorMean * m_frequency, m_periodPriorSD * m_frequency) * m_priorFactor + m_priorOffset;
//	return max(.5f, 1.f -.1f * fabsf(m_lastPulse - (m_periodPriorMean * m_frequency)));
}

int Pulser::process()
{
	if (m_dynamicPrior)
	{
		int r = input(1).samplesReady();
		if (r > 1)
			input(1).readSamples(r - 1);

		if (r > 0)
		{
			BufferData info = input(1).peekSample();
			m_periodPriorMean = 1.f/info[0];
		}
	}

	float expectation = getExpectation();

	const BufferData in = input(0).readSample();

	float s = in[0];

	m_sample.append(s);
	m_mean += s / (m_sampleWidth * m_frequency);
	if (m_sample.size() > m_sampleWidth * m_frequency)
	{
		float old = m_sample.takeFirst();
		m_sampleSorted.removeOne(old);
		m_mean -= old / (m_sampleWidth * m_frequency);
	}
	{
		int i;
		for (i = 0; i <  m_sampleSorted.size() && m_sampleSorted[i] < s; i++);
		m_sampleSorted.insert(i, s);
	}

	float m = m_mean / m_sample.size() * m_sampleWidth * m_frequency;
	m_threshold = m /*m_sampleSorted[(m_sampleSorted.size() - 1) * m_percentile / 100]*/ * m_factor;

	BufferData out = output(0).makeScratchSample();
	if (s * expectation > m_threshold)
	{
		if (!m_lastOutput)
			m_lastPulse = 0;
		out[0] = m_lastOutput ? Geddei::StreamFalse : Geddei::StreamTrue;
		m_lastOutput = true;
	}
	else
	{
		m_lastOutput = false;
		out[0] = Geddei::StreamFalse;
	}
	m_lastPulse++;

	m_last.append(Pulse(in[0], out[0], 1));
	if (m_last.size() > 10 * m_frequency)	// 10 second limit.
		m_last.removeFirst();

	out[1] = m_threshold;
	out[2] = expectation;
	out[3] = in[0];
	out[4] = m_sampleSorted[(m_sampleSorted.size() - 1) * 25 / 100];
	out[5] = m_sampleSorted[(m_sampleSorted.size() - 1) * 50 / 100];
	out[6] = m_sampleSorted[(m_sampleSorted.size() - 1) * 75 / 100];
	out[7] = m_sampleSorted[(m_sampleSorted.size() - 1) * 100 / 100];
	out[8] = m;
	out[9] = s * expectation > m_threshold ? s * expectation : Geddei::StreamFalse;
	output(0).push(out);
	return DidWork;
}

void Pulser::processorStopped()
{
}

bool Pulser::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (inTypes[0].arity() != 1)
		return false;
	if (m_dynamicPrior)
	{
		if (!inTypes[1].isA<MultiValue>() && !inTypes[1].isA<Value>()) return false;
	}
	m_frequency = inTypes[0].asA<Contiguous>().frequency();
	outTypes[0] = MultiValue(10, inTypes[0].asA<Contiguous>().frequency(), 1, 0, QVector<MultiValue::Config>()
		<< MultiValue::Config(QColor(0, 0, 0, 64), QColor(0, 0, 0, 64))
		<< MultiValue::Config(Qt::red, QColor(255, 0, 0, 32), 1, 0, 1, 100, "%")
		<< MultiValue::Config(Qt::blue, Qt::transparent, 1, 0, 2, 100, "%")
		<< MultiValue::Config(Qt::gray, Qt::transparent, 1, 0, 3, 100, "%")
		<< MultiValue::Config(Qt::yellow, Qt::transparent, 1, 0, 8, 100, "%")
		<< MultiValue::Config(Qt::green, Qt::transparent, 1, 0, 9, 100, "%"));
	return true;
}

void Pulser::initFromProperties(const Properties &p)
{
	updateFromProperties(p);
	m_dynamicPrior = p["DynamicPrior"].toBool();
	setupIO(m_dynamicPrior ? 2 : 1, 1);
}

void Pulser::updateFromProperties(const Properties &p)
{
	m_periodPriorMean = p["PeriodPriorMean"].toFloat() / 1000.f;
	m_periodPriorSD = p["PeriodPriorSD"].toFloat() / 1000.f;
	m_priorFactor = p["PriorFactor"].toFloat();
	m_priorOffset = p["PriorOffset"].toFloat();

	m_mean /= m_sampleWidth;
	m_sampleWidth = max(1.f, p["SampleWidth"].toFloat() / 1000.f);
	m_mean *= m_sampleWidth;

	m_percentile = p["Percentile"].toInt();
	m_factor = p["Factor"].toFloat();
}

PropertiesInfo Pulser::specifyProperties() const
{
	return PropertiesInfo
			("PeriodPriorMean", 120, "Period prior. {ms}")
			("PeriodPriorSD", 20, "Period prior SD. {ms}")
			("PriorFactor", 1.0, "Factor applied to calculated prior. {x}")
			("PriorOffset", 0, "Offset applied to calculated prior. {+}")
			("SampleWidth", 1000.0, "Threshold sample width. {ms}")
			("Percentile", 100, "Percentile of sample for threshold. {%}")
			("DynamicPrior", false, "Dynamic prior.")
			("Factor", 4, "Factor to transform percentile value into threshold. {0..1}");
}

EXPORT_CLASS(Pulser, 0,1,0, Processor);





class StaticValue : public CoProcessor
{
public:
	StaticValue();

private:
	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual QString simpleText() const { return QChar(0x21FB); }

	float m_frequency;
	float m_value;
};

StaticValue::StaticValue(): CoProcessor("StaticValue")
{
}

bool StaticValue::processorStarted()
{
	return true;
}

int StaticValue::process()
{
	BufferData out = output(0).makeScratchSample();
	out[0] = m_value;
	return DidWork;
}

void StaticValue::processorStopped()
{
}

bool StaticValue::verifyAndSpecifyTypes(Types const&, Types& _outTypes)
{
	_outTypes[0] = Value(1, MultiValue::Config(Qt::black, Qt::transparent, 0, m_value));
	return true;
}

void StaticValue::initFromProperties(Properties const& _p)
{
	updateFromProperties(_p);
	setupIO(0, 1);
}

void StaticValue::updateFromProperties(Properties const& _p)
{
	m_value = _p["Value"].toFloat();
}

PropertiesInfo StaticValue::specifyProperties() const
{
	return PropertiesInfo
			("Value", 0, "Value to be given.");
}

EXPORT_CLASS(StaticValue, 0,1,0, Processor);





