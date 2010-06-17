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

#pragma once

#include "WithProperties.h"

class PropertyItem: public QGraphicsItem
{
public:
	PropertyItem(QGraphicsItem* _p, QRectF const& _r, QString const& _k);

	inline WithProperties* withProperties() const { return dynamic_cast<WithProperties*>(parentItem()); }
	inline virtual QRectF boundingRect() const { return m_rect; }

	inline bool isDynamic() const { return m_isDynamic; }
	inline float bd() const { return m_bd; }
	inline QString const& key() const { return m_key; }
	inline void resize(QRectF const& _n);
	float minWidth() const;
	bool isExpandable() const;

	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

	enum { ItemType = UserType + 16 };
	virtual int type() const { return ItemType; }

protected:
	bool m_isDynamic;
	QString m_key;
	QRectF m_rect;
	float m_bd;
};

class BasePropertyItem: public QGraphicsItem
{
public:
	BasePropertyItem(PropertyItem* _p, QRectF const& _r):
		QGraphicsItem	(_p),
		m_rect			(_r)
	{}

	void					resize(QRectF const& _r);
	virtual PropertyItem*	propertyItem() const { return dynamic_cast<PropertyItem*>(parentItem()); }
	virtual WithProperties*	withProperties() const { return propertyItem()->withProperties(); }
	QRectF					gauge() const { return QRectF(m_rect); }
	virtual QRectF			boundingRect() const { return m_rect; }
	virtual void			paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual void			paintItem(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}
	virtual float			minWidth() const { return 30.f; }
	virtual bool			isExpandable() const { return true; }

	virtual bool			sceneEvent(QEvent* _e);

	enum { ItemType = UserType + 20 };
	virtual int				type() const { return ItemType; }

	QRectF m_rect;
};
