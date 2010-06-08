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
#include <QDebug>
#include <Geddei>
#include <CoreTypes>
using namespace Geddei;

#define DISPLAY 1

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Grapher: public CoProcessor
{
public:
	Grapher(): CoProcessor("Grapher"/*, Geddei::In | Geddei::Hetero*/) {}

private:
	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
//	virtual void specifyInputSpace(QVector<uint>& _s) { _s[0] = m_spu; }
	virtual void requireInputSpace(QVector<uint>& _s) { _s[0] = 1; }
	void updateLimits(QSizeF const& _s, float _ppl) const;
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(120, 96, 160); }

	struct ChannelInfo
	{
		uint input;
		mutable float min;
		mutable float delta;
		mutable float inc;
		mutable double latePoint;
		float frequency;
		MultiValue::Config config;
		inline ChannelInfo(uint _i = Undefined, MultiValue::Config _c = MultiValue::Config(false), float _f = 1.f):
				input(_i), min(0.f), delta(1.f), inc(0.1f), frequency(_f), config(_c) {}
		inline ChannelInfo(uint _i, float _m, float _d, float _n, float _f, MultiValue::Config _c):
				input(_i), min(_m), delta(_d), inc(_n), frequency(_f), config(_c) {}
	};

	uint m_labeled;

	mutable QList<ChannelInfo> m_config;

	QVector<uint> m_viewWidthSamples;
	QVector<uint> m_arity;

	mutable double m_latePoint;

	float m_viewWidth;
	float m_gain;
	bool m_antialiasing;
	int m_labeledOR;

	mutable QPixmap m_display;
	mutable QPixmap m_grid;
	mutable QPixmap m_labels;
	typedef QMultiMap<double, QVector<float> > MarkData;
	mutable QVector<MarkData> m_mPoints;
	mutable QVector<QList<QVector<float> > > m_cPoints;
	mutable QFastMutex l_points;
	mutable bool m_rejig;
};

PropertiesInfo Grapher::specifyProperties() const
{
	return PropertiesInfo	("Inputs", 1, "Number of inputs.", false, "n", AV(1, 16))
							("View Width", 5, "The width of the view (seconds).", false)
							("Antialias", false, "Antialiasing of lines.", true, "A", AVbool)
							("Labeled", -1, "The subgraph index for the unit labels. { -1: Default; >= 0 }", true, "g", AV("Auto", "?", -1) AVand(0, 16))
							("Zoom", 1, "The input zoom factor.", true, "x", AV(0.00001, 100000, AllowedValue::Log10));
}

void Grapher::initFromProperties(Properties const& _p)
{
	setupIO(_p["Inputs"].toInt(), 0);
	m_viewWidth = _p["View Width"].toDouble();
	updateFromProperties(_p);
}

void Grapher::updateFromProperties(Properties const& _p)
{
	m_gain = _p["Zoom"].toDouble();
	m_antialiasing = _p["Antialias"].toBool();
	m_labeledOR = _p["Labeled"].toInt();
	m_labeled = (m_labeledOR == -1) ? m_labeled : m_labeledOR;
	m_rejig = true;
}

bool Grapher::verifyAndSpecifyTypes(const Types& _inTypes, Types&)
{
	m_arity.resize(_inTypes.count());
	m_viewWidthSamples.resize(_inTypes.count());

	m_config.clear();

	for (uint i = 0; i < _inTypes.count(); i++)
		if (Typed<MultiValue> in = _inTypes[0])
		{
			m_labeled = (m_labeledOR == -1) ? in->labeled() + m_config.size() : m_labeledOR;
			m_arity[i] = max(1u, in->arity());
			m_viewWidthSamples[i] = (uint)(in->frequency() * m_viewWidth);
			for (int ii = 0; ii < in->config().size(); ii++)
				m_config.append(ChannelInfo(i, in->config()[ii], in->frequency()));
		}
		else if (Typed<Contiguous> in = _inTypes[0])
		{
			m_labeled = (m_labeledOR == -1) ? m_config.size() : m_labeledOR;
			m_arity[i] = max(1u, in->arity());
			m_viewWidthSamples[i] = (uint)(in->frequency() * m_viewWidth);
			for (uint ii = 0; ii < in->arity(); ii++)
				m_config.append(ChannelInfo(i, MultiValue::Config(in->max(), in->min(), ii), in->frequency()));
		}
		else if (Typed<Mark> in = _inTypes[0])
		{
			(void)in;
			m_config.append(ChannelInfo(i));
		}
		else
			return false;

	m_mPoints.resize(_inTypes.size());
	m_cPoints.resize(_inTypes.size());

	for (int i = 0; i < m_config.size(); i++)
	{
		m_config[i].min = m_config[i].config.min * m_config[i].config.conversion;
		m_config[i].inc = m_config[i].delta = (m_config[i].config.max - m_config[i].config.min) * m_config[i].config.conversion;
	}
	setupVisual(m_viewWidth * 20, 20, 20);
	return true;
}

