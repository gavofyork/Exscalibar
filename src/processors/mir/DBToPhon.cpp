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

/*
 * Docs from MatLab Phon module by Jeff Tackett (2005):
 *
 % PHON is the phon value in dB SPL that you want the equal
 % loudness curve to represent. (1phon = 1dB @ 1kHz)
 % SPL is the Sound Pressure Level amplitude returned for
 % each of the 29 frequencies evaluated by ISO226.
 % FREQ is the returned vector of frequencies that ISO226
 % evaluates to generate the contour.
 %
 % Desc: This function will return the equal loudness contour for
 % your desired phon level. The frequencies evaulated in this
 % function only span from 20Hz - 12.5kHz, and only 29 selective
 % frequencies are covered. This is the limitation of the ISO
 % standard.
 %
 % In addition the valid phon range should be 0 - 90 dB SPL.
 % Values outside this range do not have experimental values
 % and their contours should be treated as inaccurate.
 */

#include <cmath>
using namespace std;

#include <Plugin>
using namespace Geddei;

class ISO226: public SubProcessor
{
public:
	ISO226(): SubProcessor("ISO226") {}

private:
	virtual QString simpleText() const { return "ISO"; }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(30, 160, 128); }
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void processChunk(BufferDatas const& _ins, BufferDatas& _outs) const;

	QVector<float> m_Tf;
	QVector<float> m_Lu;
	QVector<float> m_af;
};

void ISO226::processChunk(BufferDatas const& _ins, BufferDatas& _outs) const
{
	for (int i = 0; i < m_af.size(); i++)
		_outs[0][i] = log10((pow(10.f, (_ins[0][i] - 94.f + m_Lu.at(i)) / 10.f * m_af.at(i)) - pow(0.4f * pow(10.f, (m_Tf.at(i) + m_Lu.at(i)) / 10.f - 9.f), m_af.at(i))) / 4.47e-3f + 1.15f) / 0.025f;
}

bool ISO226::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	Typed<Spectrum> in(_inTypes[0]);
	if (!in) return false;

	static const float s_fr[] = { 20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500 };
	static const float s_af[] = { 0.532, 0.506, 0.480, 0.455, 0.432, 0.409, 0.387, 0.367, 0.349, 0.330, 0.315, 0.301, 0.288, 0.276, 0.267, 0.259, 0.253, 0.250, 0.246, 0.244, 0.243, 0.243, 0.243, 0.242, 0.242, 0.245, 0.254, 0.271, 0.301 };
	static const float s_Lu[] = { -31.6, -27.2, -23.0, -19.1, -15.9, -13.0, -10.3, -8.1, -6.2, -4.5, -3.1, -2.0, -1.1, -0.4, 0.0, 0.3, 0.5, 0.0, -2.7, -4.1, -1.0, 1.7, 2.5, 1.2, -2.1, -7.1, -11.2, -10.7, -3.1 };
	static const float s_Tf[] = { 78.5, 68.7, 59.5, 51.1, 44.0, 37.5, 31.5, 26.5, 22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3.0, 2.2, 2.4, 3.5, 1.7, -1.3, -4.2, -6.0, -5.4, -1.5, 6.0, 12.6, 13.9, 12.3 };

	m_Tf.resize(in->bins());
	m_Lu.resize(in->bins());
	m_af.resize(in->bins());
	for (int i = 0; i < m_Tf.size(); i++)
	{
		float b = interpolateValue(s_fr, 29, in->bandFrequency(i));
		m_Tf[i] = cubicInterpolateIndex(s_Tf, 29, b);
		m_Lu[i] = cubicInterpolateIndex(s_Lu, 29, b);
		m_af[i] = cubicInterpolateIndex(s_af, 29, b);
	}

	// TODO: Set range to what it really is given band range and in's range.
	//in->setRange();

	_outTypes[0] = in;
	return true;
}

EXPORT_CLASS(ISO226, 1,0,0, SubProcessor);
