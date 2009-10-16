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
#include "MultipleInputItem.h"
#include "MultipleOutputItem.h"

// TODO: MultipleOutputPort
// qobject_cast<ProcessorsScene*>(scene())->beginMultipleConnect(this);

static const double portSize = 8.0;
static const double portLateralMargin = 4.0;

ProcessorItem::ProcessorItem(QString const& _type, Properties const& _pr, QSizeF const& _size):
	BaseItem			(_pr, _size),
	m_processor			(0),
	m_type				(_type),
	m_tryToShowMulti	(false),
	m_multiplicity		(0)
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

void ProcessorItem::updateMultiDisplay()
{
	updateMultiplicities();

	QList<MultipleInputItem*> miis = filter<MultipleInputItem>(childItems());
	QList<InputItem*> iis = filter<InputItem>(childItems());

	bool m = m_tryToShowMulti;
	if (miis.isEmpty())
		m = false;
	else if (iis.isEmpty())
		m = true;
	else
	{
		foreach (MultipleInputItem* i, miis)
			if (i->isConnected())
				m = true;
		foreach (InputItem* i, iis)
			if (i->isConnected())
				m = false;
	}

	foreach (MultipleInputItem* i, miis)
		i->setVisible(m);
	foreach (InputItem* i, iis)
		i->setVisible(!m);

	QList<MultipleOutputItem*> mois = filter<MultipleOutputItem>(childItems());
	QList<OutputItem*> ois = filter<OutputItem>(childItems());

	m = m_tryToShowMulti;
	if (mois.isEmpty())
		m = false;
	else if (ois.isEmpty())
		m = true;
	else
	{
		foreach (MultipleOutputItem* i, mois)
			if (i->isConnected())
				m = true;
		foreach (OutputItem* i, ois)
			if (i->isConnected())
				m = false;
	}

	foreach (MultipleOutputItem* i, mois)
		i->setVisible(m);
	foreach (OutputItem* i, ois)
		i->setVisible(!m);
}

void ProcessorItem::typesConfirmed()
{
	BaseItem::typesConfirmed();
	foreach (QGraphicsItem* i, childItems())
		if (InputItem* ii = dynamic_cast<InputItem*>(i))
			ii->typesConfirmed();
	updateMultiplicities();
}

void ProcessorItem::updateMultiplicities()
{
	foreach (MultipleInputItem* i, filter<MultipleInputItem>(childItems()))
		i->setMultiplicity(processor()->numInputs() > 0 ? processor()->numInputs() : Undefined);
	foreach (MultipleOutputItem* i, filter<MultipleOutputItem>(childItems()))
		i->setMultiplicity(processor()->numOutputs() > 0 ? processor()->numOutputs() : Undefined);
}

Processor* ProcessorItem::reconstructProcessor()
{
	Processor* r = ProcessorFactory::create(m_type);
	setDefaultProperties(r->properties());
	return r;
}

QSizeF ProcessorItem::centreMin() const
{
	return QSizeF(m_processor->minWidth(), max((double)m_processor->minHeight(), portLateralMargin + portLateralMargin + max(m_processor->numInputs(), m_processor->numOutputs()) * (portLateralMargin + portSize)));
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

	BaseItem::propertiesChanged(_newName);
}

