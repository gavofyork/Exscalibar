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

#include <Geddei>
using namespace Geddei;

#include "ProcessorsScene.h"
#include "MultipleInputItem.h"
#include "MultipleOutputItem.h"
#include "MultipleConnectionItem.h"
#include "MultiProcessorItem.h"
#include "ControlsItem.h"
#include "ProcessorItem.h"

MultiProcessorItem::MultiProcessorItem(QString const& _type, Properties const& _pr, QString const& _name, QSizeF const& _size):
	BaseItem			(_pr, _size),
	m_processor			(0),
	m_controls			(new ControlsItem(this)),
	m_multiProcessor	(0),
	m_type				(_type),
	m_multiplicity		(Undefined),
	m_showAll			(false),
	m_rowSize			(1),
	m_face				(0),
	m_propertiesDirty	(false)
{
	propertiesChanged(_name);
}

MultiProcessorItem::MultiProcessorItem(Properties const& _pr, QSizeF const& _size):
	BaseItem			(_pr, _size),
	m_processor			(0),
	m_controls			(0),
	m_multiProcessor	(0),
	m_type				(QString::null),
	m_multiplicity		(Undefined),
	m_showAll			(false),
	m_rowSize			(1),
	m_face				(0),
	m_propertiesDirty	(false)
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
		name = this->name();
	if (name.isEmpty())
		name = QString::number((long unsigned)(this));

	bool upbi = !m_propertiesDirty;

	// TODO: Need isRunning and update on MultiProcessor.
	if (m_multiProcessor && m_multiProcessor->isRunning())
	{
		m_multiProcessor->update(completeProperties());
		m_processor->update(completeProperties());
		assert(_newName.isEmpty() || _newName == this->name());
		m_propertiesDirty = true;
	}
	else
	{
		delete m_multiProcessor;
		delete m_processor;
		MultiProcessorCreator* creator = new FactoryCreator(m_type);
		if (creator)
		{
			m_processor = creator->newProcessor();
			PropertiesInfo pi = m_processor->properties();
			pi = pi("Processors Multiplicity", 0, "The number of processors that make this object. (>0 to set, 0 for automatic)");
			setDefaultProperties(pi);

			m_processor->init(name, completeProperties());

			m_multiProcessor = new MultiProcessor(creator);
			m_multiProcessor->init(name, completeProperties());

			if (m_controls)
				m_controls->init();
		}
		else
		{
			m_multiProcessor = 0;
			m_processor = 0;
		}
		m_propertiesDirty = false;
	}
	if (upbi)
		BaseItem::propertiesChanged(_newName);
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

static float const marginWidth = 1.f;

QSizeF MultiProcessorItem::centreMin() const
{
	int rows = m_showAll ? (multiProcessor()->multiplicity() + max(1u, m_rowSize) - 1) / max(1u, m_rowSize) : 1;
	int cols = m_showAll ? m_rowSize : 1;
	if (m_processor)
		return QSizeF(processor()->minWidth() * cols + (cols - 1) * marginWidth, max((double)processor()->minHeight() * rows + (rows - 1) * marginWidth, max(m_processor->numInputs(), m_processor->numOutputs()) * (portLateralMargin + portSize.height()) - portLateralMargin / 2));
	return QSizeF(0, 0);
}

QSizeF MultiProcessorItem::centrePref() const
{
	int rows = m_showAll ? (multiProcessor()->multiplicity() + max(1u, m_rowSize) - 1) / max(1u, m_rowSize) : 1;
	int cols = m_showAll ? m_rowSize : 1;
	return QSizeF(processor()->width() * cols + (cols - 1) * marginWidth, processor()->height() * rows + (rows - 1) * marginWidth);
}

QRectF MultiProcessorItem::adjustBounds(QRectF const& _wouldBe) const
{
	return _wouldBe.adjusted(0, -8, 0, 0);
}

Processor* MultiProcessorItem::processor() const
{
	if (m_multiProcessor && m_multiProcessor->knowMultiplicity() && m_face < m_multiProcessor->multiplicity() && m_multiProcessor->processor(m_face))
		return m_multiProcessor->processor(m_face);
	return m_processor;
}

void MultiProcessorItem::paintOutline(QPainter* _p)
{
	BaseItem::paintOutline(_p);
	uint m = multiplicity();
	_p->setPen(QPen(Qt::black, 0));
	for (int i = 1; i < min(5, int((m == Undefined) ? 5 : m)); i++)
	{
		_p->setPen(QPen(QColor::fromHsv(0, 0, (i - 1) * 255 / 4), 0));
		_p->drawLine(exteriorRect().topLeft() + QPointF(i, -i * 2), exteriorRect().topRight() + QPointF(-i, -i * 2));
	}
}

void MultiProcessorItem::timerEvent(QTimerEvent*)
{
	if (m_showAll)
		// TODO: should update each indiviually.
		update(clientRect().adjusted(2, 2, -2, -2));
	else
		update(clientRect().adjusted(2, 2, -2, -2));
}

