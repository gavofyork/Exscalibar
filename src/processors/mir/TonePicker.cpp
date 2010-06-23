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

class TonePicker : public CoProcessor
{
public:
	enum { UnityStrength = 0, AdditiveStrength, MultiplicativeStrength, MaxStrength };

	TonePicker();

private:
	virtual QString simpleText() const { return QChar(0x263C); }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(0, 255, 255); }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties();
	virtual void updateFromProperties();
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void specifyInputSpace(QVector<uint>& _s) { _s.fill(1); }
	virtual void requireInputSpace(QVector<uint>& _s) { _s.fill(1); }
	virtual void specifyOutputSpace(QVector<uint>& _s) { _s.fill(input(0).capacity()); }
	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();

	float m_distance;
	int m_maxHarmonics;
	int m_formants;
	float m_allowance;
	float m_minFrequency;
	int m_strength;
	bool m_multiplyByHarmonics;
	DECLARE_7_PROPERTIES(TonePicker, m_distance, m_maxHarmonics, m_formants, m_allowance, m_minFrequency, m_strength, m_multiplyByHarmonics);
};

TonePicker::TonePicker(): CoProcessor("TonePicker")
{
}

PropertiesInfo TonePicker::specifyProperties() const
{
	return PropertiesInfo
		("Distance", 0.05f, "Distance as fraction of frequency to search for harmonics.", true, "D", AV(0.005, 0.5, AllowedValue::Log10))
		("Allowance", 2.f, "Absolute vibrato allowance.", true, "V", AV(0.f, 100.f))
		("MaxHarmonics", 12, "Maximum number of harmonics to look for.", true, "M", AV(2, 36))
		("MinFrequency", 100.f, "Minimum frequancy.", true, "H", AVfrequency)
		("Strength", AdditiveStrength, "Strength calculation type.", true, "S", AVoption(UnityStrength, "1") AVoptionAnd(AdditiveStrength, "+") AVoptionAnd(MultiplicativeStrength, "*") AVoptionAnd(MaxStrength, QString(QChar(0x23A1)) + QChar(0x23A4)))
		("MultiplyByHarmonics", false, "Multiply final strength by harmonics found.", true, "h", AVbool)
		("Formants", 12, "Maximum number of formants to mop up.", true, "F", AV(1, 36));
}

void TonePicker::initFromProperties()
{
	setupIO(1, 1);
}

void TonePicker::updateFromProperties()
{
}

bool TonePicker::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	if (!_inTypes[0].isA<SpectralPeak>())
		return false;
	_outTypes[0] = _inTypes[0];
	return true;
}

bool TonePicker::processorStarted()
{
	return true;
}

int TonePicker::process()
{
	const BufferData in = input(0).readSamples();
	// For now, let's just hope that the last sample is an EndOfTime.

	double ts = Mark::timestamp(in);

	qDebug() << in.samples();
	QMap<float, int> peaks;
	for (uint s = 0; s < in.samples(); s++)
		if (in(s, SpectralPeak::Frequency) >= m_minFrequency)
			peaks[in(s, SpectralPeak::Frequency)] = s;

	for (int s = 0; s < peaks.count(); s++)
	{
		assert(peaks.count() <= (int)in.samples());
		float f = peaks.keys()[s];
		float fl = f * (1.f - m_distance);
		float fu = f * (1.f + m_distance);
		float strength;
		if (m_strength == UnityStrength)
			strength = 1.f;
		else
			strength = in(peaks.value(f), SpectralPeak::Value);

		int harmonics = 0;
		for (int m = 2; m < m_maxHarmonics; m++)
		{
			float mfl = m * fl - m_allowance;
			float mfu = m * fu + m_allowance;

			for (int n = 0; n < m_formants; n++)
			{
				int mx = -1;
				for (QMap<float, int>::iterator i = peaks.lowerBound(mfl); i != peaks.upperBound(mfu); i++)
					if (mx == -1 || in(mx, SpectralPeak::Value) < in(i.value(), SpectralPeak::Value))
						mx = i.value();

				if (mx == -1)
					break;
				else
				{
					peaks.remove(in(mx, SpectralPeak::Frequency));
					// TODO: Note that harmonic m has strength in(mx, SpectralPeak::Value)
					if (m_strength == AdditiveStrength)
						strength += in(mx, SpectralPeak::Value);
					else if (m_strength == MultiplicativeStrength)
						strength *= in(mx, SpectralPeak::Value);
					else if (m_strength == MaxStrength)
						strength = max<float>(strength, in(mx, SpectralPeak::Value));
					harmonics++;
				}
			}
		}
		if (m_multiplyByHarmonics)
			strength *= harmonics;
		BufferData out = output(0).makeScratchSample();
		out[SpectralPeak::Frequency] = f;
		out[SpectralPeak::Value] = strength;
		Mark::setTimestamp(out, ts);
		output(0).push(out);
	}
	return DidWork;
}

void TonePicker::processorStopped()
{
}

EXPORT_CLASS(TonePicker, 0,1,0, Processor);


