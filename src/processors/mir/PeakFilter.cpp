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

class PeakFilter : public CoProcessor
{
public:
	PeakFilter(): CoProcessor("PeakFilter") {}

private:
	virtual QString simpleText() const { return QChar(0x21FB); }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(0, 255, 255); }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties() { setupIO(1, 1); }
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void specifyOutputSpace(QVector<uint>& _s) { _s.fill(input(0).capacity()); }
	virtual int process();

	int m_maxPeaks;
	DECLARE_1_PROPERTY(PeakFilter, m_maxPeaks);
};

PropertiesInfo PeakFilter::specifyProperties() const
{
	return PropertiesInfo
			("MaxPeaks", 50, "Maximum number of peaks to output.", true, "#", AV(1, 100));
}

bool PeakFilter::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	Typed<SpectralPeak> type = _inTypes[0];
	if (!type) return false;
	_outTypes[0] = _inTypes[0];
	return true;
}

int PeakFilter::process()
{
	QMap<float, float> tin;
	while (input(0).samplesReady())
	{
		const BufferData in = input(0).readSample();
		if (Mark::isEndOfTime(in))
			break;
		tin.insert(in[SpectralPeak::Value], in[SpectralPeak::Frequency]);
	}

	int nout = min(m_maxPeaks, tin.size());
	BufferData out = output(0).makeScratchSamples(nout + 1);
	QMap<float, float>::iterator it = tin.end();
	for (int i = 0; i < nout; i++)
	{
		it--;
		out(i, SpectralPeak::Frequency) = it.value();
		out(i, SpectralPeak::Value) = it.key();
	}
	Mark::setEndOfTime(out.sample(nout));
	output(0) << out;
	return DidWork;
}

EXPORT_CLASS(PeakFilter, 0,1,0, Processor);


