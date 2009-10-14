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

#include "ConnectionItem.h"
#include "ProcessorsView.h"
#include "ProcessorItem.h"

// TODO: MultipleOutputPort
// qobject_cast<ProcessorsScene*>(scene())->beginMultipleConnect(this);

static const double portSize = 8.0;
static const double portLateralMargin = 4.0;

ProcessorItem::ProcessorItem(QString const& _type, Properties const& _pr, QSizeF const& _size):
	BaseItem		(_pr, _size),
	m_processor		(0),
	m_type			(_type),
	m_multiplicity	(0)
{
	if (!m_type.isEmpty())
		propertiesChanged();
}

void ProcessorItem::togglePause()
{
	Processor* p = processor();
	if (p->isRunning())
	{
		if (p->paused())
			p->unpause();
		else
			p->pause();
		update();
	}
}

void ProcessorItem::tick()
{
	BaseItem::tick();
	foreach (QGraphicsItem* i, childItems())
		if (InputItem* ii = dynamic_cast<InputItem*>(i))
			ii->update();
}

void ProcessorItem::typesConfirmed()
{
	BaseItem::typesConfirmed();
	foreach (QGraphicsItem* i, childItems())
		if (InputItem* ii = dynamic_cast<InputItem*>(i))
			ii->typesConfirmed();
}

Processor* ProcessorItem::reconstructProcessor()
{
	return ProcessorFactory::create(m_type);
}

QSizeF ProcessorItem::centreMin() const
{
	return QSizeF(m_processor->width(), min((double)m_processor->height(), portLateralMargin + portLateralMargin + max(m_processor->numInputs(), m_processor->numOutputs()) * (portLateralMargin + portSize)));
}

void ProcessorItem::propertiesChanged(QString const& _newName)
{
	if (m_processor && m_processor->isRunning())
	{
		m_processor->update(completeProperties());
		return;
	}
	Processor* old = m_processor;
	m_processor = reconstructProcessor();
	if (!m_processor)
	{
		m_processor = old;
		return;
	}
	m_processor->init(_newName.isEmpty() ? old ? old->name() : QString::number( (long uint)this) : _newName, completeProperties());
	delete old;

	geometryChanged();
}

void ProcessorItem::geometryChanged()
{
	QVector<InputItem*> iis(m_processor->multi() & In ? m_multiplicity : m_processor->numInputs(), 0);
	foreach (InputItem* ii, filter<InputItem>(childItems()))
		if ((int)ii->index() < iis.count())
			iis[ii->index()] = ii;
		else
			delete ii;
	for (int i = 0; i < iis.count(); i++)
		if (!iis[i])
			iis[i] = new InputItem(i, this, QSizeF(4.f, 4.f));
	foreach (InputItem* i, iis)
		i->setPos(-1.f, portLateralMargin * 3 / 2 + (portLateralMargin + i->size().height()) * i->index());

	QVector<OutputItem*> ois(m_processor->multi() & Out ? m_multiplicity : m_processor->numOutputs(), 0);
	foreach (OutputItem* oi, filter<OutputItem>(childItems()))
		if ((int)oi->index() < ois.count())
			ois[oi->index()] = oi;
		else
			delete oi;
	for (int i = 0; i < ois.count(); i++)
		if (!ois[i])
			ois[i] = new OutputItem(i, this, QSizeF(10.f, 4.f));
	foreach (OutputItem* i, ois)
		i->setPos(1.f + centreRect().width(), portLateralMargin * 3 / 2 + (portLateralMargin + i->size().height()) * i->index());

	positionChanged();
}

void ProcessorItem::positionChanged()
{
	if (scene())
		foreach (QGraphicsItem* i, scene()->items())
			if (ConnectionItem* ci = dynamic_cast<ConnectionItem*>(i))
			{	if (ci->toProcessor() == this || ci->fromProcessor() == this)
					ci->rejigEndPoints();
			}
			else if (MultipleConnectionItem* mci = dynamic_cast<MultipleConnectionItem*>(i))
				if (mci->toProcessor() == this || mci->fromProcessor() == this)
					mci->rejigEndPoints();
}

bool ProcessorItem::connectYourself(ProcessorGroup& _g)
{
	m_processor->setGroup(_g);
	bool ret = true;
	if (filter<MultipleConnectionItem>(childItems()).count())
	{
		QList<MultipleConnectionItem*> mcis = filter<MultipleConnectionItem>(childItems());
		MultipleConnectionItem* mci = mcis[0];
		if (mci->fromProcessor()->m_processor->MultiSource::deferConnect(m_processor, 1))
		{
			mci->setValid(false);
			ret = false;
		}
		else
		{
			mci->fromProcessor()->m_processor->MultiSource::connect(m_processor);
			mci->setValid(true);
		}
	}
	else
	foreach (InputItem* ii, filter<InputItem>(childItems()))
	{
		QList<ConnectionItem*> cis = filter<ConnectionItem>(ii->childItems());
		if (cis.size() != 1)
			return false;
		ConnectionItem* ci = cis[0];
		if (ci->from()->inputItem())
		{
			ci->from()->processorItem()->m_processor->disconnect(ci->from()->index());
			ci->from()->processorItem()->m_processor->split(ci->from()->index());
			ci->from()->processorItem()->m_processor->connect(ci->from()->index(), ci->from()->inputItem()->processorItem()->processor(), ci->from()->inputItem()->index());
			ci->from()->setInputItem(0);
		}
		else if (!ci->from()->processorItem()->m_processor->isConnected(ci->from()->index()))
			ci->from()->setInputItem(ii);
		if (ci->from()->processorItem()->m_processor->connect(ci->from()->index(), m_processor, ii->index()))
			ci->setValid(true);
		else
		{
			ci->setValid(false);
			ret = false;
		}
	}
	if (!ret) return false;
	return BaseItem::connectYourself(_g);
}

void ProcessorItem::disconnectYourself()
{
	BaseItem::disconnectYourself();

	if (m_processor->multi() && m_processor->knowMultiplicity())
		m_multiplicity = m_processor->multiplicity();
	else
		m_multiplicity = 0;
	geometryChanged();
	foreach (QGraphicsItem* i, childItems())
		if (OutputItem* ii = dynamic_cast<OutputItem*>(i))
			ii->setInputItem();
	m_processor->disconnectAll();
	m_processor->setNoGroup();
}

void ProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	ProcessorItem* pi = new ProcessorItem(_element.attribute("type"));
	pi->importDom(_element, _scene);
	pi->setName(_element.attribute("name"));
}

QDomElement ProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const
{
	QDomElement proc = _doc.createElement(_n);
	proc.setAttribute("type", m_processor->type());
	proc.setAttribute("name", m_processor->name());
	BaseItem::exportDom(proc, _doc);
	_root.appendChild(proc);
	return proc;
}
