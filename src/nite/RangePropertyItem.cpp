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

#include <Geddei>
using namespace Geddei;

#include "withproperties.h"
#include "RangePropertyItem.h"

void RangePropertyItem::mousePressEvent(QGraphicsSceneMouseEvent* _e)
{
	assert(withProperties());
	double va = min(1.0, max<double>(0.0, (_e->pos().x() - gauge().left()) / gauge().width()));
	double f;

	if (m_a.scale == AllowedValue::Linear)
		f = va * (m_a.to.toDouble() - m_a.from.toDouble());
	else if (m_a.scale == AllowedValue::Log2)
		f = va * (log2(m_a.to.toDouble()) - log2(m_a.from.toDouble()));
	else if (m_a.scale == AllowedValue::Log10)
		f = va * (log10(m_a.to.toDouble()) - log10(m_a.from.toDouble()));

	if (m_a.from.type() == QVariant::Int || m_a.from.type() == QVariant::UInt)
		f = round(f);
	//if (m_a.stepping)
	//	f = quantised(f, stepping);

	if (m_a.scale == AllowedValue::Linear)
		f += m_a.from.toDouble();
	else if (m_a.scale == AllowedValue::Log2)
		f = pow(2.0, f) * m_a.from.toDouble();
	else if (m_a.scale == AllowedValue::Log10)
		f = pow(10.0, f) * m_a.from.toDouble();

	withProperties()->setProperty(propertyItem()->key(), f);
}

void RangePropertyItem::paintItem(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	QRectF ga = gauge();
	float bd = propertyItem()->bd();
	deepRect(_p, ga, true, QColor::fromHsv(0, 0, 96), true, bd, false);

	double ov = withProperties()->property(propertyItem()->key()).toDouble();

	double v;

	if (m_a.scale == AllowedValue::Linear)
		v = (ov - m_a.from.toDouble()) / (m_a.to.toDouble() - m_a.from.toDouble());
	else if (m_a.scale == AllowedValue::Log2)
		v = log2(ov / m_a.from.toDouble()) / log2(m_a.to.toDouble() / m_a.from.toDouble());
	else if (m_a.scale == AllowedValue::Log10)
		v = log10(ov / m_a.from.toDouble()) / log10(m_a.to.toDouble() / m_a.from.toDouble());

	QLinearGradient g(ga.topLeft(), ga.bottomLeft());
	g.setColorAt(0, QColor::fromHsvF(0, 0, .9f));
	g.setColorAt(.49f, QColor::fromHsvF(0, 0, .825f));
	g.setColorAt(.51f, QColor::fromHsvF(0, 0, .675f));
	g.setColorAt(1, QColor::fromHsvF(0, 0, .5f));
	QRectF merc = ga.adjusted(0, 0, round((v - 1.f) * ga.width()), 0);
	_p->fillRect(merc, g);
	_p->setPen(QPen(QColor(0, 0, 0, 32), bd / 2));
	_p->drawRect(merc.adjusted(bd / 4, bd / 4, -bd / 4, -bd / 4));
	if ((1.f - v) * ga.width() > bd / 4 && v * ga.width() > bd / 4)
	{
		_p->setPen(QPen(QColor(0, 0, 0, 64), bd / 2));
		_p->drawLine(QPointF(merc.right() + bd / 4, ga.top()), QPointF(merc.right() + bd / 4, ga.bottom()));
	}
}
