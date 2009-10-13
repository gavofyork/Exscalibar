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

#include "MultiProcessorItem.h"

MultiProcessorItem::MultiProcessorItem(QString const& _type, Properties const& _pr, QString const& _name, QSizeF const& _size):
	m_creator			(new FactoryCreator(_type)),
	m_multiProcessor	(0),
	m_processor			(0)
{
	setAcceptHoverEvents(true);
	setFlags(ItemClipsToShape | ItemIsFocusable | ItemIsSelectable | ItemIsMovable);

	propertiesChanged(_name);

	setPos(round(pos().x()) - .5f, round(pos().y()) - .5f);
}

MultiProcessorItem::~MultiProcessorItem()
{
	delete m_multiProcessor;
	delete m_processor;
	delete m_creator;
}

void MultiProcessorItem::propertiesChanged(QString const& _newName)
{
	QString name = _newName;
	if (name.isEmpty())
		name = QString::number((long uint)(this));

	delete m_multiProcessor;
	delete m_processor;
	m_multiProcessor = new MultiProcessor(m_creator);
	m_processor = m_creator->newProcessor();
	m_multiProcessor->init(name, m_properties);
	m_processor->init(name, m_properties);
}

void MultiProcessorItem::setProperty(QString const& _key, QVariant const& _value)
{
	m_properties[_key] = _value;
	propertiesChanged();
}

bool MultiProcessorItem::connectYourself(ProcessorGroup& _g)
{
/*	m_multiProcessor->setGroup(_g);
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
	if (uint i = m_processor->redrawPeriod())
		m_timerId = startTimer(i);*/
	return true;
}

void MultiProcessorItem::disconnectYourself()
{
/*	if (m_processor->multi() && m_processor->knowMultiplicity())
		m_multiplicity = m_processor->multiplicity();
	else
		m_multiplicity = 0;
	rejig();
	foreach (QGraphicsItem* i, childItems())
		if (OutputItem* ii = qgraphicsitem_cast<OutputItem*>(i))
			ii->setInputItem();
	m_processor->disconnectAll();
	m_processor->setNoGroup();
	if (m_timerId > -1)
		this->killTimer(m_timerId);*/
}

QRectF MultiProcessorItem::clientArea() const
{
	return QRectF();//QPointF(cornerSize, cornerSize), QSizeF(m_size.width() - 2 * cornerSize, m_size.height() - cornerSize - (statusHeight + 2 * statusMargin)));
}

QRectF MultiProcessorItem::boundingRect() const
{
	return QRectF();//QPointF(-10, -10), m_size + QSize(20.f, 20.f));
}

void MultiProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
/*	_p->setClipping(false);
	if (isSelected())
	{
		_p->setPen(QPen(QColor::fromHsv(220, 220, 200), 0));
		_p->setBrush(QBrush(QColor::fromHsv(220, 220, 200, 64)));
		_p->drawRoundedRect(QRectF(QPointF(-8, -8), m_size + QSize(16.f, 16.f)), 3, 3);
	}

	//_p->setPen(QPen(QColor::fromHsv(120, 96, 80, 255), 0));
	_p->setPen(QPen(Qt::black, 0));
	if (m_processor)
		_p->setBrush(m_processor->outlineColour());
	_p->drawRect(QRectF(QPointF(0.0, 0.0), m_size));

	_p->setPen(QPen(QColor(0, 0, 0, 64), 1));
	for (int i = 0; i < 4; i++)
	{
		double mp = cornerSize * 2.f * (4.0 - i) / 4.0;
		_p->drawLine(mp, m_size.height(), 0, m_size.height() - mp);
		_p->drawLine(mp, 0, 0, mp);
		_p->drawLine(m_size.width(), mp, m_size.width() - mp, 0);
	}
	_p->setPen(QPen(QColor(0, 0, 0, 128), 1));
	for (int i = 0; i < 4; i++)
	{
		double mp = (statusHeight + cornerSize) * (4.0 - i) / 4.0;
		_p->drawLine(m_size.width() - statusMargin, m_size.height() - mp, m_size.width() - mp, m_size.height() - statusMargin);
	}

	QRectF ca = clientArea();
	_p->setPen(Qt::NoPen);
	_p->setBrush(QColor(224, 224, 224));
	_p->drawRect(ca);

//	_p->setClipping(true);
//	_p->setClipRect(ca, Qt::IntersectClip);
	_p->translate(ca.topLeft());
	if (m_processor)
		m_processor->draw(*_p, ca.size());*/
}

void MultiProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
/*	Properties p;
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "property")
			p[n.toElement().attribute("name")] = n.toElement().attribute("value");
	ProcessorItem* pi = new ProcessorItem(ProcessorFactory::create(_element.attribute("type")), p, _element.attribute("name"), QSizeF(_element.attribute("w").toDouble(), _element.attribute("h").toDouble()));
	_scene->addItem(pi);
	pi->setPos(_element.attribute("x").toDouble(), _element.attribute("y").toDouble());*/
}

