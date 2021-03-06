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

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Oscilloscope: public CoProcessor
{
public:
	Oscilloscope(): CoProcessor("Oscilloscope", NotMulti) {}

private:
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties();
	virtual void updateFromProperties();
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
	virtual void specifyInputSpace(QVector<uint>& _s) { _s[0] = m_chunked ? 1 : m_last.count(); }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(0, 0, 255); }

	float m_gain;
	int m_size;
	int m_refreshPeriod;
	bool m_antialiasing;
	int m_detail;
	DECLARE_5_PROPERTIES(Oscilloscope, m_gain, m_size, m_refreshPeriod, m_antialiasing, m_detail);

	QVector<float> m_last;
	bool m_chunked;
};

int Oscilloscope::process()
{
	BufferData d = input(0).readSamples(m_chunked ? 1 : m_last.count());
	d.copyTo(m_last);
	return DidWork;
}

static float rmsOf(QVector<float> const& _d)
{
	if (!_d.count())
		return 0.f;
	float ret = 0.f;
	for (int i = 0; i < _d.count(); i++)
		ret += sqr(_d[i]);
	return sqrt(ret / _d.count());
}

bool Oscilloscope::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	float rms = rmsOf(m_last);
	_p.setRenderHint(QPainter::Antialiasing, m_antialiasing);
	_p.translate(0, floor(_s.height() / 2.f) + 0.5f);
	_p.scale(1, floor(_s.height() / 2.f) * -m_gain);
	_p.setPen(QPen(QColor(0, 0, 0, 48), 0));
	_p.drawLine(QLineF(0, -0.0001f, float(_s.width()), -0.0001f));
	_p.setPen(QPen(QColor(0, 0, 0, 24), 0));
	_p.drawLine(QLineF(0.f, .5f, float(_s.width()), .5f));
	_p.drawLine(QLineF(0.f, -.5f, float(_s.width()), -.5f));
	_p.setPen(QPen(QColor(255, 0, 0, 32), 0));
	_p.drawLine(QLineF(0.f, rms - 0.0001f, float(_s.width()), rms - 0.0001f));
	_p.drawLine(QLineF(0.f, -rms - 0.0001f, float(_s.width()), -rms - 0.0001f));
	_p.setPen(QPen(Qt::black, 0));
	if (isRunning())
		for (int i = m_detail; i < (int)_s.width(); i+=clamp(m_detail, 1, 3))
			_p.drawLine(QLineF(i - m_detail, m_last[(i - m_detail) * m_last.size() / (int)_s.width()], i, m_last[i * m_last.size() / (int)_s.width()]));
	return true;
}

void Oscilloscope::processorStopped()
{
}

bool Oscilloscope::verifyAndSpecifyTypes(const Types& _inTypes, Types&)
{
	if (_inTypes[0].isA<Wave>())
	{
		m_last.resize(m_size);
		m_chunked = false;
	}
	else if (Typed<WaveChunk> in = _inTypes[0])
	{
		m_last.resize(in->length());
		m_chunked = true;
	}
	else
		return false;
	return true;
}

void Oscilloscope::initFromProperties()
{
	setupIO(1, 0);
}

void Oscilloscope::updateFromProperties()
{
	setupVisual(80, 40, m_refreshPeriod, 40, 20, true);
}
PropertiesInfo Oscilloscope::specifyProperties() const
{
	return PropertiesInfo	("Antialiasing", true, "Antialiasing.", true, "a", AVbool)
							("Detail", 2, "Detail.", true, "d", AV("High", "H", 1) AVand("Medium", "M", 2) AVand("Low", "L", 3))
							("Size", 1024, "The size of the window's width (in samples).", false, "#", AVsamples)
							("RefreshPeriod", 30, "The refresh period (in ms).", true, "r", AV(1, 1000, AllowedValue::Log10))
							("Gain", 1.f, "Gain for input signal (x).", true, "x", AVgain);
}

EXPORT_CLASS(Oscilloscope, 0,1,0, Processor);
