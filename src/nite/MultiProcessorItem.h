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
using namespace Geddei;

#include "BaseItem.h"

class ControlsItem;

class MultiProcessorItem: public BaseItem
{
public:
	MultiProcessorItem(QString const& _type, Properties const& _pr = Properties(), QString const& _name = QString::null, QSizeF const& _size = QSizeF(0, 0));
	~MultiProcessorItem();

	enum { ItemType = UserType + 7 };
	virtual int			type() const { return ItemType; }

	virtual QString		typeName() const { return m_processor->type(); }
	virtual QString		name() const { return multiProcessor() ? multiProcessor()->name() : QString::null; }

	MultiProcessor*		multiProcessor() const { return m_multiProcessor; }
	Processor*			processor() const;

	void				toggleShowAll() { m_showAll = !m_showAll; }
	bool				showingAll() { return m_showAll; }
	int					face() { return m_face; }
	void				decRowSize() { m_rowSize = max(2u, m_rowSize) - 1; }
	void				incRowSize() { if (multiplicity() != Undefined) m_rowSize = min(multiplicity() - 1u, m_rowSize) + 1; }
	void				prevFace() { if (multiplicity() != Undefined) m_face = (m_face - 1 + multiplicity()) % multiplicity(); }
	void				nextFace() { m_face = (m_face + 1) % multiplicity(); }

	virtual void		prepYourself(ProcessorGroup&);
	virtual bool		connectYourself();
	virtual void		disconnectYourself();
	virtual void		typesConfirmed();

	static void			fromDom(QDomElement& _element, QGraphicsScene* _scene);
	virtual QDomElement saveYourself(QDomElement& _root, QDomDocument& _doc) const { return saveYourself(_root, _doc, "multiprocessor"); }
	QDomElement			saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const;

	virtual QColor		outlineColour() const { return processor() ? processor()->outlineColour() : Qt::black; }

	virtual QRectF		adjustBounds(QRectF const& _wouldBe) const;

protected:
	MultiProcessorItem(Properties const& _pr = Properties(), QSizeF const& _size = QSizeF(0, 0));

	virtual void		propertiesChanged(QString const& _newName = QString::null);
	virtual void		geometryChanged();
	virtual void		positionChanged();

	uint				multiplicity() const { if (multiProcessor() && multiProcessor()->knowMultiplicity()) return multiProcessor()->multiplicity(); return Undefined; }

	virtual QSizeF		centreMin() const;
	virtual QSizeF		centrePref() const;
	virtual void		paintCentre(QPainter* _p);
	virtual void		paintOutline(QPainter* _p);
	virtual uint		redrawPeriod() const { return processor()->redrawPeriod(); }

	void				updateMultiplicities();

	virtual MultiProcessorCreator* newCreator() { return new FactoryCreator(m_type); }
	virtual void		postCreate();

	Processor*			m_processor;
	ControlsItem*		m_controls;

	virtual QTask*		primaryTask() const { return (multiplicity() != Undefined && multiplicity()) ? dynamic_cast<CoProcessor*>(m_multiProcessor->processor(0)) : 0; }

private:
	QString				m_type;
	MultiProcessor*		m_multiProcessor;

	bool				m_showAll;
	uint				m_rowSize;
	uint				m_face;
};
