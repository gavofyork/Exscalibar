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

#define DISPLAY 1

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Grapher: public CoProcessor
{
#ifdef DISPLAY
	mutable QPixmap m_display;
	mutable QPixmap m_grid;
	mutable QPixmap m_labels;
	mutable QList<QVector<float> > m_points;
#else
	QVector<float> m_points;
#endif
	mutable QFastMutex l_points;
	uint m_spu;
	float m_gain;
	float m_viewWidth;
	float m_min;
	float m_delta;
	uint m_scope;
	uint m_viewWidthSamples;
	bool m_antialiasing;

	int m_maxLines;

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
#ifdef DISPLAY
	BufferData d = input(0).readSample();
	QVector<float> x;
	x.resize(d.elements());
	d.copyTo(x.data());
	l_points.lock();
	m_points.append(x);
	while ((uint)m_points.size() > m_viewWidthSamples + 1)
		m_points.removeFirst();
	l_points.unlock();
	return DidWork;
#else
	BufferData d = input(0).readSamples(input(0).samplesReady());
	uint os = m_points.size();
	l_points.lock();
	m_points.resize(os + d.elements());
	d.copyTo(m_points.data() + os);
	l_points.unlock();
	return DidWork;
#endif
}

bool Grapher::processorStarted()
{
	m_display = QPixmap();
	m_points.clear();
	return true;
}

static const Qt::GlobalColor s_colours[] = {
	Qt::black,
	Qt::darkRed,
	Qt::darkGreen,
	Qt::darkBlue,
	Qt::darkCyan,
	Qt::darkMagenta,
	Qt::darkYellow,
	Qt::darkGray,
	Qt::red,
	Qt::green,
	Qt::blue,
	Qt::cyan,
	Qt::magenta,
	Qt::yellow,
	Qt::gray
};

