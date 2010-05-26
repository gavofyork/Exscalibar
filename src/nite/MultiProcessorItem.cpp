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
#include "ProcessorItem.h"

class ControlsItem: public QGraphicsRectItem
{
public:
	ControlsItem(MultiProcessorItem* _p, float _size = 10);

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
	virtual void paint(QPainter* _p, QStyleOptionGraphicsItem const*, QWidget*)
	{
		_p->save();
		float yo = pos().y();
		_p->translate(0, -yo);
		_p->setBrush(Qt::NoBrush);
		_p->setPen(mpi()->outerPen());
		_p->setClipRect(rect().adjusted(-1, -1, 0.5, -2).translated(0, yo));
		_p->drawRoundedRect(rect().adjusted(0, 0, 0, 3).translated(0, yo), 3, 3);
		_p->setClipping(false);
		_p->drawLine(rect().translated(0, yo).bottomRight(), rect().translated(0, yo).bottomRight() + QPointF(0, -3));
		_p->fillRect(rect().adjusted(1, 1, -1, 0).translated(0, yo), mpi()->fillBrush());
		if (m_clickedRect != Undefined)
			_p->fillRect(rectOf(m_clickedRect).adjusted(0, 0, 0, 1).translated(0, yo), QColor(255, 255, 255, 127));
		_p->setPen(mpi()->innerPen());
		_p->setClipRect(rect().adjusted(-1, -1, 0, -.5).translated(0, yo));
		_p->drawRoundedRect(rect().adjusted(1, 1, -1, 3).translated(0, yo), 1.5f, 1.5f);
		_p->setClipping(false);
		_p->drawLine(rect().translated(-1, yo).bottomRight(), rect().translated(-1, yo - 3).bottomRight());
		_p->restore();

		QPen pen = mpi()->outerPen();
		pen.setStyle(Qt::DotLine);
		_p->setPen(pen);
		for (uint i = 1; i < 3; i++)
			_p->drawLine(rectOf(i).topLeft(), rectOf(i).bottomLeft() + QPointF(0, -2));
		if (mpi()->showingAll())
		{
			_p->setBrush(Qt::NoBrush);
			for (int i = 0; i < 2; i++)
			{
				_p->setPen(QPen(i ? Qt::white : QColor(0, 0, 0, 96), 1));
				_p->drawRect(rectOf(1).adjusted(3, 3, -3, -3).translated(0, 1 - i));
				_p->drawRect(rectOf(0).adjusted(2, 3, -2, -3).translated(0, 1 - i));
				_p->drawRect(rectOf(2).adjusted(3, 2, -3, -2).translated(0, 1 - i));
			}
		}
		else
		{
			_p->setPen(QPen(QColor(Qt::black), 0));
			_p->setBrush(QColor(Qt::white));
			QFont f;
			f.setPixelSize(m_size - 2.f);
			_p->setFont(f);
			QString t = QString::number(mpi()->face());
			_p->setPen(QPen(QColor(0, 0, 0, 96), 0));
			_p->drawText(rectOf(1).translated(0, 1), t, QTextOption(Qt::AlignCenter));
			_p->setPen(QPen(QColor(Qt::white), 0));
			_p->drawText(rectOf(1), t, QTextOption(Qt::AlignCenter));

			_p->setBrush(Qt::NoBrush);
			for (int i = 0; i < 2; i++)
			{
				_p->setPen(QPen(i ? Qt::white : QColor(0, 0, 0, 96), 1));
				QRectF r = rectOf(0).adjusted(3, 3, -3, -3).translated(0, 1 - i);
				_p->drawLine(r.topRight(), QPointF(r.left(), r.center().y()));
				_p->drawLine(r.bottomRight(), QPointF(r.left(), r.center().y()));
				r = rectOf(2).adjusted(3, 3, -3, -3).translated(0, 1 - i);
				_p->drawLine(r.topLeft(), QPointF(r.right(), r.center().y()));
				_p->drawLine(r.bottomLeft(), QPointF(r.right(), r.center().y()));
			}
		}
		//_p->drawLines(QVector<QPointF>() << rect().bottomLeft() << rect().topLeft() << rect().topLeft() << rect().topRight() << rect().topRight() << rect().bottomRight());
		if (parentItem()->isSelected())
			for (int i = 1; i < 4; i++)
			{
				_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), i));
				_p->drawPolyline(QVector<QPointF>() << rect().translated(0, -2).bottomLeft() << rect().topLeft() << rect().topRight() << rect().bottomRight());
			}
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
	float m_size;
};

ControlsItem::ControlsItem(MultiProcessorItem* _p, float _size):
	QGraphicsRectItem	(_p),
	m_clickedRect		(Undefined),
	m_size				(_size)
{
	setRect(0, 0, _size * 3, _size);
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
		processor()->draw(*_p, clientRect().size());
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
			miis[i] = new MultipleInputItem(i, this, multiPortSize);
	foreach (MultipleInputItem* mii, miis)
		mii->setPos(1.f - portLateralMargin, portLateralMargin * 3 / 2 + (portLateralMargin + mii->size().height()) * mii->index());

	QVector<MultipleOutputItem*> mois(multiProcessor()->numMultiOutputs(), 0);
	foreach (MultipleOutputItem* moi, filter<MultipleOutputItem>(childItems()))
		if ((int)moi->index() < mois.count())
			mois[moi->index()] = moi;
		else
			delete moi;
	for (int i = 0; i < mois.count(); i++)
		if (!mois[i])
			mois[i] = new MultipleOutputItem(i, this, multiPortSize);
	foreach (MultipleOutputItem* moi, mois)
		moi->setPos(centreRect().width() + portLateralMargin, portLateralMargin * 3 / 2 + (portLateralMargin + moi->size().height()) * moi->index());

	updateMultiplicities();

	if (m_controls)
		m_controls->setPos(outlineRect().topRight() - QPointF(m_controls->rect().width(), m_controls->rect().height() - 2));

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
