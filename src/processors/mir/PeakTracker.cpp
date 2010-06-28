
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

#include <Plugin>
using namespace Geddei;

class PeakTracker : public CoProcessor
{
public:
	PeakTracker(): CoProcessor("PeakTracker") {}

private:
	virtual QString simpleText() const { return QChar(0x21FB); }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(0, 255, 255); }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties() { setupIO(1, 1); }
	virtual void updateFromProperties();
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void specifyInputSpace(QVector<uint>& _s) { _s.fill(1); }
	virtual void requireInputSpace(QVector<uint>& _s) { _s.fill(1); }
	virtual void specifyOutputSpace(QVector<uint>& _s) { _s.fill(input(0).capacity()); }
	virtual bool processorStarted();
	virtual int process();

	int m_maxTracks;
	float m_inertiaFactor;
	float m_weight;
	float m_searchiness;
	DECLARE_4_PROPERTIES(PeakTracker, m_maxTracks, m_inertiaFactor, m_weight, m_searchiness);

	struct Track
	{
		float frequency;
		float confidence;
		Track(): frequency(0), confidence(0) {}
		Track(float _f, float _c): frequency(_f), confidence(_c) {}
	};

	inline static bool byFrequency(Track const& _a, Track const& _b) { return _a.frequency < _b.frequency; }
	inline static bool byConfidence(Track const& _a, Track const& _b) { return _a.confidence > _b.confidence; }

	QVector<Track> m_tracks;
	Typed<SpectralPeak> m_type;
};

bool PeakTracker::processorStarted()
{
	m_tracks.resize(m_maxTracks);
	for (int i = 0; i < m_maxTracks; i++)
	{
		m_tracks[i].frequency = m_type->maxFrequency() * i / m_maxTracks;
		m_tracks[i].confidence = 0;
	}

	return true;
}

PropertiesInfo PeakTracker::specifyProperties() const
{
	return PropertiesInfo
			("MaxTracks", 50, "Number of tracks to follow.", true, "#", AV(1, 100))
			("InertiaFactor", 0.01f, "How changeable the classifier should be.", true, "i", AV("Balls", "i", 0.00001f, 1.f, AllowedValue::Log2))
			("Weight", 4, "Power to raise inertia factor to.", true, "w", AV("Weight", "w", 1, 10))
			("Searchiness", 3, "Power to raise search dynamism to.", true, "s", AV("Searchiness", "s", 1, 10));
}

bool PeakTracker::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	m_type = _inTypes[0];
	if (!m_type) return false;
	_outTypes[0] = _inTypes[0];
	return true;
}

void PeakTracker::updateFromProperties()
{
}

int PeakTracker::process()
{
	if (m_tracks.count() != m_maxTracks)
	{
		qSort(m_tracks.begin(), m_tracks.end(), byConfidence);
		m_tracks.resize(m_maxTracks);
//		qSort(m_tracks.begin(), m_tracks.end(), byFrequency);
	}

	QList<Track> tin;
	float total = 0;
	while (input(0).samplesReady())
	{
		const BufferData in = input(0).readSample();
		if (Mark::isEndOfTime(in))
			break;
		total += in[SpectralPeak::Value];
		tin.append(Track(in[SpectralPeak::Frequency], in[SpectralPeak::Value]));
	}

	for (int i = 0; i < tin.count(); i++)
		tin[i].confidence /= total;

	// Sort by frequency...
//	qSort(tin.begin(), tin.end(), byFrequency);

	QList<int> ordered;
	for (int b = 0; b < m_tracks.count(); b++)
		for (int c = 0; c <= b; c++)
			if (c == b || m_tracks[b].confidence > m_tracks[ordered[c]].confidence)
			{	ordered.insert(c, b);
				break;
			}

	int nout = 0;
	//for (int b = 0; b < m_tracks.count(); b++)
	foreach (int b, ordered)
	{
		if (tin.count())
		{
			int bestPeak = -1;
//			float tunnelVision = pow(m_tracks[b].confidence, m_searchiness);
			float factor = 0;
			if (m_tracks[b].confidence > 0)
			{
				for (int i = 0; i < tin.count(); i++)
				{
					//float f = lerp(tin[i].confidence, tin[i].confidence / fabsf(tin[i].frequency - m_tracks[b].frequency), m_inertiaFactor);
					float f = abs(m_tracks[b].frequency - tin[i].frequency) / min(m_tracks[b].frequency, tin[i].frequency);
					if (f < factor || bestPeak == -1)
					{
						bestPeak = i;
						factor = f;
					}
				}
				float df = abs(m_tracks[b].frequency - tin[bestPeak].frequency) / min(m_tracks[b].frequency, tin[bestPeak].frequency);
				if (df < 0.05)
				{
					// Small - perhaps 0.05?
					m_tracks[b].frequency = tin[bestPeak].frequency;
					m_tracks[b].confidence += tin[bestPeak].confidence * (1 - (df * 20));
					tin.removeAt(bestPeak);
				}

				m_tracks[b].confidence /= 2;
			}
			else
			{
				for (int i = 0; i < tin.count(); i++)
				{
					float f = tin[i].confidence;
					if (f > factor || bestPeak == -1)
					{
						bestPeak = i;
						factor = f;
					}
				}
				m_tracks[b].frequency = tin[bestPeak].frequency;
				m_tracks[b].confidence = 0.001;
				tin.removeAt(bestPeak);
			}
		}
		else
		{
			m_tracks[b].confidence *= .9f;
		}
		if (m_tracks[b].confidence < 0.001f)
			m_tracks[b].confidence = 0.f;
		nout++;
	}

	// Write our state to the output.
	BufferData out = output(0).makeScratchSamples(nout + 1);
	for (int i = 0; i < nout; i++)
	{
		out(i, SpectralPeak::Frequency) = m_tracks.value(i).frequency;
		out(i, SpectralPeak::Value) = m_tracks.value(i).confidence;
	}
	Mark::setEndOfTime(out.sample(nout));
	output(0) << out;
	return DidWork;
}

EXPORT_CLASS(PeakTracker, 0,1,0, Processor);
