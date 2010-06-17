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

#include "ProcessorBasedItem.h"
#include "InputItem.h"
#include "OutputItem.h"

extern const QSizeF portSize;
extern const QSizeF multiPortSize;
extern const double portLateralMargin;
extern const double portLongalMargin;

class ProcessorItem: public ProcessorBasedItem
{
	friend class InputItem;
	friend class OutputItem;

public:
	ProcessorItem(QString const& _type, Properties const& _pr = Properties(), QSizeF const& _size = QSizeF());
	~ProcessorItem() { delete m_nominal; }

	enum { ItemType = UserType + 1 };
	virtual int			type() const { return ItemType; }

	virtual Processor*	prototypal() const { return m_nominal; }
	virtual Processor*	executive() const { return m_nominal; }

	static void			fromDom(QDomElement& _element, QGraphicsScene* _scene);
	virtual QDomElement	saveYourself(QDomElement& _root, QDomDocument& _doc) const { return saveYourself(_root, _doc, "processor"); }
	QDomElement			saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const;

	void				propertiesChanged(QString const& _newName = QString::null);

	virtual void		unprepYourself();

private:
	Processor*			m_nominal;
	QString				m_type;
	bool				m_propertiesDirty;
};