void MultiProcessorItem::paintCentre(QPainter* _p)
{
	if (m_showAll && multiProcessor() && multiProcessor()->knowMultiplicity())
	{
		int rows = (multiProcessor()->multiplicity() + max(1u, m_rowSize) - 1) / max(1u, m_rowSize);
		int tw = clientRect().width();
		int th = clientRect().height();
		int mw = marginWidth;
		for (uint i = 0; i < multiProcessor()->multiplicity(); i++)
		{
			_p->save();
			int x = i % max(1u, m_rowSize) * (tw + mw) / max(1u, m_rowSize);
			int y = i / max(1u, m_rowSize) * (th + mw) / rows;
			int w = (i % max(1u, m_rowSize) + 1) * (tw + mw) / max(1u, m_rowSize) - x - mw;
			int h = (i / max(1u, m_rowSize) + 1) * (th + mw) / rows - y - mw;
			x += 2.f;
			y += 2.f;
			w -= 4.f;
			h -= 4.f;
			_p->translate(x, y);
			deepRect(_p, QRectF(0, 0, w, h), true, prototypal()->outlineColour(), true, 2.f, false);
			_p->setClipRect(QRectF(0, 0, w, h));
			//_p->fillRect(QRectF(0, 0, w, h), QColor(255, 255, 255));
			multiProcessor()->processor(i)->draw(*_p, QSizeF(w, h));
			_p->restore();
		}
	}
	else
	{
		_p->save();
		deepRect(_p, clientRect(), true, prototypal()->outlineColour(), false, 2.f, false);
		_p->translate(2, 2);
		processor()->draw(*_p, clientRect().size() - QSizeF(4, 4));
		_p->restore();
	}
}

void MultiProcessorItem::geometryChanged()
{
	if (!multiProcessor())
		return;

	QVector<MultipleInputItem*> miis(numMultiInputs(), 0);
	foreach (MultipleInputItem* mii, filter<MultipleInputItem>(childItems()))
		if ((int)mii->index() < miis.count())
			miis[mii->index()] = mii;
		else
			delete mii;
	for (int i = 0; i < miis.count(); i++)
		if (!miis[i])
			miis[i] = new MultipleInputItem(i, this, multiPortSize);
	foreach (MultipleInputItem* mii, miis)
		mii->setPos(1.f - portLateralMargin, portLongalMargin * 3 / 2 + (portLongalMargin + mii->size().height()) * mii->index());

	QVector<MultipleOutputItem*> mois(numMultiOutputs(), 0);
	foreach (MultipleOutputItem* moi, filter<MultipleOutputItem>(childItems()))
		if ((int)moi->index() < mois.count())
			mois[moi->index()] = moi;
		else
			delete moi;
	for (int i = 0; i < mois.count(); i++)
		if (!mois[i])
			mois[i] = new MultipleOutputItem(i, this, multiPortSize);
	foreach (MultipleOutputItem* moi, mois)
		moi->setPos(interiorRect().width() + portLateralMargin, portLongalMargin * 3 / 2 + (portLongalMargin + moi->size().height()) * moi->index());

	updateMultiplicities();

	if (m_controls)
		m_controls->setPos(exteriorRect().topRight() - QPointF(m_controls->rect().width(), m_controls->rect().height() - 2));

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
	m_multiplicity = Undefined;
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
		QList<MultipleConnectionItem*> mcis;
		foreach (MultipleConnectionItem* i, filter<MultipleConnectionItem>(scene()->items()))
			if (i->to() == mii)
				mcis << i;
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

void MultiProcessorItem::typesConfirmed()
{
	BaseItem::typesConfirmed();
	m_multiplicity = multiProcessor()->multiplicity();
	updateMultiplicities();
	foreach (MultipleInputItem* mii, filter<MultipleInputItem>(childItems()))
		mii->typesConfirmed();
	update();
}

void MultiProcessorItem::disconnectYourself()
{
	geometryChanged();
	m_multiProcessor->disconnectAll();
	m_multiProcessor->setNoGroup();
	BaseItem::disconnectYourself();
}

void MultiProcessorItem::unprepYourself()
{
	if (m_propertiesDirty)
		propertiesChanged();
}

void MultiProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	MultiProcessorItem* pi = new MultiProcessorItem(_element.attribute("type"));
	pi->importDom(_element, _scene);
	pi->m_showAll = _element.attribute("showAll").toInt();
	pi->m_rowSize = _element.attribute("rowSize").toUInt();
	pi->m_face = _element.attribute("face").toUInt();
}

QDomElement MultiProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const
{
	QDomElement proc = _doc.createElement(_n);
	proc.setAttribute("type", typeName());
	proc.setAttribute("showAll", m_showAll);
	proc.setAttribute("rowSize", m_rowSize);
	proc.setAttribute("face", m_face);
	BaseItem::exportDom(proc, _doc);
	_root.appendChild(proc);
	return proc;
}
