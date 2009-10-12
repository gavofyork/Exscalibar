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

#include "SubProcessorItem.h"
#include "DomProcessorItem.h"

DomProcessorItem* SubProcessorItem::domProcessorItem() const { return qgraphicsitem_cast<DomProcessorItem*>(parentItem()); }
DomProcessor* SubProcessorItem::domProcessor() const { return domProcessorItem()->domProcessor(); }

SubProcessorItem::SubProcessorItem(DomProcessorItem* _dpi, QString const& _type, int _index, Properties const& _pr):
	QGraphicsItem	(_dpi),
	m_properties	(_pr),
	m_type			(_type),
	m_index			(_index)
{
	_dpi->reorder();
	_dpi->propertiesChanged();
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

void SubProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	QRectF ca = QRectF(QPointF(0, 0), size());
	_p->save();
	_p->setClipRect(ca);
	subProcessor()->draw(*_p);
	_p->restore();

	if (isSelected())
	{
		_p->setBrush(Qt::NoBrush);
		for (int i = 0; i < 5; i+=2)
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
	domProcessorItem()->propertiesChanged();
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
	domProcessorItem()->mouseReleaseEvent(_e);
	domProcessorItem()->setSelected(false);
	QGraphicsItem::mouseReleaseEvent(_e);
}

void SubProcessorItem::mousePressEvent(QGraphicsSceneMouseEvent* _e)
{
	domProcessorItem()->mousePressEvent(_e);
	domProcessorItem()->setSelected(false);
	QGraphicsItem::mousePressEvent(_e);
}

void SubProcessorItem::mouseMoveEvent(QGraphicsSceneMouseEvent* _e)
{
	domProcessorItem()->mouseMoveEvent(_e);
}

void SubProcessorItem::fromDom(QDomElement const& _element, DomProcessorItem* _dpi)
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
