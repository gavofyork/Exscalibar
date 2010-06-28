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

class PeakFollower : public CoProcessor
{
public:
	PeakFollower(): CoProcessor("PeakFollower") {}

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
	virtual int process();

	int m_maxTracks;
	int m_maxArea;
	float m_deviation;
	float m_harmonicDeviation;
	bool m_breakOnFirst;
	float m_advantage;
	DECLARE_6_PROPERTIES(PeakFollower, m_maxTracks, m_maxArea, m_deviation, m_breakOnFirst, m_harmonicDeviation, m_advantage);

	struct Track
	{
		float frequency;
		float confidence;
		Track() {}
		Track(float _f, float _c): frequency(_f), confidence(_c) {}
	};
	struct Guess
	{
		float f;
		float c;
		int v;
	};

	inline static bool byFrequency(Track const& _a, Track const& _b) { return _a.frequency < _b.frequency; }
	inline static bool byConfidence(Track const& _a, Track const& _b) { return _a.confidence > _b.confidence; }

	QVector<Track> m_tracks;

	Typed<SpectralPeak> m_type;
};

PropertiesInfo PeakFollower::specifyProperties() const
{
	return PropertiesInfo
			("MaxTracks", 50, "Number of tracks to follow.", true, "#", AV(1, 100))
			("HarmonicDeviation", 0.05f, "Amount of deviation to allow for harmonics coalescing.", true, "h", AVlogUnity)
			("Deviation", 0.05f, "Amount of deviation to allow for interval coalescing.", true, "d", AVlogUnity)
			("BreakOnFirst", true, "BreakOnFirst.", true, "B", AVbool)
			("Advantage", 2.f, "Ad.", true, "v", AV(1.f, 8.f, AllowedValue::Log2))
			("MaxArea", 100, "", true, "A", AV(10, 200));
}

bool PeakFollower::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	m_type = _inTypes[0];
	if (!m_type) return false;
	_outTypes[0] = _inTypes[0];
	return true;
}

void PeakFollower::updateFromProperties()
{
}

// returns between 1 and 0
float priorOnFrequency(float _f)
{
	return 2 * cos(1 - normalPDFN(abs(log2(_f / 230))*.85)) - 1;
}

int PeakFollower::process()
{
	if (m_tracks.count() != m_maxTracks)
		m_tracks.resize(m_maxTracks);

	float mean = 0.f;
	QList<Track> tin;

	QList<Guess> guesses;
	while (input(0).samplesReady())
	{
		const BufferData in = input(0).readSample();
		if (Mark::isEndOfTime(in))
			break;
		tin.append(Track(in[SpectralPeak::Frequency], in[SpectralPeak::Value]));
		mean += in[SpectralPeak::Value];
/*
		float f = in[SpectralPeak::Frequency];
		float c = in[SpectralPeak::Value];
		float dummy;
		for (int i = 0; i < guesses.count(); i++)
			if (1.f - 2.f * abs(modf(guesses[i].f / f, &dummy) - .5f) < m_harmonicDeviation)
			{
				// accept as harmonic
				guesses[i].v++;
				guesses[i].f = lerp(guesses[i].f, f, guesses[i].c, c);
				guesses[i].c += c;
				goto OUTER;
			}

		Guess g; g.f = f; g.c = c; g.v = 1;
		guesses.append(g);

		OUTER:;*/
	}
	mean /= tin.count();

	// Calculate probable fundamentals...
	QList<Track> probFunds;

	// MonteCarlo GCD...
	{
		if (!isSorted(tin.begin(), tin.end(), byConfidence))
			qSort(tin.begin(), tin.end(), byConfidence);

		for (int i = 1; i < tin.count(); i++)
		{
			for (int k = 0; k < m_tracks.count(); k++)
			{
				int j = random() % i;
				int di = 1;
				int dj = 1;
				float fi = tin[i].frequency;
				float fj = tin[j].frequency;
				while (di * dj < m_maxArea)
				{
					if (fi > fj)
						fi = tin[i].frequency / ++di;
					else
						fj = tin[j].frequency / ++dj;
					if (di == dj)
						continue;
					float d = abs(fi - fj) / min(fi, fj);
					if (d < m_harmonicDeviation)
					{
						float f = lerp(fi, fj, tin[i].confidence, tin[j].confidence);
						float c = (tin[i].confidence + tin[j].confidence) / sqrt(di * dj) * (1 - d / max(.0001f, m_harmonicDeviation));
						QList<Track>::iterator lb = qLowerBound(probFunds.begin(), probFunds.end(), Track(f, 0), byFrequency);
						if (lb != probFunds.end() && abs((*lb).frequency - f) < min((*lb).frequency, f) * m_deviation)
						{
							(*lb).frequency = lerp((*lb).frequency, f, (*lb).confidence, c);
							(*lb).confidence += c;
						}
						else
							probFunds.insert(lb, Track(f, c));
						if (m_breakOnFirst)
							break;
					}
				}
			}
		}
	}

	// Three confidence factors now:
	// - Prior on fundamental frequency.
	// - Incoming probability from peaks.
	// - Continuity assumption from m_tracks.

	{
		// Go through tracks, reduce confidence by timespan.
/*		for (int i = 0; i < m_tracks; i++)
		{
			if ()
		}*/
		// Go through probfunds, add them to tracks, tapered by frequency prior, coalescing harmonics according to the one with the biggest confidence, giving lower harmonics a (double?) advantage.
		m_tracks.clear();
		for (int i = 0; i < probFunds.count(); i++)
			probFunds[i].confidence *= priorOnFrequency(probFunds[i].frequency);
		qSort(probFunds.begin(), probFunds.end(), byFrequency);
		for (int i = probFunds.count() - 1; i >= 0; i--)
		{
			QList<Track>::iterator lb = qLowerBound(probFunds.begin(), probFunds.begin() + i, Track(probFunds[i].frequency / 2.f * (1.f-m_harmonicDeviation), 0), byFrequency);
			QList<Track>::iterator ub = qUpperBound(lb, probFunds.begin() + i, Track(probFunds[i].frequency / 2.f * (1.f+m_harmonicDeviation), 0), byFrequency);
			for (QList<Track>::iterator it = lb; it != ub; ++it)
				if ((*it).confidence * m_advantage > probFunds[i].confidence)
				{
					probFunds.erase(probFunds.begin() + i);
					goto OK;
				}
			// Survived...
			foreach (Track t, probFunds)
				m_tracks.append(probFunds[i]);
			OK:;
		}
	}

	// Write our state to the output.
	int nout = m_tracks.count();
	BufferData out = output(0).makeScratchSamples(nout + 1);
	for (int i = 0; i < nout; i++)
	{
		out(i, SpectralPeak::Frequency) = m_tracks[i].frequency;
		out(i, SpectralPeak::Value) = m_tracks[i].confidence;
	}
	Mark::setEndOfTime(out.sample(nout));
	output(0) << out;
	return DidWork;
}

