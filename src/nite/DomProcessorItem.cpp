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

DomProcessorItem::DomProcessorItem(Properties const& _pr, QString const& _name, QSizeF const& _size):
	ProcessorItem(0, _pr, _name, _size)
{
}

DomProcessor* DomProcessorItem::domProcessor() const { return dynamic_cast<DomProcessor*>(processor()); }

QSizeF DomProcessorItem::centreMin() const
{
	QSizeF s(0, 0);
	foreach (SubProcessorItem* i, filter<SubProcessorItem>(childItems()))
		s = QSizeF(s.width() + i->size().width(), max(s.height(), i->size().height()));
	return s;
}

QString DomProcessorItem::composedSubs() const
{
	QString ret;
	foreach (SubProcessorItem* i, ordered())
		ret += "&" + i->spType();
	return ret.mid(1);
}

Properties DomProcessorItem::completeProperties() const
{
	Properties ret;
	QList<SubProcessorItem*> spis = ordered();
	for (int i = (uint)spis.count() - 1; i >= 0; i--)
		ret = ret.stashed() + spis[i]->properties();
	ret = ret.stashed() + properties();
	return ret;
}

Processor* DomProcessorItem::reconstructProcessor()
{
	QString cs = composedSubs();
	if (cs.isEmpty())
		return 0;
	Processor* p = new DomProcessor(cs);
	PropertiesInfo pi = p->properties();
	m_properties.defaultFrom(pi.destash());
	foreach (SubProcessorItem* i, ordered())
		i->m_properties.defaultFrom(pi.destash());
	return p;
}

void DomProcessorItem::reorder() const
{
	QList<SubProcessorItem*> spis = filter<SubProcessorItem>(childItems());

	uint oc = (uint)spis.count();
	for (uint i = 0; i < oc; i++)
	{
		SubProcessorItem* spi;
		for (uint j = i;; j++)
			foreach (spi, spis)
				if (spi->index() <= j)
				{
					spis.removeAll(spi);
					spi->m_index = i;
					goto OK;
				}
		break;
		OK: ;
	}
}

QList<SubProcessorItem*> DomProcessorItem::ordered() const
{
	QList<SubProcessorItem*> ret;
	QList<SubProcessorItem*> spis = filter<SubProcessorItem>(childItems());

	for (uint i = 0; i < (uint)spis.count(); i++)
	{
		SubProcessorItem* spi;
		foreach (spi, spis)
			if (spi->index() == i)
				goto OK;
		break;
		assert("Subprocessors out of order");
		OK:
		ret << spi;
	}
	return ret;
}

void DomProcessorItem::rejig()
{
	QPointF cp = clientArea().topLeft();
	foreach (SubProcessorItem* spi, ordered())
	{
		spi->setPos(cp);
		cp += QPointF(spi->size().width(), 0);
	}
	ProcessorItem::rejig();
}

void DomProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem* _o, QWidget* _w)
{
	QPainterPath p;
	p.addRect(boundingRect());
	foreach (SubProcessorItem* spi, filter<SubProcessorItem>(childItems()))
		p.addRect(QRectF(spi->pos(), spi->size()));
	_p->save();
	_p->setClipPath(p);
	ProcessorItem::paint(_p, _o, _w);
	_p->restore();
}

void DomProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	Properties p;
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "property")
			p[n.toElement().attribute("name")] = n.toElement().attribute("value");
	DomProcessorItem* dpi = new DomProcessorItem(p, _element.attribute("name"), QSizeF(_element.attribute("w").toDouble(), _element.attribute("h").toDouble()));
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "subprocessor")
			SubProcessorItem::fromDom(n.toElement(), dpi);
	dpi->propertiesChanged(_element.attribute("name"));
	_scene->addItem(dpi);
	dpi->setPos(_element.attribute("x").toDouble(), _element.attribute("y").toDouble());
}

QDomElement DomProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc, QString const&) const
{
	QDomElement us = ProcessorItem::saveYourself(_root, _doc, "domprocessor");
	foreach (SubProcessorItem* spi, filter<SubProcessorItem>(childItems()))
		spi->saveYourself(us, _doc);
	return us;
}
