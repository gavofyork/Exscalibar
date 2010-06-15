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

#include "ProcessorsScene.h"
#include "GeddeiNite.h"
#include "RangePropertyItem.h"
#include "SelectionPropertyItem.h"
#include "PropertyItem.h"

static const float s_widgetHeight = 12.f;

PropertyItem::PropertyItem(QGraphicsItem* _p, QRectF const& _r, QString const& _k):
	QGraphicsItem	(_p),
	m_key			(_k),
	m_bd			(2)
{
	int selectors = 0;
	m_isDynamic = withProperties()->propertiesInfo(m_key).isDynamic;
	foreach (AllowedValue i, withProperties()->propertiesInfo(m_key).allowed)
	{
		BasePropertyItem* it = 0;
		if (i.to.isNull() && !selectors++)
			it = new SelectionPropertyItem(this, _r);
		else if (i.to.isNull())
			continue;
		else
			it = new RangePropertyItem(this, _r, i);
	}
	resize(_r);
	setZValue(10);
}

float PropertyItem::minWidth() const
{
	float ret = s_widgetHeight;
	foreach (BasePropertyItem* i, filterRelaxed<BasePropertyItem>(childItems()))
		ret += i->minWidth() + 2.f;
	return ret;
}

void PropertyItem::resize(QRectF const& _r)
{
	QRectF widget = _r.translated(s_widgetHeight, 2.f);
	widget.setHeight(s_widgetHeight - 4.f);

	float totalMinWidth = 0.f;
	float fixed = 0.f;
	foreach (BasePropertyItem* i, filterRelaxed<BasePropertyItem>(childItems()))
		if (i->isExpandable())
			totalMinWidth += i->minWidth() + 2.f;
		else
			fixed += i->minWidth() + 2.f;
	float scale = totalMinWidth > 0.f ? (_r.width() - s_widgetHeight - fixed) / totalMinWidth : 0.f;
	foreach (BasePropertyItem* i, filterRelaxed<BasePropertyItem>(childItems()))
	{
		float w = i->isExpandable() ? floor(i->minWidth() * scale) : i->minWidth();
		widget.setWidth(w);
		i->resize(widget);
		widget.translate(w + 2.f, 0);
	}
	prepareGeometryChange();
	m_rect = QRectF(_r.left(), _r.top(), widget.left() - _r.left() - 2.f, s_widgetHeight);
}

void PropertyItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	assert(withProperties());
	_p->translate(.5f, .5f);
	QFont f;
	f.setPixelSize(s_widgetHeight - 1);
	f.setBold(true);
	_p->setFont(f);
	QRectF r = m_rect;
	r.setWidth(floor(s_widgetHeight) - 1);
	_p->setPen(QColor(0, 0, 0, 96));
	_p->drawText(r.translated(0, 0.f), Qt::AlignRight, withProperties()->propertiesInfo().symbolOf(m_key));
	_p->setPen(Qt::white);
	_p->drawText(r.translated(0, -1.f), Qt::AlignRight, withProperties()->propertiesInfo().symbolOf(m_key));
}

void BasePropertyItem::resize(QRectF const& _r)
{
	prepareGeometryChange();
	m_rect = _r;
}

bool BasePropertyItem::sceneEvent(QEvent* _e)
{
	if (static_cast<GeddeiNite*>(scene()->parent())->isRunning() && !propertyItem()->isDynamic())
		return true;
	return QGraphicsItem::sceneEvent(_e);
}

void BasePropertyItem::paint(QPainter* _p, const QStyleOptionGraphicsItem* _o, QWidget* _w)
{
	_p->translate(.5f, .5f);
	if (static_cast<GeddeiNite*>(scene()->parent())->isRunning() && !propertyItem()->isDynamic())
		_p->setOpacity(.5f);
	paintItem(_p, _o, _w);
}