bool Grapher::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
#ifdef DISPLAY
	if (m_points.size())
	{
		QList<QVector<float> > d;
		l_points.lock();
		d = m_points;
		m_points.clear();
		m_points.append(d.last());
		l_points.unlock();

		float mps = min(d.size() - 1, (int)m_viewWidthSamples);
		float off = m_viewWidthSamples - mps;
		// TODO: ASSERT off >= 0

		if (_s.width() != m_display.width() || _s.height() != m_display.height())
		{
			QFont f;
			f.setPixelSize(8.f);

			m_display = QPixmap(_s.width(), _s.height());
			m_display.fill(Qt::transparent);

			float ml = _s.height() / (f.pixelSize() + 2);
			float ep;
			float mt = pow(10.f, modff(logf(m_delta / ml * 5.5f) / logf(10.f), &ep));
			ep = pow(10.f, ep);
			float gd = (mt > 6.f) ? ep * 2.f : (mt > 3.f) ? ep : (mt > 1.2f) ? ep / 2.f : ep / 5.f;
			float go = ceil(min(m_min, m_min + m_delta) / gd) * gd;

			{
				m_grid = QPixmap(_s.width(), _s.height());
				m_grid.fill(Qt::white);
				QPainter p(&m_grid);
				p.setPen(Qt::lightGray);
				for (float f = go; f < m_min + m_delta; f += gd)
					p.drawLine(QLineF(0, (1.f - (f * m_gain - m_min) / m_delta) * _s.height(), _s.width(), (1.f - (f * m_gain - m_min) / m_delta) * _s.height()));
			}
			{
				m_labels = QPixmap(f.pixelSize() * 50, _s.height());
				m_labels.fill(Qt::transparent);
				QPainter p(&m_labels);
				p.setFont(f);
				for (float f = go; f < m_min + m_delta; f += gd)
				{
					QRectF pos(0, (1.f - (f * m_gain - m_min) / m_delta) * _s.height(), _s.width(), gd * m_gain / m_delta * _s.height());
					p.setPen(QColor(255, 255, 255, 192));
					for (float x = 0; x < 3; x += 2)
						for (float y = 0; y < 3; y += 2)
							p.drawText(pos.translated(x, y), QString::number(f));
					p.setPen(Qt::black);
					p.drawText(pos.translated(1, 1), QString::number(f));
				}
			}
		}

		m_display.scroll(-round(mps * _s.width() / m_viewWidthSamples), 0, m_display.rect());
		QPainter p(&m_display);
		p.setRenderHint(QPainter::Antialiasing, false);
		p.setCompositionMode(QPainter::CompositionMode_Source);
		p.fillRect(round(off * _s.width() / m_viewWidthSamples), 0, round(mps * _s.width() / m_viewWidthSamples), _s.height(), Qt::transparent);
		p.setRenderHint(QPainter::Antialiasing, m_antialiasing);
		p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		p.translate(round(off * _s.width() / m_viewWidthSamples), _s.height());
		p.scale(round(mps * _s.width() / m_viewWidthSamples) / float(d.size() - 1), _s.height());

		struct Config { QColor fore; QColor back; };
		Config m_config[m_scope];
		for (uint c = 0; c < m_scope; c++)
		{
			m_config[c].back = (c == 1) ? s_colours[c % 15] : Qt::transparent;
			if (c == 1) m_config[c].back.setAlphaF(0.1);
			m_config[c].fore = s_colours[c % 15];

			float lv = d[0][c];
			for (int i = 1; i < d.size(); i++)
			{
				float tv = d[i][c];
				if (isInf(tv) == 1)
				{
					p.fillRect(QRectF(i - 0.5, -1, 0.5, 1), m_config[c].back);
				}
				else if (isInf(lv) == 1)
				{
					p.fillRect(QRectF(i - 1, -1, 0.5, 1), m_config[c].back);
				}
				else if (isInf(tv) == -1 && !isInf(lv))
				{
					// Transition to none;
					p.setPen(Qt::NoPen);
					p.setBrush(m_config[c].fore);
					p.drawEllipse(QPointF(i - 1, (lv * -m_gain - m_min) / m_delta), .5, (float)_s.width() / (float)m_viewWidthSamples / (float)_s.height() / 2);
				}
				else if (isInf(lv) == -1 && !isInf(tv))
				{
					// Transition from none;
					p.setPen(Qt::NoPen);
					p.setBrush(m_config[c].fore);
					p.drawEllipse(QPointF(i, (tv * -m_gain - m_min) / m_delta), .5, (float)_s.width() / (float)m_viewWidthSamples / (float)_s.height() / 2);
				}
				else if (!isInf(lv) && !isInf(tv))
				{
					if (m_config[c].fore != Qt::transparent)
					{
						p.setPen(QPen(m_config[c].fore, 0));
						p.drawLine(QLineF(i - 1, (lv * -m_gain - m_min) / m_delta, i, (tv * -m_gain - m_min) / m_delta));
					}
					if (m_config[c].back != Qt::transparent)
					{
						p.setPen(Qt::NoPen);
						p.setBrush(m_config[c].back);
						QPointF ps[4];
						ps[0] = QPointF(i - 1, (lv * -m_gain - m_min) / m_delta);
						ps[1] = QPointF(i, (tv * -m_gain - m_min) / m_delta);
						ps[2] = QPointF(i, 0);
						ps[3] = QPointF(i - 1, 0);
						p.drawPolygon(ps, 4);
					}
				}
				lv = tv;
			}
		}
	}
	_p.setRenderHint(QPainter::SmoothPixmapTransform, false);
	_p.setRenderHint(QPainter::Antialiasing, false);
	_p.translate(0.5, 0.5);
	_p.drawPixmap(0, 0, m_grid);
	_p.drawPixmap(0, 0, m_display);
	_p.drawPixmap(0, 0, m_labels);
	return true;
