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

class Bark: public SubProcessor
{
public:
	Bark() : SubProcessor("Bark") {}

private:
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual QString simpleText() const { return "B"; }

	QVector<int> m_bins;
	QVector<int> m_counts;
};

static const uint s_barkBands[26] = { 100, 200, 300, 400, 510, 630, 770, 920, 1080, 1270, 1480, 1720, 2000, 2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500, 20500, 27000 };
static const uint s_barkCentres[26] = { 50, 150, 250, 350, 450, 570, 700, 840, 1000, 1170, 1370, 1600, 1850, 2150, 2500, 2900, 3400, 4000, 4800, 5800, 7000, 8500, 10500, 13500, 17500, 22500 };

void Bark::processChunk(BufferDatas const& _ins, BufferDatas& _outs) const
{
	for (int i = 0; i < m_counts.size(); i++)
		_outs[0][i] = 0.f;
	for (int i = 0; i < m_bins.size(); i++)
		_outs[0][m_bins[i]] += _ins[0][i];
	for (int i = 0; i < m_counts.size(); i++)
		_outs[0][i] /= m_counts[i];
}

bool Bark::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	Typed<Spectrum> in = _inTypes[0];
	if (!in) return false;

	m_bins.resize(in->bins());
	int minBin = INT_MAX;
	int maxBin = -1;

	for (int i = 0; i < m_bins.size(); i++)
	{
		m_bins[i] = qLowerBound(s_barkBands, s_barkBands + 26, (uint)in->bandFrequency(i)) - s_barkBands;
		maxBin = max(maxBin, m_bins[i]);
		minBin = min(minBin, m_bins[i]);
	}

	if (minBin > maxBin)
		return false;

	m_counts = QVector<int>(maxBin - minBin + 1, 0);
	for (int i = 0; i < m_bins.size(); i++)
		m_counts[m_bins[i] -= minBin]++;

	QVector<float> bands(maxBin - minBin + 1);
	for (int i = 0; i < bands.count(); i++)
		bands[i] = i + minBin;//s_barkCentres[i + minBin];
	_outTypes[0] = ArbitrarySpectrum(bands, in->frequency(), in->max(), in->min());

	return true;
}

EXPORT_CLASS(Bark, 0,2,0, SubProcessor);
