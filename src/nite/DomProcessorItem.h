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
using namespace Geddei;

#include "ProcessorBasedItem.h"

class DomProcessorItem: public ProcessorBasedItem
{
public:
	DomProcessorItem(QString const& _type, Properties const& _pr = Properties(), QSizeF const& _size = QSizeF());

	virtual QString		typeName() const { return m_prototypal->primary()->type(); }
	Properties			baseProperties() const { return Properties("Latency/Throughput", 0.0); }

	virtual QDomElement	saveYourself(QDomElement& _root, QDomDocument& _doc) const;
	static void			fromDom(QDomElement& _element, QGraphicsScene* _scene);

	virtual Processor*	executive() const { return m_combined; }
	QList<DomProcessorItem*> allBefore();
	QList<DomProcessorItem*> allAfter();
	inline QList<DomProcessorItem*> all() { return QList<DomProcessorItem*>(allBefore()) << this << allAfter(); }

	enum { ItemType = UserType + 14 };
	virtual int			type() const { return ItemType; }

protected:
	virtual Processor*	prototypal() const { return m_prototypal; }
	virtual SubProcessor* subPrototypal() const { return m_prototypal->primary(); }
	virtual DomProcessor* domPrototypal() const { return m_prototypal; }

	virtual Properties	subsProperties();
	QString				composedSubs();
	virtual void		paintCentre(QPainter* _p);
	virtual void		prepYourself(ProcessorGroup&);
	virtual bool		connectYourself();
	virtual void		preConnectYourself();
	virtual void		onOutputConnected(OutputItem* o, Connection*);

	virtual void		disconnectYourself();
	virtual void		unprepYourself();

	virtual void		propertiesChanged(QString const& _newName = QString::null);

private:
	DomProcessor*		m_combined;
	DomProcessor*		m_prototypal;

	bool				m_alreadySetSub;
	bool				m_inert;
	QString				m_type;
	QSizeF				m_controlsSize;
};
