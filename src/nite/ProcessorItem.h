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

#include "BaseItem.h"
#include "InputItem.h"
#include "OutputItem.h"

class PauseItem;

class ProcessorItem: public BaseItem
{
	friend class InputItem;
	friend class OutputItem;

public:
	ProcessorItem(QString const& _type, Properties const& _pr = Properties(), QSizeF const& _size = QSizeF(0, 0));
	~ProcessorItem() { delete m_processor; }

	enum { Type = BaseItem::Type + 1 };
	virtual int			type() const { return Type; }

	virtual bool		isRunning() const { return processor() && processor()->isRunning(); }
	virtual float		cyclePoint() const { return (processor()->guardsCrossed() % 36) * 360.0 / 36; }
	virtual void		togglePause();

	virtual bool		connectYourself(ProcessorGroup& _group);
	virtual void		disconnectYourself();
	virtual void		typesConfirmed();
	virtual void		tick();
	virtual QString		name() const { return processor()->name(); }

	Processor*			processor() const { return m_processor; }

	static void			fromDom(QDomElement& _element, QGraphicsScene* _scene);
	virtual QDomElement	saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n = "processor") const;

protected:
	virtual QSizeF		centreMin() const;
//	virtual QSizeF		centrePref() const { return QSizeF(processor()->preferedWidth(), processor()->preferedHeight()); }
	virtual QColor		outlineColour() const { return processor()->outlineColour(); }
	virtual void		paintCentre(QPainter* _p) { processor()->draw(*_p, centreRect().size()); }
	virtual uint		redrawPeriod() const { return processor()->redrawPeriod(); }

	virtual Processor*	reconstructProcessor();
	virtual void		geometryChanged();
	virtual void		positionChanged();
	virtual void		propertiesChanged(QString const& _newName = QString::null);

	Processor*			m_processor;

private:
	QString				m_type;
	uint				m_multiplicity;
};
