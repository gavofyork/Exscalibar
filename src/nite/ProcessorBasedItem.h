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

#include <QtXml>
#include <QtGui>
#include <Geddei>

#include "BaseItem.h"
#include "InputItem.h"
#include "OutputItem.h"

extern const QSizeF portSize;
extern const QSizeF multiPortSize;
extern const double portLateralMargin;

class ProcessorBasedItem: public BaseItem
{
	friend class InputItem;
	friend class OutputItem;

public:
	ProcessorBasedItem(Properties const& _pr = Properties(), QSizeF const& _size = QSizeF());
	~ProcessorBasedItem() {}

	enum { ItemType = UserType + 25 };
	virtual int			type() const { return ItemType; }

	virtual QString		typeName() const { return prototypal()->type(); }
	void				setTryMulti(bool _m) { m_tryToShowMulti = _m; updateMultiDisplay(); }

	QList<InputItem*>	inputs() const;
	QList<OutputItem*>	outputs() const;

	virtual Processor*	executive() const = 0;
	virtual void		typesConfirmed();
	virtual void		prepYourself(ProcessorGroup&);
	virtual bool		connectYourself();
	virtual void		disconnectYourself();
	virtual QTask*		primaryTask() const { return dynamic_cast<CoProcessor*>(executive()); }

protected:
	virtual QList<QPointF> magnetism(BaseItem const* _b, bool _moving) const;

	virtual float		interiorPorts() const;
	virtual void		paintCentre(QPainter* _p) { BaseItem::paintCentre(_p); (executive() ? executive() : prototypal())->draw(*_p, clientRect().size()); }

	virtual void		geometryChanged();
	virtual void		positionChanged();

	void				updateMultiplicities();

private:
	void				updateMultiDisplay();

	bool				m_tryToShowMulti;
	uint				m_multiplicity;
};
