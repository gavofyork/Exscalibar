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

#include "MultipleInputItem.h"
#include "MultipleOutputItem.h"
#include "MultipleConnectionItem.h"
#include "MultiProcessorItem.h"

MultiProcessorItem::MultiProcessorItem(QString const& _type, Properties const& _pr, QString const& _name, QSizeF const& _size):
	BaseItem			(_pr, _size),
	m_type				(_type),
	m_multiProcessor	(0),
	m_processor			(0)
{
	propertiesChanged(_name);
}

MultiProcessorItem::MultiProcessorItem(Properties const& _pr, QSizeF const& _size):
	BaseItem			(_pr, _size),
	m_type				(QString::null),
	m_multiProcessor	(0),
	m_processor			(0)
{
}

MultiProcessorItem::~MultiProcessorItem()
{
	delete m_multiProcessor;
	delete m_processor;
}

void MultiProcessorItem::propertiesChanged(QString const& _newName)
{
	QString name = _newName;
	if (name.isEmpty())
		name = QString::number((long uint)(this));

	// TODO: Need isRunning and update on MultiProcessor.
/*	if (m_multiProcessor && m_multiProcessor->isRunning())
	{
		m_multiProcessor->update(completeProperties());
		m_processor->update(completeProperties());
		return;
	}*/

	delete m_multiProcessor;
	delete m_processor;
	MultiProcessorCreator* creator = newCreator();
	if (creator)
	{
		m_processor = creator->newProcessor();
		postCreate();

		m_processor->init(name, completeProperties());

		m_multiProcessor = new MultiProcessor(creator);
		m_multiProcessor->init(name, completeProperties());

		BaseItem::propertiesChanged(_newName);
	}
	else
	{
		m_multiProcessor = 0;
		m_processor = 0;
	}
}

QSizeF MultiProcessorItem::centreMin() const
{
	if (m_processor)
		return QSizeF(m_processor->minWidth(), max((double)m_processor->minHeight(), portLateralMargin + portLateralMargin + max(m_processor->numInputs(), m_processor->numOutputs()) * (portLateralMargin + portSize)));
	return QSizeF(0, 0);
}

void MultiProcessorItem::geometryChanged()
{
	QVector<MultipleInputItem*> miis(1, 0);
	foreach (MultipleInputItem* mii, filter<MultipleInputItem>(childItems()))
		if ((int)mii->index() < miis.count())
			miis[mii->index()] = mii;
		else
			delete mii;
	for (int i = 0; i < miis.count(); i++)
		if (!miis[i])
			miis[i] = new MultipleInputItem(i, this, QSizeF(10.f, 8.f));
	foreach (MultipleInputItem* i, miis)
		i->setPos(-1.f, 8.f * 3 / 2 + (8.f + i->size().height()) * i->index());

	QVector<MultipleOutputItem*> mois(1, 0);
	foreach (MultipleOutputItem* moi, filter<MultipleOutputItem>(childItems()))
		if ((int)moi->index() < mois.count())
			mois[moi->index()] = moi;
		else
			delete moi;
	for (int i = 0; i < mois.count(); i++)
		if (!mois[i])
			mois[i] = new MultipleOutputItem(i, this, QSizeF(10.f, 8.f));
	foreach (MultipleOutputItem* i, mois)
		i->setPos(centreRect().right() + 1.f, 8.f * 3 / 2 + (8.f + i->size().height()) * i->index());

	BaseItem::geometryChanged();
}

void MultiProcessorItem::positionChanged()
{
	if (scene())
		foreach (QGraphicsItem* i, scene()->items())
			if (MultipleConnectionItem* mci = dynamic_cast<MultipleConnectionItem*>(i))
				if (mci->to()->multiProcessorItem() == this || mci->from()->multiProcessorItem() == this)
					mci->rejigEndPoints();

	BaseItem::positionChanged();
}

void MultiProcessorItem::prepYourself(ProcessorGroup& _g)
{
	multiProcessor()->setGroup(_g);
	multiProcessor()->disconnectAll();
	BaseItem::prepYourself(_g);
}

bool MultiProcessorItem::connectYourself()
{
	foreach (MultipleInputItem* mii, filter<MultipleInputItem>(childItems()))
	{
		QList<MultipleConnectionItem*> mcis = filter<MultipleConnectionItem>(mii->childItems());
		if (mcis.size() != 1)
			return false;
		MultipleConnectionItem* mci = mcis[0];
		Connection::Tristate t = mci->from()->source()->connect(multiProcessor());
		mci->setValid(t != Connection::Failed);
		if (t == Connection::Failed)
			return false;
	}
	return BaseItem::connectYourself();
}

void MultiProcessorItem::disconnectYourself()
{
	BaseItem::disconnectYourself();
	geometryChanged();
	m_multiProcessor->disconnectAll();
	m_multiProcessor->setNoGroup();
}

void MultiProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	MultiProcessorItem* pi = new MultiProcessorItem(_element.attribute("type"));
	pi->importDom(_element, _scene);
}

QDomElement MultiProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const
{
	QDomElement proc = _doc.createElement(_n);
	proc.setAttribute("type", m_processor->type());
	BaseItem::exportDom(proc, _doc);
	_root.appendChild(proc);
	return proc;
}