bool Grapher::processorStarted()
{
	m_display = QPixmap();

	for (int i = 0; i < m_mPoints.size(); i++)
	{
		m_mPoints[i].clear();
		m_cPoints[i].clear();
	}

	for (int i = 0; i < m_config.size(); i++)
		m_config[i].latePoint = 0.0;
	m_latePoint = 0.0;

	return true;
}

int Grapher::process()
{
	for (uint i = 0; i < numInputs(); i++)
	{
		BufferData d = input(i).readSamples(0, true);
		for (uint s = 0; s < d.samples(); s++)
		{
			QVector<float> x(m_arity[i]);
			d.copyTo(x);
			l_points.lock();

			if (m_config[i].config.conversion == 0.f)
			{
				m_mPoints[i].insert(Mark::timestamp(d.sample(s)), x);
			}
			else
			{
				m_cPoints[i].append(x);
				while ((uint)m_cPoints[i].size() > m_viewWidthSamples[i] + 1)
					m_cPoints[i].removeFirst();
			}
			l_points.unlock();
		}
	}
	return DidWork;
}

void Grapher::processorStopped()
{
}

void Grapher::updateLimits(QSizeF const& _s, float _ppl) const
{
	for (int i = 0; i < m_config.size(); i++)
	{
		float uMin = m_config[i].config.min * m_config[i].config.conversion / m_gain;
		float uMax = m_config[i].config.max * m_config[i].config.conversion / m_gain;
		float l10 = logf((uMax - uMin) / (_s.height() / _ppl) * 5.5f) / logf(10.f);
		float mt = pow(10.f, l10 - floor(l10));
		float ep = pow(10.f, floor(l10));
		m_config[i].inc = (mt > 6.f) ? ep * 2.f : (mt > 3.f) ? ep : (mt > 1.2f) ? ep / 2.f : ep / 5.f;
		m_config[i].min = floor(uMin / m_config[i].inc) * m_config[i].inc;
		m_config[i].delta = ceil(uMax / m_config[i].inc) * m_config[i].inc - m_config[i].min;
	}
}

// TODO: on plunge reset latepoints.

