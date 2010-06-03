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
	mutable QPixmap m_display;
	mutable QPixmap m_grid;
	mutable QPixmap m_labels;
	mutable QList<QVector<float> > m_points;
	mutable QFastMutex l_points;
	mutable bool m_rejig;
	mutable QVector<float> m_mins;
	mutable QVector<float> m_deltas;
	mutable QVector<float> m_incs;

	uint m_units;

	uint m_viewWidthSamples;
	uint m_spu;

	uint m_scope;
	QVector<MultiValue::Config> m_config;

	float m_viewWidth;
	float m_gain;
	bool m_antialiasing;
	int m_labeled;

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
	void updateLimits(QSizeF const& _s, float _ppl) const;
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(120, 96, 160); }
public:
	Grapher(): CoProcessor("Grapher") {}
};

int Grapher::process()
{
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
}

bool Grapher::processorStarted()
{
	m_display = QPixmap();
	m_points.clear();
	return true;
}

void Grapher::updateLimits(QSizeF const& _s, float _ppl) const
{
	for (int i = 0; i < m_config.size(); i++)
	{
		float uMin = m_config[i].min * m_config[i].conversion / m_gain;
		float uMax = m_config[i].max * m_config[i].conversion / m_gain;
		float l10 = logf((uMax - uMin) / (_s.height() / _ppl) * 5.5f) / logf(10.f);
		float mt = pow(10.f, l10 - floor(l10));
		float ep = pow(10.f, floor(l10));
		m_incs[i] = (mt > 6.f) ? ep * 2.f : (mt > 3.f) ? ep : (mt > 1.2f) ? ep / 2.f : ep / 5.f;
		m_mins[i] = floor(uMin / m_incs[i]) * m_incs[i];
		m_deltas[i] = ceil(uMax / m_incs[i]) * m_incs[i] - m_mins[i];
	}
}

bool Grapher::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	QVector<float> lp;
	QFont f;
	f.setPixelSize(8.f);

	#define Y(val, i) ((1.f - ((val) - m_mins[i]) / m_deltas[i]) * _s.height())

	if (m_points.size())
	{
		QList<QVector<float> > d;
		l_points.lock();
		d = m_points;
		m_points.append(d.last());
		m_points.clear();
		l_points.unlock();

		lp = d.last();

		float mps = min(d.size() - 1, (int)m_viewWidthSamples);
		float off = m_viewWidthSamples - mps;

		if (_s.width() != m_display.width() || _s.height() != m_display.height() || m_rejig)
		{
			updateLimits(_s, f.pixelSize() + 2);
			m_display = QPixmap(_s.width(), _s.height());
			m_display.fill(Qt::transparent);

			m_labels = QPixmap(f.pixelSize() * 5, _s.height());
			m_labels.fill(Qt::transparent);
			QPainter p(&m_labels);
			p.setFont(f);

			m_grid = QPixmap(_s.width(), _s.height());
			m_grid.fill(Qt::white);
			QPainter q(&m_grid);
			q.setPen(Qt::lightGray);
			for (float yp = m_mins[m_units] + m_incs[m_units]; yp < m_mins[m_units] + m_deltas[m_units]; yp += m_incs[m_units])
			{
				q.drawLine(QLineF(0, Y(yp, m_units), _s.width(), Y(yp, m_units)));
				p.setPen(QColor(255, 255, 255, 64));
				p.drawLine(QLineF(0, Y(yp, m_units), _s.width(), Y(yp, m_units)));
				QString t = QString::number(yp, 'g', 4);
				QRectF pos(0, Y(yp, m_units) - (f.pixelSize() + 2) / 2 + 1, _s.width(), f.pixelSize() + 2);
				p.setPen(QColor(255, 255, 255, 192));
				for (float x = -1; x < 2; x++)
					for (float y = -1; y < 2; y++)
						if (x != y && x != 1 - y)
							p.drawText(pos.translated(x, y), t);
				p.setPen(Qt::black);
				p.drawText(pos, t);
			}
			m_rejig = false;
		}

		m_display.scroll(-round(mps * _s.width() / m_viewWidthSamples), 0, m_display.rect());
		QPainter p(&m_display);
		p.setRenderHint(QPainter::Antialiasing, false);
		p.setCompositionMode(QPainter::CompositionMode_Source);
		p.fillRect(round(off * _s.width() / m_viewWidthSamples), 0, round(mps * _s.width() / m_viewWidthSamples), _s.height(), Qt::transparent);
		p.setRenderHint(QPainter::Antialiasing, m_antialiasing);
		p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		p.translate(round(off * _s.width() / m_viewWidthSamples), 0);
		p.scale(round(mps * _s.width() / m_viewWidthSamples) / float(d.size() - 1), 1);

		int cfi = 0;
		foreach (MultiValue::Config cf, m_config)
		{
			float lv = d[0][cf.index] * cf.conversion;
			for (int i = 1; i < d.size(); i++)
			{
				float tv = d[i][cf.index] * cf.conversion;
				if (isInf(tv) == 1)
				{
					p.fillRect(QRectF(i - 0.5, 0, 0.5, _s.height()), cf.back);
				}
				else if (isInf(lv) == 1)
				{
					p.fillRect(QRectF(i - 1, 0, 0.5, _s.height()), cf.back);
				}
				else if (isInf(tv) == -1 && !isInf(lv))
				{
					// Transition to none;
					p.setPen(Qt::NoPen);
					p.setBrush(cf.fore);
					p.drawEllipse(QPointF(i - 1, Y(lv, cfi)), .5, (float)_s.width() / (float)m_viewWidthSamples / (float)_s.height() / 2);
				}
				else if (isInf(lv) == -1 && !isInf(tv))
				{
					// Transition from none;
					p.setPen(Qt::NoPen);
					p.setBrush(cf.fore);
					p.drawEllipse(QPointF(i, Y(tv, cfi)), .5, (float)_s.width() / (float)m_viewWidthSamples / (float)_s.height() / 2);
				}
				else if (!isInf(lv) && !isInf(tv))
				{
					if (cf.back != Qt::transparent)
					{
						p.setPen(Qt::NoPen);
						p.setBrush(cf.back);
						QPointF ps[4];
						ps[0] = QPointF(i - 1, Y(lv, cfi));
						ps[1] = QPointF(i, Y(tv, cfi));
						ps[2] = QPointF(i, _s.height());
						ps[3] = QPointF(i - 1, _s.height());
						p.drawPolygon(ps, 4);
					}
					if (cf.fore != Qt::transparent)
					{
						p.setPen(QPen(cf.fore, 0));
						p.drawLine(QLineF(i - 1, Y(lv, cfi), i, Y(tv, cfi)));
					}
				}
				lv = tv;
			}
			cfi++;
		}
	}
	_p.setRenderHint(QPainter::SmoothPixmapTransform, false);
	_p.setRenderHint(QPainter::Antialiasing, false);
	_p.translate(0.5, 0.5);
	_p.drawPixmap(0, 0, m_grid);
	_p.drawPixmap(0, 0, m_display);
	_p.drawPixmap(0, 0, m_labels);
	if (m_units < (uint)m_config.size() && m_config[m_units].index < lp.size())
	{
		QString t = QString::number(lp[m_config[m_units].index] * m_config[m_units].conversion, 'g', 4) + " " + m_config[m_units].units;
		_p.setFont(f);
		_p.setPen((m_config[m_units].back == Qt::transparent) ? QColor(255, 255, 255, 192) : m_config[m_units].back);
		for (int x = -1; x < 2; x++)
			for (int y = -1; y < 2; y++)
				if (x != y && x != 1 - y)
					_p.drawText(QRectF(QPointF(x - 1, y), _s), Qt::AlignHCenter|Qt::AlignTop, t);
		_p.setPen((m_config[m_units].fore == Qt::transparent) ? Qt::black : m_config[m_units].fore);
		_p.drawText(QRectF(QPointF(0, 0), _s), Qt::AlignHCenter|Qt::AlignTop, t);
	}
	return true;
}

