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
	virtual void specifyInputSpace(QVector<uint>& _s) { for (int i = 0; i < _s.size(); i++) _s[i] = input(i).type().isA<Mark>() ? 1 : max<uint>(1, input(i).type().asA<Contiguous>().frequency() / 30); }
	virtual void requireInputSpace(QVector<uint>& _s) { for (int i = 0; i < _s.size(); i++) _s[i] = 1; }
	void updateLimits(QSizeF const& _s, float _ppl) const;
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(120, 96, 160); }
	virtual uint cyclesAvailable(QVector<uint> const& _inputs) const { return anyInput(_inputs); }
	virtual void receivedPlunger() { m_latePoint = 0.0; for (int i = 0; i < m_config.size(); i++) m_config[i].throughput = 0; }

	struct ChannelInfo
	{
		uint input;
		mutable float min;
		mutable float delta;
		mutable float inc;
		mutable double latePoint;
		mutable uint64_t throughput;
		float frequency;
		MultiValue::Config config;
		inline ChannelInfo(uint _i = Undefined, MultiValue::Config _c = MultiValue::Config(false), float _f = 1.f):
				input(_i), min(0.f), delta(1.f), inc(0.1f), frequency(_f), config(_c) {}
		inline ChannelInfo(uint _i, float _m, float _d, float _n, float _f, MultiValue::Config _c):
				input(_i), min(_m), delta(_d), inc(_n), frequency(_f), config(_c) {}
	};

	uint m_labeled;

	mutable QList<ChannelInfo> m_config;

	QVector<uint> m_arity;

	mutable double m_latePoint;

	float m_viewWidth;
	float m_gain;
	bool m_antialiasing;
	int m_labeledOR;

	mutable QPixmap m_display;
	mutable QPixmap m_units;
	mutable QPixmap m_time;
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
							("View Width", 5, "The width of the view (seconds).", true, "w", AV(0.1, 100, AllowedValue::Log10))
							("Antialias", false, "Antialiasing of lines.", true, "A", AVbool)
							("Labeled", -1, "The subgraph index for the unit labels. { -1: Default; >= 0 }", true, "g", AV("Auto", "?", -1) AVand(0, 16))
							("Zoom", 1, "The input zoom factor.", true, "x", AV(0.00001, 100000, AllowedValue::Log10));
}

void Grapher::initFromProperties(Properties const& _p)
{
	setupIO(_p["Inputs"].toInt(), 0);
	updateFromProperties(_p);
	setupVisual(100, 50, 20, 50, 20, true);
}

void Grapher::updateFromProperties(Properties const& _p)
{
	m_gain = _p["Zoom"].toDouble();
	m_antialiasing = _p["Antialias"].toBool();
	m_labeledOR = _p["Labeled"].toInt();
	m_labeled = (m_labeledOR == -1) ? m_labeled : m_labeledOR;
	m_viewWidth = _p["View Width"].toDouble();
	m_rejig = true;
}

bool Grapher::verifyAndSpecifyTypes(const Types& _inTypes, Types&)
{
	m_arity.resize(_inTypes.count());

	m_config.clear();

	for (uint i = 0; i < _inTypes.count(); i++)
		if (Typed<MultiValue> in = _inTypes[i])
		{
			m_labeled = (m_labeledOR == -1) ? in->labeled() + m_config.size() : m_labeledOR;
			m_arity[i] = max(1u, in->arity());
			for (int ii = 0; ii < in->config().size(); ii++)
				m_config.append(ChannelInfo(i, in->config()[ii], in->frequency()));
		}
		else if (Typed<Spectrum> in = _inTypes[i])
		{
			m_labeled = (m_labeledOR == -1) ? m_config.size() : m_labeledOR;
			m_arity[i] = max(1u, in->arity());
			m_config.append(ChannelInfo(i, MultiValue::Config(QColor(0, 0, 0, 0x40), Qt::transparent, in->bandFrequency(0), in->bandFrequency(in->bins() - 1), 0), in->frequency()));
		}
		else if (Typed<Contiguous> in = _inTypes[i])
		{
			m_config.append(ChannelInfo(i, MultiValue::Config(in->max(), in->min(), 0), in->frequency()));
			m_arity[i] = in->arity();
		}
		else if (Typed<Mark> in = _inTypes[i])
		{
			m_config.append(ChannelInfo(i, MultiValue::Config(QColor(0, 0, 0, 0x40), Qt::transparent, in->max(0), in->min(0), 0), 0));
			m_arity[i] = in.arity();
		}
		else
			return false;

	assert(m_labeled < (uint)m_config.size());

	m_mPoints.resize(_inTypes.size());
	m_cPoints.resize(_inTypes.size());

	for (int i = 0; i < m_config.size(); i++)
	{
		m_config[i].min = m_config[i].config.min * m_config[i].config.conversion;
		m_config[i].inc = m_config[i].delta = (m_config[i].config.max - m_config[i].config.min) * m_config[i].config.conversion;
	}
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
	receivedPlunger();

	return true;
}

