#include "connectionitem.h"
#include "processorsview.h"
#include "processoritem.h"

class PauseItem: public QGraphicsEllipseItem
{
public:
	PauseItem(ProcessorItem* _p): QGraphicsEllipseItem(_p->statusBar())
	{
		setPen(Qt::NoPen);
		setBrush(Qt::NoBrush);
		setRect(QRectF(0, 0, statusHeight, statusHeight));
	}

	virtual void paint(QPainter* _p, QStyleOptionGraphicsItem const* _o, QWidget* _w)
	{
		_p->save();
		QGraphicsEllipseItem::paint(_p, _o, _w);
		_p->restore();
		if (processor() && processor()->isRunning())
		{
			_p->setPen(QPen(QColor(0, 0, 0, 32), 2));
			_p->translate(QPointF(statusHeight / 2, statusHeight / 2));
			_p->rotate((processor()->guardsCrossed() % 36) * 360.0 / 36);
			_p->drawPoint(QPointF(statusHeight / 2, 0));
			_p->setPen(QPen(QColor(0, 0, 0, 64), 2));
			_p->rotate(360.0 / 12);
			_p->drawPoint(QPointF(statusHeight / 2, 0));
		}
	}
	Processor* processor() const { return qgraphicsitem_cast<ProcessorItem*>(parentItem()->parentItem())->processor(); }
	virtual void mousePressEvent(QGraphicsSceneMouseEvent*)
	{
		Processor* p = processor();
		if (p->isRunning())
		{
			if (p->paused())
			{
				setBrush(Qt::NoBrush);
				p->unpause();
			}
			else
			{
				setBrush(Qt::red);
				p->pause();
			}
			update();
		}
	}

	void tick()
	{
		update();
	}

	enum { Type = UserType + 6 };
	virtual int type() const { return Type; }
};

DomProcessorItem::DomProcessorItem(Properties const& _pr, QString const& _name, QSizeF const& _size):
	ProcessorItem(0, _pr, _name, _size)
{
}

DomProcessor* DomProcessorItem::domProcessor() const { return dynamic_cast<DomProcessor*>(processor()); }

QSizeF DomProcessorItem::centreMin() const
{
	QSizeF s(0, 0);
	foreach (SubProcessorItem* i, filter<SubProcessorItem>(childItems()))
		s = QSizeF(s.width() + i->size().width(), max(s.height(), i->size().height()));
	return s;
}

QString DomProcessorItem::composedSubs() const
{
	QString ret;
	foreach (SubProcessorItem* i, ordered())
		ret += "&" + i->spType();
	return ret.mid(1);
}

Properties DomProcessorItem::completeProperties() const
{
	Properties ret;
	QList<SubProcessorItem*> spis = ordered();
	for (int i = (uint)spis.count() - 1; i >= 0; i--)
		ret = ret.stashed() + spis[i]->properties();
	ret = ret.stashed() + properties();
	return ret;
}

Processor* DomProcessorItem::reconstructProcessor()
{
	QString cs = composedSubs();
	if (cs.isEmpty())
		return 0;
	Processor* p = new DomProcessor(cs);
	PropertiesInfo pi = p->properties();
	m_properties.defaultFrom(pi.destash());
	foreach (SubProcessorItem* i, ordered())
		i->m_properties.defaultFrom(pi.destash());
	return p;
}

void DomProcessorItem::reorder() const
{
	QList<SubProcessorItem*> spis = filter<SubProcessorItem>(childItems());

	uint oc = (uint)spis.count();
	for (uint i = 0; i < oc; i++)
	{
		SubProcessorItem* spi;
		for (uint j = i;; j++)
			foreach (spi, spis)
				if (spi->index() <= j)
				{
					spis.removeAll(spi);
					spi->m_index = i;
					goto OK;
				}
		break;
		OK: ;
	}
}

QList<SubProcessorItem*> DomProcessorItem::ordered() const
{
	QList<SubProcessorItem*> ret;
	QList<SubProcessorItem*> spis = filter<SubProcessorItem>(childItems());

	for (uint i = 0; i < (uint)spis.count(); i++)
	{
		SubProcessorItem* spi;
		foreach (spi, spis)
			if (spi->index() == i)
				goto OK;
		break;
		assert("Subprocessors out of order");
		OK:
		ret << spi;
	}
	return ret;
}

void DomProcessorItem::rejig(Processor* _old, bool _bootStrap)
{
	QPointF cp = clientArea().topLeft();
	foreach (SubProcessorItem* spi, ordered())
	{
		spi->setPos(cp);
		cp += QPointF(spi->size().width(), 0);
	}
	ProcessorItem::rejig(_old, _bootStrap);
}

SubProcessorItem::SubProcessorItem(DomProcessorItem* _dpi, QString const& _type, int _index, Properties const& _pr):
	QGraphicsItem	(_dpi),
	m_properties	(_pr),
	m_type			(_type),
	m_index			(_index)
{
	_dpi->reorder();
	_dpi->propertiesChanged();
	setFlags(ItemClipsToShape | ItemIsFocusable | ItemIsSelectable);
}

SubProcessor* SubProcessorItem::subProcessor() const
{
	QList<SubProcessor*> cs;
	if (!domProcessor())
		return 0;
	SubProcessor* cur = domProcessor()->primary();
	int i = m_index;
	forever
	{
		if (Combination* c = dynamic_cast<Combination*>(cur))
		{
			cs.append(c->y());
			cs.append(c->x());
		}
		else
			if (i)
				i--;
			else
				return cur;
		if (cs.isEmpty())
			return 0;
		cur = cs.takeLast();
	}
}

void DomProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem* _o, QWidget* _w)
{
	QPainterPath p;
	p.addRect(boundingRect());
	foreach (SubProcessorItem* spi, filter<SubProcessorItem>(childItems()))
		p.addRect(QRectF(spi->pos(), spi->size()));
	_p->save();
	_p->setClipPath(p);
	ProcessorItem::paint(_p, _o, _w);
	_p->restore();
}

void SubProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	QRectF ca = QRectF(QPointF(0, 0), size());
	_p->save();
	_p->setClipRect(ca);
	subProcessor()->draw(*_p);
	_p->restore();

	if (isSelected())
	{
		_p->setBrush(Qt::NoBrush);
		for (int i = 0; i < 5; i+=2)
		{
			_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), i));
			_p->drawRect(ca);
		}
	}
}

void SubProcessorItem::setProperty(QString const& _key, QVariant const& _value)
{
	m_properties[_key] = _value;
	prepareGeometryChange();
	domProcessorItem()->propertiesChanged();
	update();
}

void SubProcessorItem::focusInEvent(QFocusEvent* _e)
{
	foreach (QGraphicsItem* i, scene()->selectedItems())
		if (i != this)
			i->setSelected(false);
	setSelected(true);
	assert(isSelected());
	update();
	QGraphicsItem::focusInEvent(_e);
}

void SubProcessorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
{
	domProcessorItem()->mouseReleaseEvent(_e);
	domProcessorItem()->setSelected(false);
	QGraphicsItem::mouseReleaseEvent(_e);
}

void SubProcessorItem::mousePressEvent(QGraphicsSceneMouseEvent* _e)
{
	domProcessorItem()->mousePressEvent(_e);
	domProcessorItem()->setSelected(false);
	QGraphicsItem::mousePressEvent(_e);
}

void SubProcessorItem::mouseMoveEvent(QGraphicsSceneMouseEvent* _e)
{
	domProcessorItem()->mouseMoveEvent(_e);
}

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
		m_processor->init(_name.isEmpty() ? QString::number(uint(this)) : _name, m_properties);
		rejig(0, true);
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

	m_processor->init(_newName.isEmpty() ? old ? old->name() : QString::number((uint)this) : _newName, completeProperties());
	rejig(old, !old);
	delete old;
}

void ProcessorItem::rejig(Processor* _old, bool _bootStrap)
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

void ProcessorItem::setProperty(QString const& _key, QVariant const& _value)
{
	m_properties[_key] = _value;
	propertiesChanged();
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
	if (uint i = m_processor->redrawPeriod())
		m_timerId = startTimer(i);
	return true;
}

void ProcessorItem::disconnectYourself()
{
	if (m_processor->multi() && m_processor->knowMultiplicity())
		m_multiplicity = m_processor->multiplicity();
	else
		m_multiplicity = 0;
	rejig(0, false);
	foreach (QGraphicsItem* i, childItems())
		if (OutputItem* ii = qgraphicsitem_cast<OutputItem*>(i))
			ii->setInputItem();
	m_processor->disconnectAll();
	m_processor->setNoGroup();
	if (m_timerId > -1)
		this->killTimer(m_timerId);
}

QRectF ProcessorItem::clientArea() const
{
	return QRectF(QPointF(cornerSize, cornerSize), QSizeF(m_size.width() - 2 * cornerSize, m_size.height() - cornerSize - (statusHeight + 2 * statusMargin)));
}

void ProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->setClipping(false);
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
		m_processor->draw(*_p, ca.size());
}

void ProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	Properties p;
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "property")
			p[n.toElement().attribute("name")] = n.toElement().attribute("value");
	ProcessorItem* pi = new ProcessorItem(ProcessorFactory::create(_element.attribute("type")), p, _element.attribute("name"), QSizeF(_element.attribute("w").toDouble(), _element.attribute("h").toDouble()));
	_scene->addItem(pi);
	pi->setPos(_element.attribute("x").toDouble(), _element.attribute("y").toDouble());
}

QDomElement ProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n) const
{
	QDomElement proc = _doc.createElement(_n);
	proc.setAttribute("type", m_processor->type());

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
	return proc;
}

void DomProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	Properties p;
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "property")
			p[n.toElement().attribute("name")] = n.toElement().attribute("value");
	DomProcessorItem* dpi = new DomProcessorItem(p, _element.attribute("name"), QSizeF(_element.attribute("w").toDouble(), _element.attribute("h").toDouble()));
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "subprocessor")
			SubProcessorItem::fromDom(n.toElement(), dpi);
	dpi->propertiesChanged(_element.attribute("name"));
	_scene->addItem(dpi);
	dpi->setPos(_element.attribute("x").toDouble(), _element.attribute("y").toDouble());
}

QDomElement DomProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc, QString const&) const
{
	QDomElement us = ProcessorItem::saveYourself(_root, _doc, "domprocessor");
	foreach (SubProcessorItem* spi, filter<SubProcessorItem>(childItems()))
		spi->saveYourself(us, _doc);
	return us;
}

void SubProcessorItem::fromDom(QDomElement const& _element, DomProcessorItem* _dpi)
{
	Properties p;
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "property")
			p[n.toElement().attribute("name")] = n.toElement().attribute("value");
	new SubProcessorItem(_dpi, _element.attribute("type"), _element.attribute("index").toInt(), p);
}

void SubProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement proc = _doc.createElement("subprocessor");
	proc.setAttribute("type", m_type);
	proc.setAttribute("index", m_index);
	foreach (QString k, m_properties.keys())
	{
		QDomElement prop = _doc.createElement("property");
		proc.appendChild(prop);
		prop.setAttribute("name", k);
		prop.setAttribute("value", m_properties[k].toString());
	}
	_root.appendChild(proc);
}