void Grapher::processorStopped()
{
}

bool Grapher::verifyAndSpecifyTypes(const SignalTypeRefs& _inTypes, SignalTypeRefs&)
{
	m_viewWidthSamples = (uint)(_inTypes[0].frequency() * m_viewWidth);
	m_scope = max(1u, _inTypes[0].scope());
	if (_inTypes[0].isA<MultiValue>())
	{
		m_config = _inTypes[0].asA<MultiValue>().config();
		m_units = (m_labeled == -1) ? _inTypes[0].asA<MultiValue>().labeled() : m_labeled;
	}
	else
	{
		m_config.resize(m_scope);
		m_units = (m_labeled == -1) ? 0 : m_labeled;
		for (uint i = 0; i < m_scope; i++)
		{
			m_config[i].index = i;
			m_config[i].min = _inTypes[0].asA<SignalType>().minAmplitude();
			m_config[i].max = _inTypes[0].asA<SignalType>().maxAmplitude();
		}
	}
	m_mins.resize(m_config.size());
	m_deltas.resize(m_config.size());
	m_incs.resize(m_config.size());
	for (int i = 0; i < m_config.size(); i++)
	{
		m_mins[i] = m_config[i].min * m_config[i].conversion;
		m_incs[i] = m_deltas[i] = (m_config[i].max - m_config[i].min) * m_config[i].conversion;
	}
	setupVisual(m_viewWidthSamples, 20, m_spu * 1000 / _inTypes[0].frequency());
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
	m_labeled = _p["Labeled"].toInt();
	m_units = (m_labeled == -1) ? m_units : m_labeled;
	m_rejig = true;
}


PropertiesInfo Grapher::specifyProperties() const
{
	return PropertiesInfo	("Samples/Update", 1, "The number of samples to read per graph update.")
							("Antialias", false, "Antialiasing of lines.", true, "A")
							("Labeled", -1, "The subgraph index for the unit labels. { -1: Default; >= 0 }", true)
							("Gain", 1, "The amount to multiply the incoming value by.", true)
							("View Width", 5, "The width of the view (seconds).");
}

EXPORT_CLASS(Grapher, 0,1,0, Processor);
