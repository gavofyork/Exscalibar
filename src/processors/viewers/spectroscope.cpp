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

#include "qfactoryexporter.h"
#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "spectrum.h"
using namespace SignalTypes;

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Spectroscope: public CoProcessor
{
	QVector<float> m_last;
	float m_minAmp;
	float m_deltaAmp;

	float m_curMin;
	float m_curMax;

	bool m_autoScale;
	float m_dropSpeedMin;
	float m_dropSpeedMax;
	int m_refreshFreq;

	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;

public:
	Spectroscope(): CoProcessor("Spectroscope", NotMulti) {}
};

int Spectroscope::process()
{
	input(0).readSample().copyTo(m_last.data());
	if (m_autoScale)
	{
		float frameMin = INT_MAX;
		float frameMax = INT_MIN;
		foreach (float f, m_last)
			if (!isnan(f) && !isinf(f))
				frameMin = min(f, frameMin), frameMax = max(f, frameMax);
		if (frameMin != (float)INT_MAX && frameMax != (float)INT_MIN)
		{
			m_curMin = lerp(frameMin, m_curMin, m_dropSpeedMin);
			m_curMax = lerp(frameMax, m_curMax, m_dropSpeedMax);
			m_minAmp = m_curMin;
			m_deltaAmp = m_curMax - m_curMin;
		}
	}
	return DidWork;
}

bool Spectroscope::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	_p.setRenderHint(QPainter::Antialiasing, false);
	_p.translate(0, _s.height());
	_p.setPen(QPen(QColor(0, 0, 0, 64), 0));
	_p.drawLine(0, 0, _s.width(), 0);
	if (isRunning())
	{
		_p.scale(_s.width() / m_last.size(), 1);
		_p.translate(0.5, 0);
		_p.setPen(QPen(Qt::black, _s.width() > m_last.size() ? 1 : 0));
		for (int i = 0; i < m_last.size(); i++)
			_p.drawLine(QLineF(i, 0, i, -(m_last[i] - m_minAmp) / max(0.0001f, m_deltaAmp) * _s.height()));
	}
	return true;
}

void Spectroscope::processorStopped()
{
}

bool Spectroscope::verifyAndSpecifyTypes(const SignalTypeRefs& _inTypes, SignalTypeRefs&)
{
	if (_inTypes.count() != 1 || !_inTypes[0].isA<Spectrum>())
		return false;
	m_minAmp = _inTypes[0].asA<Spectrum>().minAmplitude();
	m_deltaAmp = _inTypes[0].asA<Spectrum>().maxAmplitude() - m_minAmp;
	m_curMin = m_minAmp;
	m_curMax = m_minAmp + m_deltaAmp;
	m_last.resize(_inTypes[0].asA<Spectrum>().size());
	setupVisual(m_last.size(), 40, m_refreshFreq);
	return true;
}

void Spectroscope::initFromProperties(Properties const& _p)
{
	m_refreshFreq = 1000 / max(1, _p["Refresh Frequency"].toInt());
	setupIO(1, 0);
}

void Spectroscope::updateFromProperties(Properties const& _p)
{
	m_autoScale = _p["Auto-scale"].toBool();
	m_dropSpeedMin = _p["Drop speed (min)"].toDouble();
	m_dropSpeedMax = _p["Drop speed (max)"].toDouble();
}

PropertiesInfo Spectroscope::specifyProperties() const
{
	return PropertiesInfo("Refresh Frequency", 30, "The number of times to redraw the wave each second (Hz).")
							("Auto-scale", true, "Whether to scale the amplitude according to the incoming data, rather than its type.")
							("Drop speed (max)", 0.01f, "How fast to forget the old scale and rescale to new data.")
							("Drop speed (min)", 0.01f, "How fast to forget the old scale and rescale to new data.");
}

EXPORT_CLASS(Spectroscope, 0,1,0, Processor);