void ProcessorItem::geometryChanged()
{
	QVector<InputItem*> iis(m_processor->numInputs() == Undefined ? 0 : m_processor->numInputs(), 0);
	if (m_processor->multi() & In)
	{
		MultipleInputItem* mii;
		if (filter<MultipleInputItem>(childItems()).isEmpty())
			(mii = new MultipleInputItem(this, QSizeF(10.f, 8.f)))->hide();
		else
			mii = filter<MultipleInputItem>(childItems())[0];
		mii->setPos(-1.f, portLateralMargin * 3 / 2 + mii->size().height() / 2);
	}
	foreach (InputItem* ii, filter<InputItem>(childItems()))
		if ((int)ii->index() < iis.count())
			iis[ii->index()] = ii;
		else
			delete ii;
	for (int i = 0; i < iis.count(); i++)
		if (!iis[i])
			iis[i] = new InputItem(i, this, QSizeF(10.f, 8.f));
	foreach (InputItem* i, iis)
		i->setPos(-1.f, portLateralMargin * 3 / 2 + (portLateralMargin + i->size().height()) * i->index());

	QVector<OutputItem*> ois(m_processor->numOutputs() == Undefined ? 0 : m_processor->numOutputs(), 0);
	if (m_processor->multi() & Out)
	{
		MultipleOutputItem* moi;
		if (filter<MultipleOutputItem>(childItems()).isEmpty())
			(moi = new MultipleOutputItem(this, QSizeF(10.f, 8.f)))->hide();
		else
			moi = filter<MultipleOutputItem>(childItems())[0];
		moi->setPos(1.f + centreRect().width(), portLateralMargin * 3 / 2 + moi->size().height() / 2);
	}

	foreach (OutputItem* oi, filter<OutputItem>(childItems()))
		if ((int)oi->index() < ois.count())
			ois[oi->index()] = oi;
		else
			delete oi;
	for (int i = 0; i < ois.count(); i++)
		if (!ois[i])
			ois[i] = new OutputItem(i, this, QSizeF(10.f, 8.f));
	foreach (OutputItem* i, ois)
		i->setPos(1.f + centreRect().width(), portLateralMargin * 3 / 2 + (portLateralMargin + i->size().height()) * i->index());

	updateMultiDisplay();
	BaseItem::geometryChanged();
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
				if (mci->to()->processorItem() == this || mci->from()->processorItem() == this)
					mci->rejigEndPoints();

	BaseItem::positionChanged();
}

QList<QPointF> ProcessorItem::magnetism(BaseItem const* _b, bool _moving) const
{
	QList<QPointF> ret = BaseItem::magnetism(_b, _moving);
	if (_b == this && !_moving && processor())
		ret << QPointF(processor()->width() - centreRect().width(), processor()->height() - centreRect().height());
	return ret;
}

void ProcessorItem::prepYourself(ProcessorGroup& _g)
{
	m_processor->setGroup(_g);
	m_processor->disconnectAll();
	m_processor->resetMulti();
	BaseItem::prepYourself(_g);
}

bool ProcessorItem::connectYourself()
{
	bool m = false;
	foreach (MultipleInputItem* i, filter<MultipleInputItem>(childItems()))
		if (i->isConnected())
			m = true;

	bool ret = true;
	if (m)
		foreach (MultipleInputItem* mii, filter<MultipleInputItem>(childItems()))
		{
			QList<MultipleConnectionItem*> mcis = filter<MultipleConnectionItem>(mii->childItems());
			if (mcis.size() != 1)
				return false;
			MultipleConnectionItem* mci = mcis[0];

			Connection::Tristate t = mci->from()->source()->connect(mci->from()->index(), processor(), 0);
			mci->setValid(t != Connection::Failed);
			if (t == Connection::Failed)
				ret = false;
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
/*
	qDebug() << processor()->numInputs() << processor()->numOutputs();
	for (uint i = 0; i < processor()->numInputs(); i++)
		qDebug() << &processor()->input(i);
	for (uint i = 0; i < processor()->numOutputs(); i++)
		qDebug() << &processor()->output(i);
*/
	if (!ret) return false;
	return BaseItem::connectYourself();
}

void ProcessorItem::disconnectYourself()
{
	qDebug() << name() << "Disconnecting...";

	BaseItem::disconnectYourself();
	qDebug() << processor()->numInputs() << processor()->numOutputs();
	for (uint i = 0; i < processor()->numInputs(); i++)
		qDebug() << &processor()->input(i);
	for (uint i = 0; i < processor()->numOutputs(); i++)
		qDebug() << &processor()->output(i);

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
}

QDomElement ProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const
{
	QDomElement proc = _doc.createElement(_n);
	proc.setAttribute("type", m_processor->type());
	BaseItem::exportDom(proc, _doc);
	_root.appendChild(proc);
	return proc;
}
