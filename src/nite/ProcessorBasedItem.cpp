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

#include "ConnectionItem.h"
#include "ProcessorsView.h"
#include "ProcessorBasedItem.h"
#include "MultipleInputItem.h"
#include "MultipleOutputItem.h"

const QSizeF portSize = QSizeF(8.f, 12.f);
const QSizeF multiPortSize = QSizeF(20.f, 12.f);
const double portLongalMargin = 4.0;
const double portLateralMargin = 2.0;

ProcessorBasedItem::ProcessorBasedItem(Properties const& _pr, QSizeF const& _size):
	BaseItem			(_pr, _size),
	m_tryToShowMulti	(false),
	m_multiplicity		(0)
{
}

void ProcessorBasedItem::tick()
{
	BaseItem::tick();
	foreach (QGraphicsItem* i, childItems())
		if (InputItem* ii = item_cast<InputItem>(i))
			ii->update();
}

void ProcessorBasedItem::updateMultiDisplay()
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

void ProcessorBasedItem::typesConfirmed()
{
	BaseItem::typesConfirmed();
	foreach (QGraphicsItem* i, childItems())
		if (MultipleInputItem* ii = item_cast<MultipleInputItem>(i))
			ii->typesConfirmed();
		else if (InputItem* ii = item_cast<InputItem>(i))
			ii->typesConfirmed();
	updateMultiplicities();
}

void ProcessorBasedItem::updateMultiplicities()
{
	foreach (MultipleInputItem* i, filter<MultipleInputItem>(childItems()))
		i->setMultiplicity(prototypal()->numInputs() > 0 ? prototypal()->numInputs() : Undefined);
	foreach (MultipleOutputItem* i, filter<MultipleOutputItem>(childItems()))
		i->setMultiplicity(prototypal()->numOutputs() > 0 ? prototypal()->numOutputs() : Undefined);
}

float ProcessorBasedItem::interiorPorts() const
{
	return max(prototypal()->numInputs(), prototypal()->numOutputs()) * (portLateralMargin + portSize.height()) - portLateralMargin - 1.f;
}

void ProcessorBasedItem::geometryChanged()
{
	QVector<InputItem*> iis(prototypal()->numInputs() == Undefined ? 0 : prototypal()->numInputs(), 0);
	if (prototypal()->multi() & In)
	{
		MultipleInputItem* mii;
		if (filter<MultipleInputItem>(childItems()).isEmpty())
			(mii = new MultipleInputItem(this, multiPortSize))->hide();
		else
			mii = filter<MultipleInputItem>(childItems())[0];
		mii->setPos(1.f - portLateralMargin, portLongalMargin * 3 / 2);/* + (portLateralMargin + i->size().height()) * i->index());*/
	}
	foreach (InputItem* ii, filter<InputItem>(childItems()))
		if ((int)ii->index() < iis.count())
			iis[ii->index()] = ii;
		else
			delete ii;
	for (int i = 0; i < iis.count(); i++)
		if (!iis[i])
			iis[i] = new InputItem(i, this, portSize);
	foreach (InputItem* i, iis)
		i->setPos(1.f - portLateralMargin, portLongalMargin * 3 / 2 + (portLongalMargin + i->size().height()) * i->index());

	QVector<OutputItem*> ois(prototypal()->numOutputs() == Undefined ? 0 : prototypal()->numOutputs(), 0);
	if (prototypal()->multi() & Out)
	{
		MultipleOutputItem* moi;
		if (filter<MultipleOutputItem>(childItems()).isEmpty())
			(moi = new MultipleOutputItem(this, multiPortSize))->hide();
		else
			moi = filter<MultipleOutputItem>(childItems())[0];
		moi->setPos(interiorRect().width() + portLateralMargin, portLongalMargin * 3 / 2);
	}

	foreach (OutputItem* oi, filter<OutputItem>(childItems()))
		if ((int)oi->index() < ois.count())
			ois[oi->index()] = oi;
		else
			delete oi;
	for (int i = 0; i < ois.count(); i++)
		if (!ois[i])
			ois[i] = new OutputItem(i, this, portSize);
	foreach (OutputItem* i, ois)
		i->setPos(interiorRect().width() + portLateralMargin, portLongalMargin * 3 / 2 + (portLongalMargin + i->size().height()) * i->index());

	updateMultiDisplay();
	BaseItem::geometryChanged();
}

/*QSizeF ProcessorBasedItem::centreMin() const
{
	return QSizeF(prototypal()->minWidth(), max((double)prototypal()->minHeight(), ));
}*/

