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

#include "WithProperties.h"
#include "SelectionPropertyItem.h"

void SelectionPropertyItem::mousePressEvent(QGraphicsSceneMouseEvent* _e)
{
	assert(withProperties());
	int o = (_e->pos().x() - gauge().left()) / gauge().width() * withProperties()->propertiesInfo(propertyItem()->key()).allowed.count();
	foreach (AllowedValue i, withProperties()->propertiesInfo(propertyItem()->key()).allowed)
		if (!i.to.isNull()) {}
		else if (!o)
		{
			withProperties()->setProperty(propertyItem()->key(), i.from);
			return;
		}
		else
			o--;
}

void SelectionPropertyItem::paintItem(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	QRectF ga = gauge();
	float bd = propertyItem()->bd();
	_p->setPen(QPen(QColor(0, 0, 0, 32), 1));
	_p->drawRoundedRect(ga.adjusted(-.5f, -.5f, 0.5f, 0.5f), 1, 1);

	QStringList syms;

	int j = 0;
	int v = -1;
	foreach (AllowedValue i, withProperties()->propertiesInfo(propertyItem()->key()).allowed)
		if (i.to.isNull())
		{
			syms << i.symbol;
			if (i.from == withProperties()->property(propertyItem()->key()))
				v = j;
			else
				j++;
		}

	float c = withProperties()->propertiesInfo(propertyItem()->key()).allowed.count();

	if (v != -1)
	{
		float vf = round(ga.width() * v / c);
		float vt = round(ga.width() * (v + 1) / c);

		QRectF merc(ga.left() + vf, ga.top(), vt - vf, ga.height());
		deepRect(_p, merc, true, QColor::fromHsv(0, 0, 96), true, bd, false);
	}

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
