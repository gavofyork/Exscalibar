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

#include "ProcessorsScene.h"
#include "SubsContainer.h"
#include "withproperties.h"
#include "GeddeiNite.h"
#include "SubProcessorItem.h"

SubsContainer* SubProcessorItem::subsContainer() const { return dynamic_cast<SubsContainer*>(parentItem()); }
DomProcessor* SubProcessorItem::domProcessor() const { return subsContainer()->domProcessor(); }

class PropertyItem: public QGraphicsItem
{
public:
	PropertyItem(QGraphicsItem* _p, QRectF const& _r, QString const& _k):
		QGraphicsItem	(_p),
		m_key			(_k),
		m_rect			(_r.left(), _r.top(), _r.width(), _r.height()),
		m_bd			(2)
	{
	}

	virtual WithProperties* withProperties() const { return dynamic_cast<WithProperties*>(parentItem()); }
	virtual QRectF boundingRect() const
	{
		return m_rect;
	}

	QRectF gauge() const { return QRectF(m_rect.adjusted(floor(m_rect.height() * .7f) + m_bd, m_bd, -m_bd, -m_bd)); }

	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
	{
		assert(withProperties());
		_p->translate(.5f, .5f);
		QFont f;
		f.setPixelSize(m_rect.height() - 1);
		f.setBold(true);
		_p->setFont(f);
		_p->setPen(QColor(0, 0, 0, 96));
		_p->drawText(m_rect.translated(0, 1.f), withProperties()->propertiesInfo().symbolOf(m_key), Qt::AlignLeft|Qt::AlignVCenter);
		_p->setPen(Qt::white);
		_p->drawText(m_rect.translated(0, 0.f), withProperties()->propertiesInfo().symbolOf(m_key), Qt::AlignLeft|Qt::AlignVCenter);
	}

	enum { Type = UserType + 16 };
	virtual int type() const { return Type; }

protected:
	QString m_key;
	QRectF m_rect;
	float m_bd;
};

class FloatPropertyItem: public PropertyItem
{
public:
	FloatPropertyItem(QGraphicsItem* _p, QRectF const& _r, QString const& _k):
		PropertyItem(_p, _r, _k)
	{
	}

	virtual void		mousePressEvent(QGraphicsSceneMouseEvent* _e)
	{
		assert(withProperties());
		withProperties()->setProperty(m_key, min(1.f, max<float>(0.f, (_e->pos().x() - gauge().left()) / gauge().width())));
	}
	virtual void		mouseMoveEvent(QGraphicsSceneMouseEvent* _e)
	{
		mousePressEvent(_e);
	}

	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem* _o, QWidget* _w)
	{
		PropertyItem::paint(_p, _o, _w);
		QRectF ga = gauge();
		deepRect(_p, ga, true, QColor::fromHsv(0, 0, 96), true, m_bd, false);
		float v = withProperties()->property(m_key).toFloat();
		QLinearGradient g(ga.topLeft(), ga.bottomLeft());
		g.setColorAt(0, QColor::fromHsvF(0, 0, .9f));
		g.setColorAt(.49f, QColor::fromHsvF(0, 0, .825f));
		g.setColorAt(.51f, QColor::fromHsvF(0, 0, .675f));
		g.setColorAt(1, QColor::fromHsvF(0, 0, .5f));
		QRectF merc = ga.adjusted(0, 0, round((v - 1.f) * ga.width()), 0);
		_p->fillRect(merc, g);
		_p->setPen(QPen(QColor(0, 0, 0, 32), m_bd / 2));
		_p->drawRect(merc.adjusted(m_bd / 4, m_bd / 4, -m_bd / 4, -m_bd / 4));
		if ((1.f - v) * ga.width() > m_bd / 4 && v * ga.width() > m_bd / 4)
		{
			_p->setPen(QPen(QColor(0, 0, 0, 64), m_bd / 2));
			_p->drawLine(QPointF(merc.right() + m_bd / 4, ga.top()), QPointF(merc.right() + m_bd / 4, ga.bottom()));
		}
	}

	enum { Type = UserType + 18 };
	virtual int type() const { return Type; }
};

class IntPropertyItem: public PropertyItem
{
public:
	IntPropertyItem(WithProperties* _p, QRectF const& _r, QString const& _k):
		PropertyItem(_p, _r, _k)
	{
	}

	virtual void		mousePressEvent(QGraphicsSceneMouseEvent* _e)
	{
		assert(withProperties());
		int o = (_e->pos().x() - gauge().left()) / gauge().width() * withProperties()->propertiesInfo(m_key).allowed.count();
		foreach (AllowedValue i, withProperties()->propertiesInfo(m_key).allowed)
			if (!i.to.isNull()) {}
			else if (!o)
			{
				withProperties()->setProperty(m_key, i.from);
				return;
			}
			else
				o--;
	}

	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem* _o, QWidget* _w)
	{
		PropertyItem::paint(_p, _o, _w);

		QRectF ga = gauge();
		_p->setPen(QPen(QColor(0, 0, 0, 32), 1));
		_p->drawRoundedRect(ga.adjusted(-.5f, -.5f, .5f, .5f), 1, 1);

		QStringList syms;

		int j = 0;
		int v = -1;
		foreach (AllowedValue i, withProperties()->propertiesInfo(m_key).allowed)
			if (i.to.isNull())
			{
				syms << i.symbol;
				if (i.from == withProperties()->property(m_key))
					v = j;
				else
					j++;
			}
		if (v == -1)
			return;

		float c = withProperties()->propertiesInfo(m_key).allowed.count();
		float vf = round(ga.width() * v / c);
		float vt = round(ga.width() * (v + 1) / c);

		QRectF merc(ga.left() + vf, ga.top(), vt - vf, ga.height());
		deepRect(_p, merc, true, QColor::fromHsv(0, 0, 96), true, m_bd, false);

		QFont f;
		f.setPixelSize(ga.height() - 1);
		f.setBold(true);
		_p->setFont(f);
		for (int i = 0; i < c; i++)
		{
			float vf = round(ga.width() * i / c);
			float vt = round(ga.width() * (i + 1) / c);
			QRectF vr(ga.left() + vf, ga.top(), vt - vf, ga.height());
			_p->setPen(QColor(0, 0, 0, 96));
			if (i == v)
			{
				_p->drawText(vr.translated(0, 1.f), Qt::AlignCenter, syms[i]);
				_p->setPen(Qt::white);
			}
			_p->drawText(vr.translated(0, 0.f), Qt::AlignCenter, syms[i]);
		}

	}

	enum { Type = UserType + 19 };
	virtual int type() const { return Type; }
};

