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
#include "SubProcessorItem.h"

SubsContainer* SubProcessorItem::subsContainer() const { return dynamic_cast<SubsContainer*>(parentItem()); }
DomProcessor* SubProcessorItem::domProcessor() const { return subsContainer()->domProcessor(); }

SubProcessorItem::SubProcessorItem(SubsContainer* _dpi, QString const& _type, int _index, Properties const& _pr):
	QGraphicsItem	(_dpi->baseItem()),
	m_properties	(_pr),
	m_type			(_type),
	m_index			(_index)
{
	_dpi->reorder();
	setFlags(ItemClipsToShape | ItemIsFocusable | ItemIsSelectable);
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

float widgetMargin = 1.f;
float widgetHeight = 9.f;

QSizeF SubProcessorItem::size() const
{
	QStringList ds;
	foreach (QString k, m_propertiesInfo.keys())
		if (m_propertiesInfo.isDynamic(k))
			ds << k;
	return QSizeF(subProcessor()->width(), subProcessor()->height() + (ds.count() ? 2 * widgetMargin + widgetHeight * ds.count() : 0));
}

void SubProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->translate(.5f, .5f);

	QRectF ca = QRectF(QPointF(0, 0), size());

	_p->save();
	_p->setClipRect(ca);
	subProcessor()->draw(*_p);
	_p->restore();

	QRectF widget(widgetMargin, subProcessor()->height() + widgetMargin, size().width() - widgetMargin * 2, widgetHeight);
	foreach (QString k, m_propertiesInfo.keys())
		if (m_propertiesInfo.isDynamic(k))
		{
			QFont f;
			f.setPixelSize(widgetHeight - 1);
			f.setBold(true);
			_p->setFont(f);
			_p->setPen(QColor(0, 0, 0, 96));
			_p->drawText(widget.translated(0, 2), m_propertiesInfo.symbolOf(k), Qt::AlignLeft|Qt::AlignVCenter);
			_p->setPen(Qt::white);
			_p->drawText(widget.translated(0, 1), m_propertiesInfo.symbolOf(k), Qt::AlignLeft|Qt::AlignVCenter);
			deepRect(_p, widget.adjusted(floor(widgetHeight * .7f), 0, 0, 0), true, QColor::fromHsv(0, 0, 192), false, 1.f, false);
			widget.translate(0, widgetHeight);
		}

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
