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

#pragma once

#include <QtXml>
#include <QtGui>
#include <Geddei>

#include "Magnetic.h"
#include "InputItem.h"
#include "OutputItem.h"

class PauseItem;

class BaseItem: public QObject, public QGraphicsItem, public Magnetic
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

public:
	BaseItem(Properties const& _pr = Properties(), QSizeF const& _size = QSizeF());
	~BaseItem();

	enum { Type = UserType };
	virtual int			type() const { return Type; }

	void				setPos(qreal _x, qreal _y) { QGraphicsItem::setPos(floor(_x) + .5f, floor(_y) + .5f); }
	void				setPos(QPointF _p) { QGraphicsItem::setPos(floor(_p.x()) + .5f, floor(_p.y()) + .5f); }

	virtual QTask*		primaryTask() const { return 0; }

	Properties const&	properties() const { return m_properties; }
	void				setProperty(QString const& _key, QVariant const& _value);
	void				setDefaultProperties(PropertiesInfo const& _def);	// Doesn't call propertiesChanged.

	virtual QString		typeName() const { return QString::null; }
	virtual QString		name() const { return QString::null; }
	void				setName(QString const& _name) { propertiesChanged(_name); }

	virtual QDomElement	saveYourself(QDomElement&, QDomDocument&) const = 0;

	virtual void		prepYourself(ProcessorGroup&);
	virtual bool		connectYourself();
	virtual void		disconnectYourself();
	virtual void		typesConfirmed();

	virtual void		tick();

	virtual bool		isRunning() const { return false; }
	virtual float		cyclePoint() const { return 0; }
	virtual void		togglePause() {}

	virtual float		marginSize() const;

	QRectF				centreRect() const { return QRectF(QPointF(0, 0), m_size); }
	virtual QRectF		outlineRect() const;
	virtual QRectF		boundingRect() const;
	inline QRectF		basicBoundingRect() const { float ms = marginSize(); return outlineRect().adjusted(-ms, -ms, ms, ms); }
	virtual QRectF		adjustBounds(QRectF const& _wouldBe) const { return _wouldBe; }

	void				forwardEvent(QGraphicsSceneEvent* _e);

	QBrush				fillBrush() const;
	QPen				outerPen() const;
	QPen				innerPen() const;

protected:
	virtual Properties	completeProperties() const { return m_properties; }
	virtual void		propertiesChanged(QString const& = QString::null);	// Calls...
	virtual void		geometryChanged();									// ...which in turn calls...
	virtual void		positionChanged() {}

	virtual QList<QPointF> magnetism(BaseItem const* _b, bool _moving) const;

	virtual QSizeF		centreMin() const { return QSizeF(0.f, 0.f); }
	virtual QSizeF		centrePref() const { return QSizeF(32.f, 32.f); }
	virtual void		paintOutline(QPainter* _p);
	virtual QColor		highlightColour() const { return QColor::fromHsv(220, 220, 200); }
	virtual QColor		outlineColour() const { return QColor::fromHsv(0, 220, 200); }
	virtual void		paintCentre(QPainter* _p);
	virtual uint		redrawPeriod() const { return 0; }

	void				importDom(QDomElement& _item, QGraphicsScene* _scene);
	void				exportDom(QDomElement& _item, QDomDocument& _doc) const;

private:
	virtual void		paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual void		focusInEvent(QFocusEvent* _e);
	virtual void		mouseMoveEvent(QGraphicsSceneMouseEvent* _e);
	virtual void		mousePressEvent(QGraphicsSceneMouseEvent* _e);
	virtual void		mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);
	virtual void		hoverMoveEvent(QGraphicsSceneHoverEvent* _e);
	virtual void		timerEvent(QTimerEvent*);

	QRectF				resizeRect() const;

	QGraphicsRectItem*	m_statusBar;
	PauseItem*			m_pauseItem;

	QSizeF				m_size;
	int					m_timerId;
	bool				m_resizing;
	QPointF				m_origPosition;
	Properties			m_properties;
};
