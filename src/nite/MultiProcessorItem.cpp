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

class ControlsItem: public QGraphicsRectItem
{
public:
	ControlsItem(MultiProcessorItem* _p);

	MultiProcessorItem* mpi() const { return dynamic_cast<MultiProcessorItem*>(parentItem()); }
	void init();

protected:
	virtual void paint(QPainter* _p, QStyleOptionGraphicsItem const* _o, QWidget* _w) { QGraphicsRectItem::paint(_p, _o, _w); }
	virtual void mousePressEvent(QGraphicsSceneMouseEvent*)
	{
		setBrush(mpi()->outlineColour().lighter(200));
	}
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
	{
		setBrush(mpi()->outlineColour());
		if (rect().contains(_e->pos()))
			mpi()->toggleShowAll();
	}
};

ControlsItem::ControlsItem(MultiProcessorItem* _p):
	QGraphicsRectItem(_p)
{
	setRect(0, 0, 48, 16);
}

void ControlsItem::init()
{
	setPen(QPen(QColor(Qt::black), 0));
	setBrush(mpi()->outlineColour());
}

MultiProcessorItem::MultiProcessorItem(QString const& _type, Properties const& _pr, QString const& _name, QSizeF const& _size):
	BaseItem			(_pr, _size),
	m_processor			(0),
	m_type				(_type),
	m_multiProcessor	(0),
	m_showAll			(false),
	m_rowSize			(1),
	m_face				(0)
{
	m_controls = new ControlsItem(this);
	propertiesChanged(_name);
}

MultiProcessorItem::MultiProcessorItem(Properties const& _pr, QSizeF const& _size):
	BaseItem			(_pr, _size),
	m_processor			(0),
	m_type				(QString::null),
	m_multiProcessor	(0),
	m_showAll			(false),
	m_rowSize			(1),
	m_face				(0)
{
	m_controls = new ControlsItem(this);
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
	if (m_multiProcessor && m_multiProcessor->isRunning())
	{
		m_multiProcessor->update(completeProperties());
		m_processor->update(completeProperties());
		return;
	}

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

		m_controls->init();
		BaseItem::propertiesChanged(_newName);
	}
	else
	{
		m_multiProcessor = 0;
		m_processor = 0;
	}
}

void MultiProcessorItem::updateMultiplicities()
{
	if (multiProcessor())
	{
		foreach (MultipleInputItem* i, filter<MultipleInputItem>(childItems()))
			i->setMultiplicity(multiProcessor()->multiplicity());
		foreach (MultipleOutputItem* i, filter<MultipleOutputItem>(childItems()))
			i->setMultiplicity(multiProcessor()->multiplicity());
		if (m_multiProcessor->knowMultiplicity() && m_face >= m_multiProcessor->multiplicity())
			m_face = 0;
	}
}

QSizeF MultiProcessorItem::centreMin() const
{
	if (m_processor)
		return QSizeF(m_processor->minWidth(), max((double)m_processor->minHeight(), portLateralMargin + portLateralMargin + max(m_processor->numInputs(), m_processor->numOutputs()) * (portLateralMargin + portSize)));
	return QSizeF(0, 0);
}

QSizeF MultiProcessorItem::centrePref() const
{
	return QSizeF(processor()->width(), processor()->height());
}

Processor* MultiProcessorItem::processor() const
{
	if (m_multiProcessor && m_multiProcessor->knowMultiplicity() && m_face < m_multiProcessor->multiplicity() && m_multiProcessor->processor(m_face))
		return m_multiProcessor->processor(m_face);
	return m_processor;
}

void MultiProcessorItem::paintCentre(QPainter* _p)
{
	BaseItem::paintCentre(_p);
	if (m_showAll && multiProcessor() && multiProcessor()->knowMultiplicity())
	{
		int rows = (multiProcessor()->multiplicity() + max(1u, m_rowSize) - 1) / max(1u, m_rowSize);
		int tw = centreRect().width();
		int th = centreRect().height();
		for (uint i = 0; i < multiProcessor()->multiplicity(); i++)
		{
			_p->save();
			int x = i % m_rowSize * tw / m_rowSize;
			int y = i / m_rowSize * th / rows;
			int w = (i % m_rowSize + 1) * tw / m_rowSize - x;
			int h = (i / m_rowSize + 1) * th / rows - y;
			_p->translate(x, y);
			multiProcessor()->processor(i)->draw(*_p, QSizeF(w, h));
			_p->restore();
		}
	}
	else
		processor()->draw(*_p, centreRect().size());
}

void MultiProcessorItem::postCreate()
{
	PropertiesInfo pi = m_processor->properties();
	pi = pi("Processors Multiplicity", 0, "The number of processors that make this object. (>0 to set, 0 for automatic)");
	setDefaultProperties(pi);
}

void MultiProcessorItem::geometryChanged()
{
	if (!multiProcessor())
		return;

	QVector<MultipleInputItem*> miis(multiProcessor()->numMultiInputs(), 0);
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

	QVector<MultipleOutputItem*> mois(multiProcessor()->numMultiOutputs(), 0);
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

	updateMultiplicities();

	m_controls->setPos(outlineRect().topRight() - QPointF(m_controls->rect().width(), m_controls->rect().height()));

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
	multiProcessor()->resetMulti();

	foreach (MultipleOutputItem* mii, filter<MultipleOutputItem>(childItems()))
	{
		int c = 0;
		foreach (MultipleConnectionItem* i, filter<MultipleConnectionItem>(scene()->items()))
			if (i->from() == mii)
				c++;
		if (c > 1)
			multiProcessor()->MultiSource::split(mii->index());
	}

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
		Connection::Tristate t = mci->from()->source()->connect(mci->from()->index(), multiProcessor(), mci->to()->index());
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

void MultiProcessorItem::typesConfirmed()
{
	BaseItem::typesConfirmed();
	updateMultiplicities();
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