int Grapher::process()
{
	int cycles = 0;
	for (uint i = 0; i < numInputs(); i++)
	{
		BufferData d = input(i).readSamples(0, true);
		l_points.lock();
		for (uint s = 0; s < d.samples(); s++)
		{
			cycles++;
			QVector<float> x(m_arity[i]);
			d.sample(s).copyTo(x);
			if (input(i).type().isA<Mark>())
				m_mPoints[i].insert(Mark::timestamp(d.sample(s)), x);
			else
				m_cPoints[i].append(x);
		}
		if (!input(i).type().isA<Mark>())
			if (int n = max<int>(0, m_cPoints.size() - (input(i).type().asA<Contiguous>().frequency() * m_viewWidth + 1)))
				m_cPoints = m_cPoints.mid(n);
		l_points.unlock();
		d.nullify();
	}
	return cycles;
}

void Grapher::processorStopped()
{
}

void Grapher::updateLimits(QSizeF const& _s, float _ppl) const
{
	for (int i = 0; i < m_config.size(); i++)
		m_config[i].inc = graphParameters<float>(m_config[i].config.min * m_config[i].config.conversion / m_gain, m_config[i].config.max * m_config[i].config.conversion / m_gain, _s.height() / _ppl, &(m_config[i].min), &(m_config[i].delta));
}

void deepRect(QPainter* _p, QRectF _r, bool _down = true, QColor const& _fill = Qt::transparent, bool _rIsInside = true, float _thickness = 2.f, bool _radialFill = true, bool _bright = false);
void deepRect(QPainter* _p, QRectF _r, bool _down, QColor const& _fill, bool _rIsInside, float _thickness, bool _radialFill, bool _bright)
{
	_r.adjust(.5f, .5f, -.5f, -.5f);
	if (!_rIsInside)
		_r.adjust(_thickness, _thickness, -_thickness, -_thickness);
	QLinearGradient g(_r.topLeft(), _r.bottomLeft());
	g.setColorAt(_down ? 0 : 1, Qt::transparent);
	g.setColorAt(_down ? 1 : 0, QColor(255, 255, 255, 64));
	_p->setPen(QPen(QBrush(g), 1));
	_p->drawRoundedRect(_r.adjusted(-_thickness, -_thickness, _thickness, _thickness), _thickness * 1, _thickness * 1);
	_p->setPen(QPen(QColor(0, 0, 0, 128), 1));
	_p->drawRoundedRect(_r.adjusted(-_thickness / 2, -_thickness / 2, _thickness / 2, _thickness / 2), _thickness * .5, _thickness * .5);

	if (_fill != Qt::transparent)
	{
		_r.adjust(-.5f, -.5f, .5f, .5f);
		if (_radialFill)
		{
			QRadialGradient g(_r.center(), max(_r.width(), _r.height()) * 2 / 3, _r.center());
			g.setColorAt(0, _bright ? _fill.lighter(125) : _fill);
			g.setColorAt(1, _bright ? _fill.darker(112) : _fill.darker(150));
			_p->fillRect(_r, g);
		}
		else
		{
			QLinearGradient g(_r.topLeft(), _r.bottomLeft());
			g.setColorAt(0, _bright ? _fill.darker(112) : _fill.darker(150));
			g.setColorAt(1, _bright ? _fill.lighter(125) : _fill);
			_p->fillRect(_r, g);
		}
	}
}

