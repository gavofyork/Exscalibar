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
	uint whichRect(QPointF _pos) const
	{
		if (rect().contains(_pos))
			return _pos.x() * 3 / rect().width();
		return Undefined;
	}
	QRectF rectOf(uint _i) const
	{
		if (_i != Undefined)
			return QRectF(rect().left() + rect().width() / 3 * _i, rect().top(), rect().width() / 3, rect().height());
		return QRectF();
	}
	virtual void paint(QPainter* _p, QStyleOptionGraphicsItem const* _o, QWidget* _w)
	{
		QGraphicsRectItem::paint(_p, _o, _w);
		if (m_clickedRect != Undefined)
			_p->fillRect(rectOf(m_clickedRect).adjusted(0, 0, 0, 1), QBrush(mpi()->outlineColour().lighter(200)));
		_p->setPen(mpi()->outlineColour());
		_p->drawLine(rect().bottomLeft(), rect().bottomRight());
		_p->setPen(QPen(QColor::fromHsv(0, 0, 32), 0, Qt::DotLine));
		for (uint i = 1; i < 3; i++)
			_p->drawLine(rectOf(i).topLeft(), rectOf(i).bottomLeft());
		_p->drawLine(rect().bottomLeft(), rect().bottomRight());
		if (mpi()->showingAll())
		{
			_p->setPen(QPen(QColor(Qt::black), 0));
			_p->setBrush(QColor(Qt::white));
			_p->drawRect(rectOf(1).adjusted(5, 5, -5, -5));
			_p->drawRect(rectOf(0).adjusted(2, 6, -2, -6));
			_p->drawRect(rectOf(2).adjusted(6, 2, -6, -2));
		}
		else
		{
			_p->setPen(QPen(QColor(Qt::black), 0));
			_p->setBrush(QColor(Qt::white));
			_p->drawText(rectOf(1), QString::number(mpi()->face()), QTextOption(Qt::AlignCenter));
			QRectF r = rectOf(0).adjusted(5, 5, -5, -5);
			_p->drawLine(r.topRight(), QPointF(r.left(), r.center().y()));
			_p->drawLine(r.bottomRight(), QPointF(r.left(), r.center().y()));
			r = rectOf(2).adjusted(5, 5, -5, -5);
			_p->drawLine(r.topLeft(), QPointF(r.right(), r.center().y()));
			_p->drawLine(r.bottomLeft(), QPointF(r.right(), r.center().y()));
		}
		_p->drawLines(QVector<QPointF>() << rect().bottomLeft() << rect().topLeft() << rect().topLeft() << rect().topRight() << rect().topRight() << rect().bottomRight());
	}
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* _e)
	{
		m_clickedRect = whichRect(_e->pos());
		update();
	}
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
	{
		if (whichRect(_e->pos()) == m_clickedRect)
		{
			if (mpi()->showingAll())
				switch (m_clickedRect)
				{
				case 0: mpi()->decRowSize(); break;
				case 1: mpi()->toggleShowAll(); break;
				case 2: mpi()->incRowSize(); break;
				}
			else
				switch (m_clickedRect)
				{
				case 0: mpi()->prevFace(); break;
				case 1: mpi()->toggleShowAll(); break;
				case 2: mpi()->nextFace(); break;
				}
		}
		m_clickedRect = Undefined;
		update();
	}
	uint m_clickedRect;
};

ControlsItem::ControlsItem(MultiProcessorItem* _p):
	QGraphicsRectItem	(_p),
	m_clickedRect		(Undefined)
{
	setRect(0, 0, 48, 16);
	setCursor(Qt::ArrowCursor);
}

