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

class PeakPicker : public CoProcessor
{
public:
	PeakPicker(): CoProcessor("PeakPicker") {}

private:
	virtual QString simpleText() const { return QChar(0x21FB); }
	virtual QColor outlineColour() const { return QColor::fromHsv(0, 255, 255); }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties() { setupIO(1, 1); setupVisual(10, 10, 1, 10, 10, true); }
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void specifyInputSpace(QVector<uint>& _s) { _s.fill(1); }
	virtual void requireInputSpace(QVector<uint>& _s) { _s.fill(1); }
	virtual void specifyOutputSpace(QVector<uint>& _s) { _s.fill(m_type->bins() / 2); }
	virtual int process();

	bool m_peaks;
	DECLARE_1_PROPERTY(PeakPicker, m_peaks);

	Typed<Spectrum> m_type;
};

PropertiesInfo PeakPicker::specifyProperties() const
{
	return PropertiesInfo
			("Peaks", 50, "Maximum number of peaks to pick from each spectrum.", false, "#", AV(1, 100));
}

bool PeakPicker::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	m_type = _inTypes[0];
	if (!m_type) return false;
	_outTypes[0] = SpectralPeak(m_type->maxFrequency(), m_type->minFrequency(), m_type->max(), m_type->min());
	assert(!_outTypes[0]->isNull());
	return true;
}

int PeakPicker::process()
{
	const BufferData in = input(0).readSample();

	bool gu = false;
	int b = m_type->bins() - 1;
	for (int i = 1; i < b; i++)
	{
		float const& a = in[i - 1];
		float const& b = in[i];
		float const& c = in[i + 1];
		if ((a < b && b <= c) || (a <= b && b < c))
			gu = true;
		else if ((a < b && b > c) || gu)
		{
			BufferData out = output(0).makeScratchSample();
			float p = (a - c) / 2 / (a - 2 * b + c);
			out[SpectralPeak::Frequency] = m_type->bandFrequency(i + p);
			out[SpectralPeak::Value] = b - (a - c) * p / 4.f;
			out[SpectralPeak::Spread] = 0.f;
			out[SpectralPeak::HalfSpread] = 0.f;
			output(0) << out;
			gu = false;
		}
	}
//	BufferData out = output(0).makeScratchSample();
//	Mark::setEndOfTime(out);
//	output(0) << out;
	return DidWork;
}

EXPORT_CLASS(PeakPicker, 0,1,0, Processor);