bool Grapher::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	QVector<float> lp(m_config.size());
	QFont f;
	f.setPixelSize(8.f);

	QSizeF ds = _s - QSizeF(4, 4);

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

		if (ds.width() != m_display.width() || ds.height() != m_display.height() || m_rejig)
		{
			updateLimits(ds, f.pixelSize() + 2);
			m_display = QPixmap(ds.width(), ds.height());
			m_display.fill(Qt::transparent);

			m_labels = QPixmap(f.pixelSize() * 5, ds.height());
			m_labels.fill(Qt::transparent);
			QPainter p(&m_labels);
			p.setFont(f);

			m_units = QPixmap(ds.width(), ds.height());
			m_units.fill(Qt::transparent);
			QPainter q(&m_units);
			q.setPen(QColor(0, 0, 0, 0x20));
			for (float yp = m_config[m_labeled].min + m_config[m_labeled].inc; yp < m_config[m_labeled].min + m_config[m_labeled].delta; yp += m_config[m_labeled].inc)
			{
				float mn = m_config[m_labeled].min;
				float dl = m_config[m_labeled].delta;
				q.drawLine(QLineF(0, Y(yp, mn, dl), ds.width(), Y(yp, mn, dl)));
				QString t = QString::number(yp, 'g', 4);
				QRectF pos(0, Y(yp, mn, dl) - (f.pixelSize() + 2) / 2 + 1, ds.width(), f.pixelSize() + 2);
				p.setPen(QColor(255, 255, 255, 192));
				for (float x = -1; x < 2; x++)
					for (float y = -1; y < 2; y++)
						if (x != y && x != -y)
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
			if (ci.frequency == 0.f)
				ci.latePoint = mp[ci.input].size() ? mp[ci.input].keys().last() : 0.0;
			else
				ci.latePoint = double(ci.throughput += max<int>(0, cp[ci.input].size() - 1)) / (double)ci.frequency;
			newLatePoint = max(newLatePoint, ci.latePoint);
		}

		double pixelOff = ceil((newLatePoint - m_latePoint) / m_viewWidth * ds.width());
		if (pixelOff > 0.f)
		{
			m_latePoint += pixelOff * m_viewWidth / ds.width();
			m_display.scroll(-pixelOff, 0, m_display.rect());

			{
				QPainter p(&m_display);
				p.setRenderHint(QPainter::Antialiasing, false);
				p.setCompositionMode(QPainter::CompositionMode_Source);
				p.fillRect(ds.width() - pixelOff, 0, pixelOff, ds.height(), Qt::transparent);
			}
			m_time = QPixmap(ds.width(), ds.height());
			m_time.fill(Qt::transparent);
			QPainter p(&m_time);
			p.setFont(f);
			double from;
			double inc = graphParameters<double>(m_latePoint - m_viewWidth, m_latePoint, (double)ds.width() / 30.0, &from, 0, true);
			for (double i = from; i < m_latePoint; i += inc)
			{
				float x = (i - m_latePoint) * ds.width() / m_viewWidth + ds.width() - 2.f;
				if (x > ds.width() - 20.f || x < 20.f)
					continue;
				p.setPen(QColor(0, 0, 0, 0x10));
				p.drawLine(QPointF(x, 0), QPointF(x, ds.height()));
			}
			inc = graphParameters<double>(m_latePoint - m_viewWidth, m_latePoint, (double)ds.width() / 30.0, &from);
			for (double i = from; i < m_latePoint; i += inc)
			{
				float x = (i - m_latePoint) * ds.width() / m_viewWidth + ds.width() - 2.f;
				p.setPen(QColor(0, 0, 0, 0x20));
				p.drawLine(QPointF(x, 0), QPointF(x, ds.height()));

				if (x > ds.width() - 20.f || x < 20.f)
					continue;

				QString t = QString::number(i, 'g', 3);
				QRectF pos(x - 20.f, 0.f, 40.f, f.pixelSize() + 2);
				p.setPen(QColor(255, 255, 255, 192));
				for (float x = -1; x < 2; x++)
					for (float y = -1; y < 2; y++)
						if (x != y && x != y)
							p.drawText(pos.translated(x, y), Qt::AlignHCenter|Qt::AlignTop, t);
				p.setPen(Qt::black);
				p.drawText(pos, Qt::AlignHCenter|Qt::AlignTop, t);
			}
		}

		QPainter p(&m_display);
		p.setRenderHint(QPainter::Antialiasing, m_antialiasing);
		p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		int cfi = 0;
		foreach (ChannelInfo ci, m_config)
		{
			MultiValue::Config cf = ci.config;
			float mn = ci.min;
			float dl = ci.delta;
			if (!&const_cast<Grapher*>(this)->input(ci.input))
				continue;

			p.save();
			if (ci.frequency == 0.f)
			{
				for (QMap<double, QVector<float> >::iterator i = mp[ci.input].lowerBound(m_latePoint - m_viewWidth); i != mp[ci.input].end(); i++)
				{
					float x = (i.key() - m_latePoint) * ds.width() / m_viewWidth + ds.width() - 2.f;
					if (i.value().size())
						p.fillRect(QRectF(x - 2.f, ds.height(), 4.f, Y(i.value()[0], mn, dl) - ds.height()), QBrush(cf.fore));
					else
						p.fillRect(QRectF(x - 2.f, 0, 4.f, ds.height()), QBrush(cf.fore));
				}
			}
			else if (Typed<Spectrum> in = const_cast<Grapher*>(this)->input(ci.input).type())
			{
				QList<QVector<float> > const& d = cp[ci.input];
				p.translate(ds.width() - 2.f, 0.f);
				p.scale(ds.width() / m_viewWidth, 1.f);
				p.translate(ci.latePoint - m_latePoint, 0.f);
				p.scale(1.f / ci.frequency, 1.f);
				p.translate(-(d.size() - 1), 0.f);
				p.setRenderHint(QPainter::Antialiasing, false);
				for (int i = 1; i < d.size(); i++)
					for (int b = 0; b < (int)in->bins(); b++)
					{
						float l = (d[i][b] - in->min()) / (in->max() - in->min()) * m_gain;
						QColor col;
						if (l < 0.f)
							continue;
						else if (l > 1.f)
							col = Qt::darkRed;
						else
							col = QColor::fromHsv(240 - int(l * 240.f), 255, 255);
						float bf = Y(in->bandFrequency(b-.5f), mn, dl);
						p.fillRect(QRectF(i - 1, bf, 1, Y(in->bandFrequency(b+.5f), mn, dl) - bf), col);
					}
				p.setRenderHint(QPainter::Antialiasing, m_antialiasing);
			}
			else if (cp[ci.input].size())
			{
				QList<QVector<float> > const& d = cp[ci.input];

				float lv = d[0][cf.index] * cf.conversion;
				lp[cfi] = lv;

				p.translate(ds.width() - 2.f, 0.f);
				p.scale(ds.width() / m_viewWidth, 1.f);
				p.translate(ci.latePoint - m_latePoint, 0.f);
				p.scale(1.f / ci.frequency, 1.f);
				p.translate(-(d.size() - 1), 0.f);

				for (int i = 1; i < d.size(); i++)
				{
					float tv = d[i][cf.index] * cf.conversion;
					if (isInf(tv) == 1)
					{
						p.fillRect(QRectF(i - 0.5, 0, 0.5, ds.height()), cf.back);
					}
					else if (isInf(lv) == 1)
					{
						p.fillRect(QRectF(i - 1, 0, 0.5, ds.height()), cf.back);
					}
					else if (isInf(tv) == -1 && !isInf(lv))
					{
						// Transition to none;
						p.setPen(Qt::NoPen);
						p.setBrush(cf.fore);
						p.drawEllipse(QPointF(i - 1, Y(lv, mn, dl)), .5, (float)ds.width() / (ci.frequency * m_viewWidth) / (float)ds.height() / 2);
					}
					else if (isInf(lv) == -1 && !isInf(tv))
					{
						// Transition from none;
						p.setPen(Qt::NoPen);
						p.setBrush(cf.fore);
						p.drawEllipse(QPointF(i, Y(tv, mn, dl)), .5, (float)ds.width() / (ci.frequency * m_viewWidth) / (float)ds.height() / 2);
					}
					else if (!isInf(lv) && !isInf(tv))
					{
						if (cf.back != Qt::transparent)
						{
							p.setPen(Qt::NoPen);
							p.setBrush(cf.back);
							QVector<QPointF> ps(4);
							ps[0] = QPointF(i - 1, Y(lv, mn, dl));
							ps[1] = QPointF(i, Y(tv, mn, dl));
							ps[2] = QPointF(i, ds.height());
							ps[3] = QPointF(i - 1, ds.height());
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
	deepRect(&_p, QRectF(QPointF(0.0, 0.0), _s), true, Qt::white, false, 2, false, true);
	_p.translate(2, 2);
	_p.setRenderHint(QPainter::SmoothPixmapTransform, false);
	_p.setRenderHint(QPainter::Antialiasing, false);
	_p.drawPixmap(0, 0, m_display);
	_p.drawPixmap(0, 0, m_units);
	_p.drawPixmap(0, 0, m_time);
	_p.drawPixmap(0, 0, m_labels);
/*	if (m_labeled < (uint)m_config.size() && m_config[m_labeled].config.index < lp.size())
	{
		QString t = QString::number(lp[m_config[m_labeled].index] * m_config[m_labeled].conversion, 'g', 4) + " " + m_config[m_labeled].units;
		_p.setFont(f);
		_p.setPen((m_config[m_labeled].back == Qt::transparent) ? QColor(255, 255, 255, 192) : m_config[m_labeled].back);
		for (int x = -1; x < 2; x++)
			for (int y = -1; y < 2; y++)
				if (x != y && x != y)
					_p.drawText(QRectF(QPointF(x - 1, y), _s), Qt::AlignHCenter|Qt::AlignTop, t);
		_p.setPen((m_config[m_labeled].fore == Qt::transparent) ? Qt::black : m_config[m_labeled].fore);
		_p.drawText(QRectF(QPointF(0, 0), _s), Qt::AlignHCenter|Qt::AlignTop, t);
	}*/
	return true;
}

EXPORT_CLASS(Grapher, 0,1,0, Processor);
