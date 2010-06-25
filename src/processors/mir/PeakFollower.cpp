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
	DECLARE_5_PROPERTIES(PeakFollower, m_maxTracks, m_maxArea, m_deviation, m_breakOnFirst, m_harmonicDeviation);

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
	if (m_tracks.count() != m_maxTracks)
		m_tracks.resize(m_maxTracks);
}

template<class T>
bool divides(T _n, T _d, T _e)
{
	T dummy;
	T x = modf(_n / _d, &dummy);
	if (x > .5)
		return 1.0 - x < _e;
	else
		return x < _e;
}

template <typename RandomAccessIterator, typename LessThan>
inline bool isSorted(RandomAccessIterator _start, RandomAccessIterator _end, LessThan _lessThan)
{
	if (_start != _end)
	{
		RandomAccessIterator lit = _start;
		RandomAccessIterator it = lit;
		for (it++; it != _end; ++it, ++lit)
			if (!_lessThan(*lit, *it))
				return false;
	}
	return true;
}

int PeakFollower::process()
{
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

	// Calculate intervals...
	QList<Track> rawIntervals;
//	rawIntervals = tin;
	int workDone = 0;

	// MonteCarlo GCD...
	{
		if (!isSorted(tin.begin(), tin.end(), byConfidence))
			qSort(tin.begin(), tin.end(), byConfidence);

		float chd = 1.f - sqr(1.f - m_harmonicDeviation);
		for (int i = 1; i < tin.count(); i++)
		{
			for (int k = 0; k < m_maxTracks; k++)
			{
				int j = random() % i;
				int di = 1;
				int dj = 1;
				float fi = tin[i].frequency;
				float fj = tin[j].frequency;
				while (di * dj < 100)
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
						float c = (tin[i].confidence + tin[j].confidence) / sqrt(di * dj);
						QList<Track>::iterator lb = qLowerBound(rawIntervals.begin(), rawIntervals.end(), Track(f, 0), byFrequency);
						if (lb != rawIntervals.end() && abs((*lb).frequency - f) < min((*lb).frequency, f) * m_deviation)
						{
							(*lb).frequency = lerp((*lb).frequency, f, (*lb).confidence, c);
							(*lb).confidence += c;
						}
						else
							rawIntervals.insert(lb, Track(f, c));
						if (m_breakOnFirst)
							break;
					}
				}
/*
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
			}
		}
	}

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

	m_tracks = QVector<Track>::fromList(rawIntervals);
/*	{
		QList<Track> intervals;
		qSort(rawIntervals.begin(), rawIntervals.end(), byFrequency);
		while (intervals.count() < m_maxTracks && rawIntervals.count())
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
	// Write our state to the output.
	int nout = m_tracks.count();
/*	foreach (Track const& t, m_tracks)
		if (t.confidence > m_requirement)
			nout++;
		else
			break;*/
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