QDomElement MultiProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const
{
	QDomElement proc = _doc.createElement(_n);
/*	proc.setAttribute("type", m_processor->type());

	proc.setAttribute("name", m_processor->name());
	proc.setAttribute("x", pos().x());
	proc.setAttribute("y", pos().y());
	proc.setAttribute("w", m_size.width());
	proc.setAttribute("h", m_size.height());

	foreach (QString k, m_properties.keys())
	{
		QDomElement prop = _doc.createElement("property");
		proc.appendChild(prop);
		prop.setAttribute("name", k);
		prop.setAttribute("value", m_properties[k].toString());
	}
	_root.appendChild(proc);
*/
	return proc;
}

#if 0
ProcessorItem::ProcessorItem(Processor* _p, Properties const& _pr, QString const& _name, QSizeF const& _size): QGraphicsItem(), m_properties(_pr), m_processor(_p), m_size(_size), m_timerId(-1), m_resizing(false), m_multiplicity(0)
{
	m_statusBar = new QGraphicsRectItem(this);
	m_statusBar->setPen(Qt::NoPen);
	m_statusBar->setBrush(QColor(255, 255, 255, 16));

	m_pauseItem = new PauseItem(this);
	m_pauseItem->setPos(0, 0);

	setAcceptHoverEvents(true);
	setFlags(ItemClipsToShape | ItemIsFocusable | ItemIsSelectable | ItemIsMovable);

	if (m_processor)
	{
		m_properties.defaultFrom(m_processor->properties());
		m_processor->init(_name.isEmpty() ? QString::number((long uint)(this)) : _name, m_properties);
		rejig();
	}
	setPos(round(pos().x()) - .5f, round(pos().y()) - .5f);
}

void ProcessorItem::mousePressEvent(QGraphicsSceneMouseEvent* _e)
{
	if (_e->modifiers() & Qt::ShiftModifier)
		qobject_cast<ProcessorsScene*>(scene())->beginMultipleConnect(this);
	else
	{
		m_resizing = QRectF(m_size.width(), m_size.height(), -(statusHeight + cornerSize), -(statusHeight + cornerSize)).contains(_e->pos());
		if (m_resizing)
			m_origPosition = QPointF(m_size.width(), m_size.height()) - _e->pos();
		QGraphicsItem::mousePressEvent(_e);
	}
}

void ProcessorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
{
	m_resizing = false;
	QGraphicsItem::mouseReleaseEvent(_e);
	setPos(floor(pos().x()) + .5f, floor(pos().y()) + .5f);
}

void ProcessorItem::hoverMoveEvent(QGraphicsSceneHoverEvent* _e)
{
	if (QRectF(m_size.width(), m_size.height(), -(statusHeight + cornerSize), -(statusHeight + cornerSize)).contains(_e->pos()))
		setCursor(Qt::SizeFDiagCursor);
	else
		setCursor(Qt::ArrowCursor);
}

void ProcessorItem::mouseMoveEvent(QGraphicsSceneMouseEvent* _e)
{
	if (filter<IncompleteMultipleConnectionItem>(childItems()).count())
		return;
	if (m_resizing)
	{
		QPointF d = _e->pos() + m_origPosition;
		prepareGeometryChange();
		m_size = QSizeF(d.x(), d.y());
	}
	else
		QGraphicsItem::mouseMoveEvent(_e);

	QPointF best = QPointF(1.0e99, 1.0e99);
	foreach (QGraphicsItem* i, scene()->items())
	{
		QList<QPointF> was;
		if (ConnectionItem* ci = qgraphicsitem_cast<ConnectionItem*>(i))
		{
			if ((ci->toProcessor() == this || ci->fromProcessor() == this) && !m_resizing)
				was << (ci->toProcessor() == this ? 1 : -1) * ci->wouldAdjust();
		}
		else if (ProcessorItem* pi = qgraphicsitem_cast<ProcessorItem*>(i))
		{
			if (pi != this)
			{
				if (!m_resizing)
				{
					was << QPointF(pi->pos().x() - pos().x(), pi->pos().y() - pos().y());																				// their left, my left.
					was << QPointF(pi->pos().x() + pi->m_size.width() - pos().x(), pi->pos().y() + pi->m_size.height() - pos().y());									// their right, my left.
				}
				was << QPointF(pi->pos().x() + pi->m_size.width() - pos().x() - m_size.width(), pi->pos().y() + pi->m_size.height() - pos().y() - m_size.height());	// their right, my right.
				was << QPointF(pi->pos().x() - pos().x() - m_size.width(), pi->pos().y() - pos().y() - m_size.height());											// their left, my right.
			}
		}
		foreach (QPointF wa, was)
		{
			if (fabs(best.x()) > fabs(wa.x()))
				best.setX(wa.x());
			if (fabs(best.y()) > fabs(wa.y()))
				best.setY(wa.y());
		}
	}

	if (m_resizing)
	{
		if (fabs(best.x()) < 5)
			m_size = m_size + QSizeF(best.x(), 0);
		if (fabs(best.y()) < 5)
			m_size = m_size + QSizeF(0, best.y());
		rejig();
	}
	else
	{
		if (fabs(best.x()) < 5)
			setPos(pos() + QPointF(best.x(), 0));
		if (fabs(best.y()) < 5)
			setPos(pos() + QPointF(0, best.y()));
	}

	foreach (QGraphicsItem* i, scene()->items())
		if (ConnectionItem* ci = qgraphicsitem_cast<ConnectionItem*>(i))
		{	if (ci->toProcessor() == this || ci->fromProcessor() == this)
				ci->rejigEndPoints();
		}
		else if (MultipleConnectionItem* mci = qgraphicsitem_cast<MultipleConnectionItem*>(i))
			if (mci->toProcessor() == this || mci->fromProcessor() == this)
				mci->rejigEndPoints();
}

