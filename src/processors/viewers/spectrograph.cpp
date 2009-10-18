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
class Spectrograph: public CoProcessor
{
	mutable QPixmap m_display;
	mutable QList<QVector<float> > m_points;
	mutable QFastMutex l_points;
	float m_gain;
	float m_min;
	float m_delta;
	float m_viewWidth;
	uint m_viewWidthSamples;

	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;

public:
	Spectrograph(): CoProcessor("Spectrograph") {}
};

int Spectrograph::process()
{
	BufferData d = input(0).readSample();
	QVector<float> x;
	x.resize(d.elements());
	d.copyTo(x.data());
	l_points.lock();
	m_points.append(x);
	while ((uint)m_points.size() > m_viewWidthSamples)
		m_points.removeFirst();
	l_points.unlock();
	return DidWork;
}

bool Spectrograph::processorStarted()
{
	m_points.clear();
	return true;
}

bool Spectrograph::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	if (m_points.size())
	{
		QList<QVector<float> > d;
		l_points.lock();
		d = m_points;
		m_points.clear();
		l_points.unlock();

		QPainter p(&m_display);
		p.setRenderHint(QPainter::Antialiasing, false);
		p.setRenderHint(QPainter::SmoothPixmapTransform, false);
		p.setCompositionMode(QPainter::CompositionMode_Source);
		p.drawPixmap(0, 0, m_display, d.size(), 0, m_display.width() - d.size(), m_display.height());
		for (int x = 0; x < d.size(); x++)
			for (int y = 0; y < m_display.height(); y++)
			{
				float l = (d[x][y] - m_min) / m_delta * m_gain;
				if (l < 0.f)
					p.setPen(Qt::black);
				else if (l > 1.f)
					p.setPen(Qt::white);
				else
					p.setPen(QColor::fromHsv(240 - int(l * 240.f), 255, 255));
//				p.setPen(QColor::fromHsv((y == m_display.height() - 1) ? 300 : (int)(240.f * y / m_display.height()), 255, 255));
				p.drawPoint(m_display.width() - d.size() + x, y);
			}
	}
	_p.setRenderHint(QPainter::SmoothPixmapTransform, false);
	_p.setRenderHint(QPainter::Antialiasing, false);
	_p.translate(0.5, 0.5);
	_p.scale(_s.width() / (float)m_display.width(), _s.height() / (float)m_display.height());
	_p.drawPixmap(0, 0, m_display);
	return true;
}

void Spectrograph::processorStopped()
{
}

bool Spectrograph::verifyAndSpecifyTypes(const SignalTypeRefs& _inTypes, SignalTypeRefs&)
{
	m_viewWidthSamples = (uint)(_inTypes[0].frequency() * m_viewWidth);
	m_display = QPixmap(m_viewWidthSamples, _inTypes[0].scope());
	m_display.fill(Qt::white);
	setupVisual(m_display.width(), m_display.height(), 30);
	m_min = _inTypes[0].asA<SignalType>().minAmplitude();
	m_delta = _inTypes[0].asA<SignalType>().maxAmplitude() - _inTypes[0].asA<SignalType>().minAmplitude();
	return _inTypes[0].isA<Spectrum>();
}

void Spectrograph::initFromProperties(Properties const& _p)
{
	setupIO(1, 0);
	m_viewWidth = _p["View Width"].toDouble();
	updateFromProperties(_p);
}

void Spectrograph::updateFromProperties(Properties const& _p)
{
	m_gain = _p["Gain"].toDouble();
}


PropertiesInfo Spectrograph::specifyProperties() const
{
	return PropertiesInfo	("Gain", 2, "The amount to multiply the incoming value by.")
							("View Width", 5, "The width of the view (seconds).");
}

EXPORT_CLASS(Spectrograph, 0,1,0, Processor);