#else
	l_points.lock();
	int mps = min((int)(m_points.size() / m_scope), (int)m_viewWidthSamples);
	int off = max(0, (int)(m_points.size() / m_scope - m_viewWidthSamples));
	l_points.unlock();
	if (mps > 1)
	{
		_p.setRenderHint(QPainter::Antialiasing, m_antialiasing);
		_p.translate(0, _s.height());
		_p.scale((float)_s.width() / (float)mps, _s.height());
		for (uint c = 0; c < m_scope; c++)
		{
			_p.setPen(QPen(s_colours[c % 15], 0));
			QColor col(s_colours[c%15]);
			col.setAlphaF(0.1);
			l_points.lock();
			for (int i = 1; i < mps; i++)
				if (isInf(m_points[(off + i) * m_scope + c]) == 1)
					_p.fillRect(QRectF(i - .5, -1, 1, 1), QBrush(col));
//					_p.drawLine(QLineF(i - 1, -1, i, -1));
//					_p.drawEllipse(i - 1, -_s.height(), 1, (float)_s.width() / (float)mps / (float)_s.height());
				else if (isInf(m_points[(off + i) * m_scope + c]) == -1 && !isInf(m_points[(off + i - 1) * m_scope + c]))
				{
					// Transition to none;
					_p.setBrush(_p.pen().color());
					_p.drawEllipse(QPointF(i - 1, (m_points[(off + i - 1) * m_scope + c] * -m_gain - m_min) / m_delta), .5, (float)_s.width() / (float)mps / (float)_s.height() / 2);
				}
				else if (isInf(m_points[(off + i - 1) * m_scope + c]) == -1 && !isInf(m_points[(off + i) * m_scope + c]))
				{
					// Transition from none;
					_p.setBrush(_p.pen().color());
					_p.drawEllipse(QPointF(i, (m_points[(off + i) * m_scope + c] * -m_gain - m_min) / m_delta), .5, (float)_s.width() / (float)mps / (float)_s.height() / 2);
				}
				else if (!isInf(m_points[(off + i - 1) * m_scope + c]) && !isInf(m_points[(off + i) * m_scope + c]))
					_p.drawLine(QLineF(i - 1, (m_points[(off + i - 1) * m_scope + c] * -m_gain - m_min) / m_delta, i, (m_points[(off + i) * m_scope + c] * -m_gain - m_min) / m_delta));
			l_points.unlock();
		}
	}
	return true;
#endif
}

void Grapher::processorStopped()
{
}

bool Grapher::verifyAndSpecifyTypes(const SignalTypeRefs& _inTypes, SignalTypeRefs&)
{
	m_viewWidthSamples = (uint)(_inTypes[0].frequency() * m_viewWidth);
	m_min = _inTypes[0].asA<SignalType>().minAmplitude();
	m_delta = max(0.00000001f, _inTypes[0].asA<SignalType>().maxAmplitude() - _inTypes[0].asA<SignalType>().minAmplitude());

	setupVisual(m_viewWidthSamples, 20, 30);
	m_scope = max(1u, _inTypes[0].scope());
	return true;
}

void Grapher::initFromProperties(Properties const& _p)
{
	setupIO(1, 0);
	m_scope = 1;
	m_spu = _p["Samples/Update"].toInt();
	m_viewWidth = _p["View Width"].toDouble();
	updateFromProperties(_p);
}

void Grapher::updateFromProperties(Properties const& _p)
{
	m_gain = _p["Gain"].toDouble();
	m_antialiasing = _p["Antialias"].toBool();
}


PropertiesInfo Grapher::specifyProperties() const
{
	return PropertiesInfo	("Samples/Update", 1, "The number of samples to read per graph update.")
							("Antialias", false, "Antialiasing of lines.")
							("Gain", 2, "The amount to multiply the incoming value by.")
							("View Width", 5, "The width of the view (seconds).");
}

EXPORT_CLASS(Grapher, 0,1,0, Processor);
