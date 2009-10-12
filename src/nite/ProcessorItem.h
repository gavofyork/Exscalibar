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

#include "InputItem.h"
#include "OutputItem.h"

static const double cornerSize = 4.0;
static const double statusHeight = 8.0;
static const double statusMargin = 2.0;
static const double portSize = 8.0;
static const double portLateralMargin = 4.0;

class PauseItem;

class ProcessorItem: public QObject, public QGraphicsItem
{
	Q_OBJECT

	friend class InputItem;
	friend class OutputItem;

public:
	ProcessorItem(Processor* _p, Properties const& _pr = Properties(), QString const& _name = QString::null, QSizeF const& _size = QSizeF(0, 0));
	~ProcessorItem()
	{
		delete m_processor;
	}

	Processor* processor() const { return m_processor; }
	QGraphicsItem* statusBar() const { return m_statusBar; }
	QRectF clientArea() const;

	void adjustSize(QSizeF const& _a);

	Properties const& properties() const { return m_properties; }
	void setProperty(QString const& _key, QVariant const& _value);

	bool connectYourself(ProcessorGroup& _group);
	void typesConfirmed();
	void disconnectYourself();

	void tick();

	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	virtual QDomElement saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n = "processor") const;

	virtual QSizeF centreMin() const { return QSizeF(m_processor->width(), m_processor->height()); }

	virtual QRectF boundingRect() const
	{
		return QRectF(QPointF(-10, -10), m_size + QSize(20.f, 20.f));
	}
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual bool sceneEvent(QEvent* _e) { return QGraphicsItem::sceneEvent(_e); }
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* _e);

	enum { Type = UserType + 1 };
	virtual int type() const { return Type; }

	virtual void propertiesChanged(QString const& _newName = QString::null);

protected:
	virtual void rejig();
	virtual Processor* reconstructProcessor();
	virtual Properties completeProperties() const { return m_properties; }

	Properties			m_properties;
	Processor*			m_processor;

private:
	virtual void timerEvent(QTimerEvent*);

	QGraphicsRectItem*	m_statusBar;
	PauseItem*			m_pauseItem;
	QSizeF				m_size;
	int					m_timerId;
	bool				m_resizing;
	QPointF				m_origPosition;
	uint				m_multiplicity;
};