float widgetMargin = 0.f;
float widgetHeight = 12.f;

SubProcessorItem::SubProcessorItem(SubsContainer* _dpi, QString const& _type, int _index, Properties const& _pr):
	WithProperties	(_dpi->baseItem(), _pr),
	m_type			(_type),
	m_index			(_index)
{
	_dpi->reorder();
	setFlags(ItemClipsToShape | ItemIsFocusable | ItemIsSelectable);
	QRectF widget(widgetMargin, subProcessor()->height() + widgetMargin, size().width() - widgetMargin * 2, widgetHeight);
	foreach (QString k, m_dynamicKeys)
	{
		PropertyItem* item = 0;
		if (propertiesInfo().defaultValue(k).type() == QVariant::Int)
			item = new IntPropertyItem(this, widget, k);
		else if ((int)propertiesInfo().defaultValue(k).type() == (int)QMetaType::Float || propertiesInfo().defaultValue(k).type() == QVariant::Double)
			item = new FloatPropertyItem(this, widget, k);
		else
			qDebug() << "Type is unknown?" << propertiesInfo().defaultValue(k).typeName() << (int)propertiesInfo().defaultValue(k).type();
		if (item)
			widget.translate(0, item->boundingRect().height());
	}
}

SubProcessor* SubProcessorItem::subProcessor() const
{
	QList<SubProcessor*> cs;
	if (!domProcessor())
		return 0;
	SubProcessor* cur = domProcessor()->primary();
	int i = m_index;
	forever
	{
		if (Combination* c = dynamic_cast<Combination*>(cur))
		{
			cs.append(c->y());
			cs.append(c->x());
		}
		else
			if (i)
				i--;
			else
				return cur;
		if (cs.isEmpty())
			return 0;
		cur = cs.takeLast();
	}
}

QSizeF SubProcessorItem::size() const
{
	return QSizeF(max<float>(m_dynamicKeys.count() ? 40.f : 0.f, subProcessor()->width()), subProcessor()->height() + (m_dynamicKeys.count() ? 2 * widgetMargin + widgetHeight * m_dynamicKeys.count() : 0));
}

void SubProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->translate(.5f, .5f);

	QRectF ca = QRectF(QPointF(0, 0), size());
	_p->save();
	_p->translate(round((size().width() - subProcessor()->width()) / 2), 0);
	_p->setClipRect(ca);
	subProcessor()->draw(*_p);
	_p->setClipping(false);
	_p->restore();

	if (isSelected())
	{
		_p->setBrush(Qt::NoBrush);
		for (int i = 1; i < 4; i++)
		{
			_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), i));
			_p->drawRect(ca);
		}
	}
}

void SubProcessorItem::setProperty(QString const& _key, QVariant const& _value)
{
	m_properties[_key] = _value;
	prepareGeometryChange();
	subsContainer()->subPropertiesChanged();
	update();
	dynamic_cast<GeddeiNite*>(scene()->parent())->propertyHasBeenChanged();
}

void SubProcessorItem::focusInEvent(QFocusEvent* _e)
{
	foreach (QGraphicsItem* i, scene()->selectedItems())
		if (i != this)
			i->setSelected(false);
	setSelected(true);
	assert(isSelected());
	update();
	QGraphicsItem::focusInEvent(_e);
}

void SubProcessorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
{
	subsContainer()->baseItem()->forwardEvent(_e);
	subsContainer()->baseItem()->setSelected(false);
	QGraphicsItem::mouseReleaseEvent(_e);
}

void SubProcessorItem::mousePressEvent(QGraphicsSceneMouseEvent* _e)
{
	subsContainer()->baseItem()->forwardEvent(_e);
	subsContainer()->baseItem()->setSelected(false);
	QGraphicsItem::mousePressEvent(_e);
}

void SubProcessorItem::mouseMoveEvent(QGraphicsSceneMouseEvent* _e)
{
	subsContainer()->baseItem()->forwardEvent(_e);
}

void SubProcessorItem::fromDom(QDomElement const& _element, SubsContainer* _dpi)
{
	Properties p;
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "property")
			p[n.toElement().attribute("name")] = n.toElement().attribute("value");
	new SubProcessorItem(_dpi, _element.attribute("type"), _element.attribute("index").toInt(), p);
}

void SubProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement proc = _doc.createElement("subprocessor");
	proc.setAttribute("type", m_type);
	proc.setAttribute("index", m_index);
	foreach (QString k, m_properties.keys())
	{
		QDomElement prop = _doc.createElement("property");
		proc.appendChild(prop);
		prop.setAttribute("name", k);
		prop.setAttribute("value", m_properties[k].toString());
	}
	_root.appendChild(proc);
}