void ControlsItem::init()
{
	setPen(Qt::NoPen);
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
		name = this->name();
	if (name.isEmpty())
		name = QString::number((long uint)(this));

	// TODO: Need isRunning and update on MultiProcessor.
	if (m_multiProcessor && m_multiProcessor->isRunning())
	{
		m_multiProcessor->update(completeProperties());
		m_processor->update(completeProperties());
		assert(_newName.isEmpty() || _newName == this->name());
		return;
	}

	delete m_multiProcessor;
	delete m_processor;
	MultiProcessorCreator* creator = newCreator();
	if (creator)
	{
		m_processor = creator->newProcessor();
		postCreate();

		m_processor->init(QString::null, completeProperties());

		m_multiProcessor = new MultiProcessor(creator);
		m_multiProcessor->init(name, completeProperties());

		if (m_controls)
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

static float const marginWidth = 3.f;

QSizeF MultiProcessorItem::centreMin() const
{
	int rows = m_showAll ? (multiProcessor()->multiplicity() + max(1u, m_rowSize) - 1) / max(1u, m_rowSize) : 1;
	int cols = m_showAll ? m_rowSize : 1;
	if (m_processor)
		return QSizeF(processor()->minWidth() * cols + (cols - 1) * marginWidth, max((double)processor()->minHeight() * rows + (rows - 1) * marginWidth, portLateralMargin + portLateralMargin + max(m_processor->numInputs(), m_processor->numOutputs()) * (portLateralMargin + portSize)));
	return QSizeF(0, 0);
}

QSizeF MultiProcessorItem::centrePref() const
{
	int rows = m_showAll ? (multiProcessor()->multiplicity() + max(1u, m_rowSize) - 1) / max(1u, m_rowSize) : 1;
	int cols = m_showAll ? m_rowSize : 1;
	return QSizeF(processor()->width() * cols + (cols - 1) * marginWidth, processor()->height() * rows + (rows - 1) * marginWidth);
}

QRectF MultiProcessorItem::boundingRect() const
{
	return outlineRect().adjusted(-4.f, -12.f, 4.f, 4.f);
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
	for (int i = 1; i < int((m == Undefined) ? 5 : m); i++)
	{
		if (m == Undefined)
			_p->setPen(QPen(QColor::fromHsv(0, 0, (i - 1) * 255 / 4), 0));
		_p->drawLine(outlineRect().topLeft() + QPointF(i, -i * 2), outlineRect().topRight() + QPointF(-i, -i * 2));
	}
}

void MultiProcessorItem::paintCentre(QPainter* _p)
{
	if (m_showAll && multiProcessor() && multiProcessor()->knowMultiplicity())
	{
		int rows = (multiProcessor()->multiplicity() + max(1u, m_rowSize) - 1) / max(1u, m_rowSize);
		int tw = centreRect().width();
		int th = centreRect().height();
		int mw = marginWidth;
		for (uint i = 0; i < multiProcessor()->multiplicity(); i++)
		{
			_p->save();
			int x = i % max(1u, m_rowSize) * (tw + mw) / max(1u, m_rowSize);
			int y = i / max(1u, m_rowSize) * (th + mw) / rows;
			int w = (i % max(1u, m_rowSize) + 1) * (tw + mw) / max(1u, m_rowSize) - x - mw;
			int h = (i / max(1u, m_rowSize) + 1) * (th + mw) / rows - y - mw;
			_p->translate(x, y);
			_p->setClipRect(QRectF(0, 0, w, h));
			_p->fillRect(QRectF(0, 0, w, h), QColor(255, 255, 255));
			multiProcessor()->processor(i)->draw(*_p, QSizeF(w, h));
			_p->restore();
		}
	}
	else
	{
		BaseItem::paintCentre(_p);
		processor()->draw(*_p, centreRect().size());
	}
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
		i->setPos(-1.f, 6.f * 3 / 2 + (8.f + i->size().height()) * i->index());

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
		i->setPos(centreRect().right() + 1.f, 6.f * 3 / 2 + (8.f + i->size().height()) * i->index());

	updateMultiplicities();

	if (m_controls)
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
	foreach (MultipleInputItem* mii, filter<MultipleInputItem>(childItems()))
		mii->typesConfirmed();
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
	proc.setAttribute("type", m_processor->type());
	proc.setAttribute("showAll", m_showAll);
	proc.setAttribute("rowSize", m_rowSize);
	proc.setAttribute("face", m_face);
	BaseItem::exportDom(proc, _doc);
	_root.appendChild(proc);
	return proc;
}
