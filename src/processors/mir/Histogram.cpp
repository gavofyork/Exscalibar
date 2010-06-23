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

#include <cmath>
using namespace std;

#include <Plugin>
using namespace Geddei;

/**
 * @brief Takes what comes in and makes a histogram with it.
 */
class Histogram: public SubProcessor
{
public:
	Histogram() : SubProcessor("Histogram") {}

private:
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties();
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void processChunk(BufferDatas const& _in, BufferDatas& _out) const;

	uint m_rows;
	float m_period;
	float m_overlap;
	DECLARE_3_PROPERTIES(Histogram, m_rows, m_period, m_overlap);

	uint m_columns;
	uint m_count;
	uint m_step;
	QVector<float> m_window;
};

PropertiesInfo Histogram::specifyProperties() const
{
	return PropertiesInfo
		("Period", 12.f, "The period of time to derive each histogram from.", false, "P", AV(0.01, 20))
		("Overlap", 6.f, "The period of overlap to leave between successive histograms.", false, "O", AV(0.01, 20))
		("Rows", 50, "The vertical resolution of the histograms.", false, "R", AV(2, 100));
}

void Histogram::initFromProperties()
{
	setupIO(1, 1);
}

bool Histogram::verifyAndSpecifyTypes(Types const& inTypes, Types& outTypes)
{
	if (!inTypes[0].isA<Spectrum>()) return false;
	m_columns = inTypes[0].arity();
	outTypes[0] = Matrix(m_columns, m_rows, inTypes[0].asA<Contiguous>().frequency() / m_step, 0, 0);
	m_count = uint(m_period * inTypes[0].asA<Contiguous>().frequency());
	m_step = uint(m_overlap * inTypes[0].asA<Contiguous>().frequency());
	setupSamplesIO(m_count, m_step, 1);

	m_window.resize(m_count);
	for (uint i = 0; i < m_count; ++i)
		m_window[i] = 1.f;//.5f * (1.f - cos(2.f * M_PI * float(i) / float(m_count - 1)));

	return true;
}

void Histogram::processChunk(BufferDatas const& in, BufferDatas& out) const
{
	float mout = 0.f;
	for (uint i = 0; i < m_columns * m_rows; ++i)
		out[0][i] = 0.f;
	for (uint s = 0; s < m_count; ++s)
	{	for (uint c = 0; c < m_columns; ++c)
		{	for (uint r = 0; float(r + 1) / float(m_rows) < in[0](s, c) * m_window[s]; ++r)
			{
				out[0][r + c * m_rows] += 1.f;
				mout = max(mout, out[0][r + c * m_rows]);
			}
		}
	}
	if (mout > 0.f)
		for (uint c = 0; c < m_columns; ++c)
			for (uint r = 0; r < m_rows; ++r)
				out[0][r + c * m_rows] /= mout;
}

EXPORT_CLASS(Histogram, 1,0,1, SubProcessor);