void ProcessorBasedItem::positionChanged()
{
	if (scene())
		foreach (QGraphicsItem* i, scene()->items())
			if (ConnectionItem* ci = item_cast<ConnectionItem>(i))
			{	if (ci->toProcessor() == this || ci->fromProcessor() == this)
					ci->rejigEndPoints();
			}
			else if (MultipleConnectionItem* mci = item_cast<MultipleConnectionItem>(i))
				if (mci->to()->processorItem() == this || mci->from()->processorItem() == this)
					mci->rejigEndPoints();

	BaseItem::positionChanged();
}

QList<QPointF> ProcessorBasedItem::magnetism(BaseItem const* _b, bool _moving) const
{
	QList<QPointF> ret = BaseItem::magnetism(_b, _moving);
	return ret;
}

void ProcessorBasedItem::prepYourself(ProcessorGroup& _g)
{
	if (executive())
	{
		executive()->setGroup(_g);
		executive()->disconnectAll();
		executive()->resetMulti();

		foreach (MultipleOutputItem* moi, filter<MultipleOutputItem>(childItems()))
		{
			int c = 0;
			foreach (MultipleConnectionItem* i, filter<MultipleConnectionItem>(scene()->items()))
				if (i->from() == moi)
					c++;
			if (c > 1)
				executive()->MultiSource::split(0);
		}
	}
	BaseItem::prepYourself(_g);
}

bool ProcessorBasedItem::connectYourself()
{
	if (executive())
	{
		bool m = false;
		foreach (MultipleInputItem* i, filter<MultipleInputItem>(childItems()))
			if (i->isConnected())
				m = true;

		bool ret = true;
		if (m)
			foreach (MultipleInputItem* mii, filter<MultipleInputItem>(childItems()))
			{
				QList<MultipleConnectionItem*> mcis;
				foreach (MultipleConnectionItem* i, filter<MultipleConnectionItem>(scene()->items()))
					if (i->to() == mii)
						mcis << i;

				if (mcis.size() != 1)
					return false;
				MultipleConnectionItem* mci = mcis[0];

				Connection::Tristate t = mci->from()->source()->connect(mci->from()->index(), executive(), 0);
				mci->setValid(t != Connection::Failed);
				if (t == Connection::Failed)
					ret = false;
			}
		else
			foreach (InputItem* ii, filter<InputItem>(childItems()))
			{
				QList<ConnectionItem*> cis;
				foreach (ConnectionItem* i, filter<ConnectionItem>(scene()->items()))
					if (i->to() == ii)
						cis << i;
				if (cis.size() != 1)
					return false;
				ConnectionItem* ci = cis[0];

				if (ci->nature() == ConnectionItem::Connection)
				{
					assert(ci->fromProcessor()->executive());
					if (ci->from()->inputItem())
					{
						ci->from()->processorItem()->executive()->disconnect(ci->from()->index());
						ci->from()->processorItem()->executive()->split(ci->from()->index());
						ci->from()->processorItem()->executive()->connect(ci->from()->index(), ci->from()->inputItem()->processorItem()->executive(), ci->from()->inputItem()->index());
						ci->from()->setInputItem(0);
					}
					else if (!ci->fromProcessor()->executive()->isConnected(ci->from()->index()))
						ci->from()->setInputItem(ii);
					if (ci->fromProcessor()->executive()->connect(ci->from()->index(), executive(), ii->index()))
						ci->setValid(true);
					else
					{
						ci->setValid(false);
						ret = false;
					}
				}
				else
				{
					ci->setValid(true);
				}
			}
	/*
		qDebug() << executive()->numInputs() << executive()->numOutputs();
		for (uint i = 0; i < executive()->numInputs(); i++)
			qDebug() << &executive()->input(i);
		for (uint i = 0; i < executive()->numOutputs(); i++)
			qDebug() << &executive()->output(i);
	*/
		if (!ret) return false;
	}
	return BaseItem::connectYourself();
}

void ProcessorBasedItem::disconnectYourself()
{
	qDebug() << name() << "Disconnecting...";

	BaseItem::disconnectYourself();
	if (executive())
	{
		for (uint i = 0; i < executive()->numInputs(); i++)
			qDebug() << &executive()->input(i);
		for (uint i = 0; i < executive()->numOutputs(); i++)
			qDebug() << &executive()->output(i);

		if (executive()->multi() && executive()->knowMultiplicity())
			m_multiplicity = executive()->multiplicity();
		else
			m_multiplicity = 0;

		geometryChanged();

		foreach (QGraphicsItem* i, childItems())
			if (OutputItem* ii = item_cast<OutputItem>(i))
				ii->setInputItem();
		executive()->disconnectAll();
		executive()->setNoGroup();
	}
}
