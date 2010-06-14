/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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
	DECLARE_3_PROPERTIES(Oscilloscope, m_gain, m_size, m_refreshPeriod);

	QVector<float> m_last;
	bool m_chunked;
};

int Oscilloscope::process()
{
	BufferData d = input(0).readSamples(m_chunked ? 1 : m_last.count());
	d.copyTo(m_last);
	return DidWork;
}

bool Oscilloscope::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	_p.setRenderHint(QPainter::Antialiasing, false);
	_p.translate(0, _s.height() / 2);
	_p.scale(1, _s.height() / 2 * -m_gain);
	_p.setPen(QPen(QColor(0, 0, 0, 64), 0));
	_p.drawLine(0, 0, _s.width(), 0);
	_p.drawLine(0, .5f, _s.width(), .5f);
	_p.drawLine(0, -.5f, _s.width(), -.5f);
	_p.setPen(QPen(Qt::black, 0));
	if (isRunning())
		for (int i = 2; i < _s.width(); i+=2)
			_p.drawLine(QLineF(i - 2, m_last[(i - 2) * m_last.size() / (int)_s.width()], i, m_last[i * m_last.size() / (int)_s.width()]));
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
	return PropertiesInfo	("Size", 1024, "The size of the window's width (in samples).", false, "#", AVsamples)
							("RefreshPeriod", 30, "The refresh period (in ms).", true, "r", AV(1, 1000, AllowedValue::Log10))
							("Gain", 1.f, "Gain for input signal (x).", true, "x", AVgain);
}

EXPORT_CLASS(Oscilloscope, 0,1,0, Processor);
