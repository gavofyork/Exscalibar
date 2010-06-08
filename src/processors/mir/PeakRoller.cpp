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

#include "transmissiontype.h"
#include "bufferdata.h"
#include "processor.h"
#include "buffer.h"
using namespace Geddei;

#include "value.h"
#include "spectrum.h"
#include "matrix.h"
using namespace Geddei;

class PeakRoller : public CoProcessor
{
public:
	PeakRoller();

private:
	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual QString simpleText() const { return QChar(0x21FB); }

	struct Ball
	{
		float position;
		float inertia;
	};
	QVector<Ball> m_balls;

	float m_inertiaFactor;
	float m_weight;
	float m_repulsion;
	float m_searchiness;
};

PeakRoller::PeakRoller(): CoProcessor("PeakRoller", OutConst)
{
}

bool PeakRoller::processorStarted()
{
	// Arrayify according to multiplicity
	m_balls.resize(multiplicity());
	for (uint i = 0; i < multiplicity(); i++)
	{
		m_balls[i].position = input(0).type().asA<TransmissionType>().arity() / 2;
		m_balls[i].inertia = 0;
	}

	return true;
}

static float dist(float _a, float _b, float)
{
	float r = fabsf(_a - _b);
	return r;//min(r, _w - r);
}

int PeakRoller::process()
{
	int s = input(0).type().asA<TransmissionType>().arity();
	float in[s];
	input(0).readSample().copyTo(in, s);
	in[s - 1] = 0;

	QList<int> ordered;
	for (uint b = 0; b < multiplicity(); b++)
		for (uint c = 0; c <= b; c++)
			if (c == b || m_balls[b].inertia > m_balls[ordered[c]].inertia)
			{	ordered.insert(c, b);
				break;
			}

	int o = 0;
	foreach (int b, ordered)
	{
		int maxPeak = 0;
		float tunnelVision = pow(m_balls[b].inertia, m_searchiness);
		float maxFactor = 0;
		for (int i = 1; i < s; i++)
		{
			float f = lerp(in[i], in[i] / dist(i, m_balls[b].position, s), tunnelVision);
			if (f > maxFactor)
			{
				maxPeak = i;
				maxFactor = f;
			}
		}
		if (!maxPeak)
			maxPeak = rand() % input(0).type().asA<TransmissionType>().arity();
		m_balls[b].position = Geddei::lerp(m_balls[b].position, m_balls[b].position + (maxPeak > m_balls[b].position ? 1 : -1), pow(1 - m_balls[b].inertia, m_weight));
		//* (in[maxPeak] - max(in[floor(m_position)], in[ceil(m_position)]))
		m_balls[b].inertia = Geddei::lerp(m_balls[b].inertia, max(in[(int)floor(m_balls[b].position)], in[(int)ceil(m_balls[b].position)]) / max(.001f, in[maxPeak]), m_inertiaFactor);
		in[maxPeak] = 0;
		BufferData out = output(b).makeScratchSample(true);
		out[0] = input(0).type().asA<Spectrum>().bandFrequency(m_balls[b].position);
		out[1] = m_balls[b].inertia;
		out[2] = (maxPeak > 0) ? input(0).type().asA<Spectrum>().bandFrequency(maxPeak) : Geddei::StreamFalse;
		o++;
	}
	return DidWork;
}

void PeakRoller::processorStopped()
{
}

bool PeakRoller::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (!inTypes[0].isA<Spectrum>())
		return false;
	// inTypes[0].asA<Spectrum>().bandFrequency(1)
	// inTypes[0].asA<Spectrum>().bandFrequency(inTypes[0].asA<Spectrum>().bins() - 1)
	for (uint i = 0; i < multiplicity(); i++)
		outTypes[i] = MultiValue(3, inTypes[0].asA<Contiguous>().frequency(), QVector<MultiValue::Config>()
								 << MultiValue::Config(Qt::red, QColor(255, 0, 0, 32), 1, 0, 1, 100, "%")
								 << MultiValue::Config(QColor(0, 0, 0, 64), Qt::transparent, inTypes[0].asA<Spectrum>().bandFrequency(1), inTypes[0].asA<Spectrum>().bandFrequency(inTypes[0].asA<Spectrum>().bins() - 1), 2, 60.f, "bpm")
								 << MultiValue::Config(Qt::black, Qt::transparent, inTypes[0].asA<Spectrum>().bandFrequency(inTypes[0].asA<Spectrum>().bins() - 1), inTypes[0].asA<Spectrum>().bandFrequency(1), 0, 60.f, "bpm"), 2);
	return true;
}

void PeakRoller::initFromProperties(const Properties &p)
{
	updateFromProperties(p);
	setupIO(1, p["Balls"].toInt());
}

void PeakRoller::updateFromProperties(Properties const&_p)
{
	m_inertiaFactor = _p["InertiaFactor"].toFloat();
	m_weight = _p["Weight"].toFloat();
	m_repulsion = _p["Repulsion"].toFloat();
	m_searchiness = _p["Searchiness"].toFloat();
}

PropertiesInfo PeakRoller::specifyProperties() const
{
	return PropertiesInfo
			("Balls", 1, "How many balls.", false, "#", AV("Balls", "#", 1, 12))
			("InertiaFactor", 0.01f, "How changeable the classifier should be.", true, "i", AV("Balls", "i", 0.00001f, 1.f, AllowedValue::Log2))
			("Weight", 4, "Power to raise inertia factor to.", true, "w", AV("Weight", "w", 1, 10))
			("Searchiness", 3, "Power to raise search dynamism to.", true, "s", AV("Searchiness", "s", 1, 10))
			("Repulsion", 0.1f, "Repulsion between balls.", true, "r", AV("Repulsion", "r", 0.00001f, 1.f, AllowedValue::Log2));
}

EXPORT_CLASS(PeakRoller, 0,1,0, Processor);
