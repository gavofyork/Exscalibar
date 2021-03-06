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

#include "qfactoryexporter.h"
#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "spectrum.h"
using namespace Geddei;

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Spectroscope: public CoProcessor
{
public:
	Spectroscope(): CoProcessor("Spectroscope") {}

private:
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(0, 0, 255); }
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual int process();

	bool m_autoScale;
	float m_dropSpeedMin;
	float m_dropSpeedMax;
	int m_refreshPeriod;
	DECLARE_4_PROPERTIES(Spectroscope, m_autoScale, m_dropSpeedMin, m_dropSpeedMax, m_refreshPeriod);

	QVector<float> m_last;
	float m_minAmp;
	float m_deltaAmp;

	float m_curMin;
	float m_curMax;
};

PropertiesInfo Spectroscope::specifyProperties() const
{
	return PropertiesInfo
				("RefreshPeriod", 30, "The refresh period (in ms).", false, "r", AV(1, 1000, AllowedValue::Log10))
				("AutoScale", true, "Whether to scale the amplitude according to the incoming data, rather than its type.", true, "s", AVbool)
				("DropSpeedMax", 0.01f, "How fast to forget the old scale and rescale to new data.", true, "x", AVlogUnity)
				("DropSpeedMin", 0.01f, "How fast to forget the old scale and rescale to new data.", true, "n", AVlogUnity);
}

void Spectroscope::initFromProperties()
{
	setupIO(1, 0);
	setupVisual(40, 20, m_refreshPeriod, 40, 20, true);
}

bool Spectroscope::verifyAndSpecifyTypes(const Types& _inTypes, Types&)
{
	if (_inTypes.count() != 1 || !_inTypes[0].isA<Spectrum>())
		return false;
	m_minAmp = _inTypes[0].asA<Spectrum>().min();
	m_deltaAmp = _inTypes[0].asA<Spectrum>().max() - m_minAmp;
	m_curMin = m_minAmp;
	m_curMax = m_minAmp + m_deltaAmp;
	m_last.resize(_inTypes[0].asA<Spectrum>().size());
	return true;
}

int Spectroscope::process()
{
	input(0).readSample().copyTo(m_last);
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

EXPORT_CLASS(Spectroscope, 0,1,0, Processor);