void ProcessorItem::tick()
{
	m_pauseItem->tick();
	foreach (QGraphicsItem* i, childItems())
		if (InputItem* ii = qgraphicsitem_cast<InputItem*>(i))
			ii->update();
}

void ProcessorItem::timerEvent(QTimerEvent*)
{
	update(clientArea().adjusted(2, 2, -2, -2));
}

void ProcessorItem::typesConfirmed()
{
	foreach (QGraphicsItem* i, childItems())
		if (InputItem* ii = qgraphicsitem_cast<InputItem*>(i))
			ii->typesConfirmed();
}

Processor* ProcessorItem::reconstructProcessor()
{
	return ProcessorFactory::create(m_processor->type());
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

	m_processor->init(_newName.isEmpty() ? old ? old->name() : QString::number((long uint)this) : _newName, completeProperties());
	rejig();
	delete old;
}

void ProcessorItem::rejig()
{
	double minHeight = cornerSize + cornerSize / 2 + portLateralMargin + max(m_processor->numInputs(), m_processor->numOutputs()) * (portLateralMargin + portSize) + cornerSize / 2 + cornerSize;
	minHeight = max(minHeight, cornerSize + centreMin().height() + statusHeight + statusMargin * 2);
	double minWidth = cornerSize * 2 + centreMin().width();
	prepareGeometryChange();
	m_size = QSizeF(max(m_size.width(), minWidth), max(m_size.height(), minHeight));
	update();

	QVector<InputItem*> iis(m_processor->multi() & In ? m_multiplicity : m_processor->numInputs(), 0);
	foreach (InputItem* ii, filter<InputItem>(childItems()))
		if ((int)ii->index() < iis.count())
			iis[ii->index()] = ii;
		else
			delete ii;
	for (int i = 0; i < iis.count(); i++)
		if (!iis[i])
			iis[i] = new InputItem(i, this);
	foreach (InputItem* i, iis)
		i->setPos(cornerSize - i->size().width() - 1.f, cornerSize + cornerSize / 2 + portLateralMargin + (portLateralMargin + portSize) * i->index());

	QVector<OutputItem*> ois(m_processor->multi() & Out ? m_multiplicity : m_processor->numOutputs(), 0);
	if (ois.count() == 0 && m_processor->multi())
{
	qDebug() << "BP";
}
	foreach (OutputItem* oi, filter<OutputItem>(childItems()))
		if ((int)oi->index() < ois.count())
			ois[oi->index()] = oi;
		else
			delete oi;
	for (int i = 0; i < ois.count(); i++)
		if (!ois[i])
			ois[i] = new OutputItem(i, this);
	foreach (OutputItem* i, ois)
		i->setPos(m_size.width() - cornerSize + 1.f, cornerSize * 3 / 2 + portLateralMargin / 2 + (portLateralMargin + portSize) * (i->index() + 0.5));

	m_statusBar->setPos(cornerSize * 2, m_size.height() - statusHeight - statusMargin);
	m_statusBar->setRect(QRectF(0, 0, m_size.width() - cornerSize * 4, statusHeight));
	update();
}

void ProcessorItem::focusInEvent(QFocusEvent* _e)
{
	foreach (QGraphicsItem* i, scene()->selectedItems())
		if (i != this)
			i->setSelected(false);
	setSelected(true);
	assert(isSelected());
	update();
	QGraphicsItem::focusInEvent(_e);
}
#endif
