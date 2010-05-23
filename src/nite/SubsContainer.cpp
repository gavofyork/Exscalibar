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
#include "SubProcessorItem.h"
#include "SubsContainer.h"

SubsContainer::SubsContainer()
{
}

QSizeF SubsContainer::centreMin() const
{
	QSizeF s(-margin(), 0);
	foreach (SubProcessorItem* i, subProcessorItems())
		s = QSizeF(s.width() + i->size().width() + margin(), max(s.height(), i->size().height()));
	return s + QSizeF(padding() * 2, padding() * 2);
}

void SubsContainer::paintFrames(QPainter* _p) const
{
	_p->save();
	foreach (SubProcessorItem* spi, ordered())
		deepRect(_p, QRectF(spi->pos(), spi->size()), true, spi->subProcessor()->outlineColour());

	_p->restore();
}

QString SubsContainer::composedSubs() const
{
	QString ret;
	foreach (SubProcessorItem* i, ordered())
		ret += "&" + i->spType();
	return ret.mid(1);
}

Properties SubsContainer::completeProperties() const
{
	Properties ret;
	QList<SubProcessorItem*> spis = ordered();
	for (int i = (uint)spis.count() - 1; i >= 0; i--)
		ret = ret.stashed() + spis[i]->properties();
	ret = ret.stashed() + baseItem()->properties();
	return ret;
}

void SubsContainer::reorder()
{
	QList<SubProcessorItem*> spis = subProcessorItems();

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
	propertiesChanged();
}

QList<SubProcessorItem*> SubsContainer::ordered() const
{
	QList<SubProcessorItem*> ret;
	QList<SubProcessorItem*> spis = subProcessorItems();

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

void SubsContainer::geometryChanged()
{
	QPointF cp(padding(), padding());
	foreach (SubProcessorItem* spi, ordered())
	{
		spi->setPos(cp);
		cp += QPointF(spi->size().width() + margin(), 0);
	}
}

void SubsContainer::importDom(QDomElement& _item, QGraphicsScene*)
{
	for (QDomNode n = _item.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "subprocessor")
			SubProcessorItem::fromDom(n.toElement(), this);
}

void SubsContainer::exportDom(QDomElement& _item, QDomDocument& _doc) const
{
	foreach (SubProcessorItem* spi, subProcessorItems())
		spi->saveYourself(_item, _doc);
}