EXPORT_CLASS(PeakFollower, 0,1,0, Processor);


/*
	int workDone = 0;
		float chd = 1.f - sqr(1.f - m_harmonicDeviation);
				int u;
				int l;
				if (tin[i].frequency < tin[j].frequency)
					l = i, u = j;
				else
					l = j, u = i;
				for (int d = 1; d < 20; d++)
				{
					workDone++;
					float f = tin[l].frequency / d;
					if (f < 70.f) break;
					if (::divides(tin[u].frequency, f, chd))
					{
						float c = 1.f;//min(tin[u].confidence, tin[l].confidence);
						QList<Track>::iterator lb = qLowerBound(rawIntervals.begin(), rawIntervals.end(), Track(f, 0), byFrequency);
						if (lb != rawIntervals.end() && abs((*lb).frequency - f) < min((*lb).frequency, f) * m_deviation)
						{
							(*lb).frequency = lerp((*lb).frequency, f, (*lb).confidence, c);
							(*lb).confidence += c;
						}
						else
							rawIntervals.insert(lb, Track(f, c));
						break;
					}
				}*/
	/*
	for (int i = 0; i < tin.count(); i++)
		for (int j = i + 1; j < tin.count(); j++)
		{
			float c = tin[i].confidence * tin[j].confidence;
			if (c > mean * mean / 2)
			{
				workDone++;
				float f = abs(tin[i].frequency - tin[j].frequency);
				QList<Track>::iterator lb = qLowerBound(rawIntervals.begin(), rawIntervals.end(), Track(f, 0), byFrequency);
				if (lb != rawIntervals.end() && abs((*lb).frequency - f) < min((*lb).frequency, f) * m_deviation)
				{
					(*lb).frequency = lerp((*lb).frequency, f, (*lb).confidence, c);
					(*lb).confidence += c;
				}
				else
					rawIntervals.insert(lb, Track(f, c));
			}
		}*/

//	qDebug() << rawIntervals.count() << workDone << tin.count() << (tin.count() * tin.count() / 2);
	// Until no more intervals left...
	//   Pass a <deviation>-wide window over each interval to get interval with greatest window-sum.
	//   Coalesce intervals within window.

	/*	{
			QList<Track> intervals;
			qSort(rawIntervals.begin(), rawIntervals.end(), byFrequency);
			while (intervals.count() < m_tracks.count() && rawIntervals.count())
			{
				float bestSum = 0;
				int bestInterval = -1;
				QList<Track>::iterator bestLb;
				QList<Track>::iterator bestUb;
				for (int i = 0; i < rawIntervals.count(); i++)
				{
					QList<Track>::iterator ub = qUpperBound(rawIntervals.begin(), rawIntervals.end(), Track(rawIntervals[i].frequency * (1 + m_deviation), 0), byFrequency);
					QList<Track>::iterator lb = qLowerBound(rawIntervals.begin(), rawIntervals.end(), Track(rawIntervals[i].frequency * (1 - m_deviation), 0), byFrequency);
					float sum = 0.f;
					for (QList<Track>::const_iterator j = lb; j != ub; j++)
						sum += (*j).confidence;
					if (bestInterval == -1 || sum > bestSum)
					{
						bestInterval = i;
						bestSum = sum;
						bestLb = lb;
						bestUb = ub;
					}
				}
				intervals.append(Track(rawIntervals[bestInterval].frequency, bestSum));
				rawIntervals.erase(bestLb, bestUb);
			}
			for (int i = 0; i < m_tracks.count(); i++)
			{
				if (i < intervals.count())
					m_tracks[i] = intervals[i];
				else
					m_tracks[i].confidence = m_tracks[i].frequency = 0;
			}
		}
	*/
