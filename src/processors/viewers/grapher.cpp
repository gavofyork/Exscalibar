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

#include "value.h"
using namespace SignalTypes;

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Grapher: public CoProcessor
{
	QVector<float> m_points;
	mutable QFastMutex l_points;
	uint m_spu;
	float m_gain;
	float m_viewWidth;
	float m_min;
	float m_delta;
	uint m_viewWidthSamples;

	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
	virtual void specifyInputSpace(QVector<uint>& _s) { _s[0] = m_spu; }
	virtual void requireInputSpace(QVector<uint>& _s) { _s[0] = 1; }

public:
	Grapher(): CoProcessor("Grapher") {}
};

int Grapher::process()
{
	BufferData d = input(0).readSamples(input(0).samplesReady());
	uint os = m_points.size();
	l_points.lock();
	m_points.resize(os + d.elements());
	l_points.unlock();
	d.copyTo(m_points.data() + os);
	return DidWork;
}

bool Grapher::processorStarted()
{
	m_points.clear();
	return true;
}

bool Grapher::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	int mps = min(m_points.size(), (int)m_viewWidthSamples);
	int off = max(0, (int)(m_points.size() - m_viewWidthSamples));
	if (mps > 1)
	{
		_p.setRenderHint(QPainter::Antialiasing, false);
		_p.translate(0, _s.height());
		_p.scale((float)_s.width() / (float)mps, _s.height());
		_p.setPen(QPen(Qt::black, 0));
		l_points.lock();
		for (int i = 1; i < mps; i++)
			_p.drawLine(QLineF(i - 1, (m_points[off + i - 1] * -m_gain - m_min) * m_delta, i, (m_points[off + i] * -m_gain - m_min) * m_delta));
		l_points.unlock();
	}
	return true;
}

void Grapher::processorStopped()
{
}

bool Grapher::verifyAndSpecifyTypes(const SignalTypeRefs& _inTypes, SignalTypeRefs&)
{
	m_viewWidthSamples = (uint)(_inTypes[0].frequency() * m_viewWidth);
	m_min = _inTypes[0].asA<SignalType>().minAmplitude();
	m_delta = _inTypes[0].asA<SignalType>().maxAmplitude() - _inTypes[0].asA<SignalType>().minAmplitude();
	setupVisual(m_viewWidthSamples, 20, 30);
	return _inTypes[0].isA<Value>();
}

void Grapher::initFromProperties(Properties const& _p)
{
	setupIO(1, 0);
	m_spu = _p["Samples/Update"].toInt();
	m_viewWidth = _p["View Width"].toDouble();
	updateFromProperties(_p);
}

void Grapher::updateFromProperties(Properties const& _p)
{
	m_gain = _p["Gain"].toDouble();
}


PropertiesInfo Grapher::specifyProperties() const
{
	return PropertiesInfo	("Samples/Update", 1, "The number of samples to read per graph update.")
							("Gain", 2, "The amount to multiply the incoming value by.")
							("View Width", 5, "The width of the view (seconds).");
}

EXPORT_CLASS(Grapher, 0,1,0, Processor);