bool Grapher::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	QVector<float> lp(m_config.size());
	QFont f;
	f.setPixelSize(8.f);

	#define Y(_V, _M, _D) ((1.f - ((_V) - _M) / _D) * _s.height())

	if (m_cPoints.size() && m_mPoints.size())
	{
		QVector<QList<QVector<float> > > cp;
		QVector<MarkData> mp;
		l_points.lock();
		cp = m_cPoints;
		mp = m_mPoints;
		for (int i = 0; i < cp.size(); i++)
		{
			m_cPoints[i].clear();
			if (cp[i].size())
				m_cPoints[i].append(cp[i].last());
			m_mPoints[i].clear();
		}
		l_points.unlock();

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
			for (float yp = m_config[m_labeled].min + m_config[m_labeled].inc; yp < m_config[m_labeled].min + m_config[m_labeled].delta; yp += m_config[m_labeled].inc)
			{
				float mn = m_config[m_labeled].min;
				float dl = m_config[m_labeled].delta;
				q.drawLine(QLineF(0, Y(yp, mn, dl), _s.width(), Y(yp, mn, dl)));
				p.setPen(QColor(255, 255, 255, 64));
				p.drawLine(QLineF(0, Y(yp, mn, dl), _s.width(), Y(yp, mn, dl)));
				QString t = QString::number(yp, 'g', 4);
				QRectF pos(0, Y(yp, mn, dl) - (f.pixelSize() + 2) / 2 + 1, _s.width(), f.pixelSize() + 2);
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

		double newLatePoint = m_latePoint;
		for (int cii = 0; cii < m_config.size(); cii++)
		{
			ChannelInfo& ci = m_config[cii];
			if (ci.config.conversion == 0.f)
				ci.latePoint = mp[ci.input].keys().last();
			else
				ci.latePoint += max<int>(0, cp[ci.input].size() - 1) / ci.frequency;
			newLatePoint = max(newLatePoint, ci.latePoint);
		}

		double pixelOff = ceil((newLatePoint - m_latePoint) / m_viewWidth * _s.width());
		if (pixelOff > 0.f)
		{
			m_latePoint += pixelOff * m_viewWidth / _s.width();
			m_display.scroll(-pixelOff, 0, m_display.rect());

			QPainter p(&m_display);
			p.setRenderHint(QPainter::Antialiasing, false);
			p.setCompositionMode(QPainter::CompositionMode_Source);
			p.fillRect(_s.width() - pixelOff, 0, pixelOff, _s.height(), Qt::transparent);
		}

		QPainter p(&m_display);
		p.setRenderHint(QPainter::Antialiasing, m_antialiasing);
		p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		int cfi = 0;
		foreach (ChannelInfo ci, m_config)
		{
			p.save();
			// basic scaling:
			// basic transform: newLatePoint -
			MultiValue::Config cf = ci.config;
			float mn = ci.min;
			float dl = ci.delta;
			if (cf.conversion == 0.f)
			{
				QMap<double, QVector<float> > const& d = mp[ci.input];
				lp[cfi] = d.value(d.keys().last())[0];
			}
			else if (cp[ci.input].size())
			{
				QList<QVector<float> > const& d = cp[ci.input];

				float lv = d[0][cf.index] * cf.conversion;
				lp[cfi] = lv;

				p.translate(_s.width(), 0.f);
				p.scale(_s.width() / m_viewWidth, 1.f);
				p.translate(ci.latePoint - m_latePoint, 0.f);
				p.scale(1.f / ci.frequency, 1.f);
				p.translate(-(d.size() - 1), 0.f);

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
						p.drawEllipse(QPointF(i - 1, Y(lv, mn, dl)), .5, (float)_s.width() / (float)m_viewWidthSamples[ci.input] / (float)_s.height() / 2);
					}
					else if (isInf(lv) == -1 && !isInf(tv))
					{
						// Transition from none;
						p.setPen(Qt::NoPen);
						p.setBrush(cf.fore);
						p.drawEllipse(QPointF(i, Y(tv, mn, dl)), .5, (float)_s.width() / (float)m_viewWidthSamples[ci.input] / (float)_s.height() / 2);
					}
					else if (!isInf(lv) && !isInf(tv))
					{
						if (cf.back != Qt::transparent)
						{
							p.setPen(Qt::NoPen);
							p.setBrush(cf.back);
							QVector<QPointF> ps(4);
							/*qDebug() << lv << cfi;
							float y = Y(lv, mn, dl);
							qDebug() << y;
							ps[0] = QPointF(i - 1, y);
							y = Y(tv, mn, dl);
							qDebug() << y;
							ps[1] = QPointF(i, y);*/
							ps[0] = QPointF(i - 1, Y(lv, mn, dl));
							ps[1] = QPointF(i, Y(tv, mn, dl));
							ps[2] = QPointF(i, _s.height());
							ps[3] = QPointF(i - 1, _s.height());
							qDebug() << ps;
							p.drawPolygon(ps.data(), 4);
						}
						if (cf.fore != Qt::transparent)
						{
							p.setPen(QPen(cf.fore, 0));
							p.drawLine(QLineF(i - 1, Y(lv, mn, dl), i, Y(tv, mn, dl)));
						}
					}
					lv = tv;
				}
			}
			p.restore();
			cfi++;
		}
	}
	_p.setRenderHint(QPainter::SmoothPixmapTransform, false);
	_p.setRenderHint(QPainter::Antialiasing, false);
	_p.translate(0.5, 0.5);
	_p.drawPixmap(0, 0, m_grid);
	_p.drawPixmap(0, 0, m_display);
	_p.drawPixmap(0, 0, m_labels);
/*	if (m_labeled < (uint)m_config.size() && m_config[m_labeled].config.index < lp.size())
	{
		QString t = QString::number(lp[m_config[m_labeled].index] * m_config[m_labeled].conversion, 'g', 4) + " " + m_config[m_labeled].units;
		_p.setFont(f);
		_p.setPen((m_config[m_labeled].back == Qt::transparent) ? QColor(255, 255, 255, 192) : m_config[m_labeled].back);
		for (int x = -1; x < 2; x++)
			for (int y = -1; y < 2; y++)
				if (x != y && x != 1 - y)
					_p.drawText(QRectF(QPointF(x - 1, y), _s), Qt::AlignHCenter|Qt::AlignTop, t);
		_p.setPen((m_config[m_labeled].fore == Qt::transparent) ? Qt::black : m_config[m_labeled].fore);
		_p.drawText(QRectF(QPointF(0, 0), _s), Qt::AlignHCenter|Qt::AlignTop, t);
	}*/
	return true;
}

EXPORT_CLASS(Grapher, 0,1,0, Processor);
