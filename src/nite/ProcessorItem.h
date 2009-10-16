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
	ProcessorItem(QString const& _type, Properties const& _pr = Properties(), QSizeF const& _size = QSizeF());
	~ProcessorItem() { delete m_processor; }

	enum { Type = BaseItem::Type + 1 };
	virtual int			type() const { return Type; }

	virtual bool		isRunning() const { return processor() && processor()->isRunning(); }
	virtual float		cyclePoint() const { return (processor()->guardsCrossed() % 36) * 360.0 / 36; }
	virtual void		togglePause();

	void				setTryMulti(bool _m) { m_tryToShowMulti = _m; updateMultiDisplay(); }
	void				updateMultiDisplay();

	virtual void		prepYourself(ProcessorGroup&);
	virtual bool		connectYourself();
	virtual void		disconnectYourself();
	virtual void		typesConfirmed();
	virtual void		tick();
	virtual QString		name() const { return processor()->name(); }
	virtual QString		typeName() const { return processor()->type(); }

	Processor*			processor() const { return m_processor; }

	static void			fromDom(QDomElement& _element, QGraphicsScene* _scene);
	virtual QDomElement	saveYourself(QDomElement& _root, QDomDocument& _doc) const { return saveYourself(_root, _doc, "processor"); }
	QDomElement			saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const;

protected:
	virtual QSizeF		centreMin() const;
	virtual QSizeF		centrePref() const { return QSizeF(processor()->width(), processor()->height()); }
	virtual QColor		outlineColour() const { return processor()->outlineColour(); }
	virtual void		paintCentre(QPainter* _p) { BaseItem::paintCentre(_p); processor()->draw(*_p, centreRect().size()); }
	virtual uint		redrawPeriod() const { return processor()->redrawPeriod(); }

	virtual QList<QPointF> magnetism(BaseItem const* _b, bool _moving) const;

	virtual Processor*	reconstructProcessor();
	virtual void		geometryChanged();
	virtual void		positionChanged();
	virtual void		propertiesChanged(QString const& _newName = QString::null);

	Processor*			m_processor;

private:
	void				updateMultiplicities();

	QString				m_type;
	bool				m_tryToShowMulti;
	uint				m_multiplicity;
};
