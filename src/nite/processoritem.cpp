#include "connectionitem.h"
#include "processorsview.h"
#include "processoritem.h"

ProcessorItem::ProcessorItem(Processor* _p, Properties const& _pr): QGraphicsItem(), m_processor(_p), m_properties(_pr), m_size(0, 0)
{
	foreach (QString s, m_processor->properties().keys())
		if (!m_properties.keys().contains(s))
			m_properties[s] = m_processor->properties().defaultValue(s);
	propertiesChanged();
	setFlags(ItemClipsToShape | ItemIsFocusable | ItemIsSelectable | ItemIsMovable);
}

ProcessorItem::ProcessorItem(Processor* _p, Properties const& _pr, QString const& _name): QGraphicsItem(), m_processor(_p), m_properties(_pr)
{
	m_processor->init(_name, m_properties);
	m_size = QSizeF(100, 100);
}

void ProcessorItem::mouseMoveEvent(QGraphicsSceneMouseEvent* _e)
{
	QGraphicsItem::mouseMoveEvent(_e);
	foreach (QGraphicsItem* i, scene()->items())
		if (ConnectionItem* ci = qgraphicsitem_cast<ConnectionItem*>(i))
			if (ci->toProcessor() == this || ci->fromProcessor() == this)
				ci->rejigEndPoints();
}

void ProcessorItem::propertiesChanged()
{
	if (m_processor->isRunning())
		return;
	Processor* old = m_processor;
	m_processor = ProcessorFactory::create(old->type());
	if (!m_processor)
	{
		m_processor = old;
		return;
	}

	m_processor->init(QString::number(uint(this)), m_properties);

	double minHeight = cornerSize + cornerSize / 2 + portLateralMargin + max(m_processor->numInputs(), m_processor->numOutputs()) * (portLateralMargin + portSize) + cornerSize / 2 + cornerSize;
	minHeight = max(minHeight, cornerSize * 2 + m_processor->height());
	double minWidth = cornerSize * 2 + m_processor->width();
	m_size = QSizeF(max(m_size.width(), minWidth), max(m_size.height(), minHeight));

	foreach (QGraphicsItem* i, childItems())
		if ((qgraphicsitem_cast<InputItem*>(i) && qgraphicsitem_cast<InputItem*>(i)->index() >= m_processor->numInputs()) ||
			(qgraphicsitem_cast<OutputItem*>(i) && qgraphicsitem_cast<OutputItem*>(i)->index() >= m_processor->numOutputs()))
			delete i;
	for (uint i = old->numInputs(); i < m_processor->numInputs(); i++)
		new InputItem(i, this);
	for (uint i = old->numOutputs(); i < m_processor->numOutputs(); i++)
		new OutputItem(i, this);

	delete old;
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
	foreach (QGraphicsItem* i, childItems())
		if (InputItem* ii = qgraphicsitem_cast<InputItem*>(i))
			foreach (QGraphicsItem* j, ii->childItems())
				if (ConnectionItem* ci = qgraphicsitem_cast<ConnectionItem*>(j))
					if (!ci->from()->processorItem()->m_processor->connect(ci->from()->index(), m_processor, ii->index()))
						return false;
	return true;
}

void ProcessorItem::disconnectYourself()
{
	m_processor->disconnectAll();
}

void ProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (isSelected())
	{
		_p->setPen(QPen(QColor::fromHsv(220, 220, 200), 0));
		_p->setBrush(QBrush(QColor::fromHsv(220, 220, 200, 64)));
		_p->drawRoundedRect(QRectF(QPointF(-8, -8), m_size + QSize(16.f, 16.f)), 3, 3);
	}

	//_p->setPen(QPen(QColor::fromHsv(120, 96, 80, 255), 0));
	_p->setPen(QPen(Qt::black, 0));
	_p->setBrush(QColor::fromHsv(120, 96, 160, 255));
	_p->drawRect(QRectF(QPointF(0.0, 0.0), m_size));

	_p->setPen(QPen(QColor(0, 0, 0, 64), 1));
	for (int i = 0; i < 4; i++)
	{
		double mp = cornerSize * 2.f * (4.0 - i) / 4.0;
		_p->drawLine(mp, m_size.height(), 0, m_size.height() - mp);
		_p->drawLine(mp, 0, 0, mp);
		_p->drawLine(m_size.width(), mp, m_size.width() - mp, 0);
		_p->drawLine(m_size.width(), m_size.height() - mp, m_size.width() - mp, m_size.height());
	}

	QRectF clientArea = QRectF(QPointF(cornerSize, cornerSize), m_size - QSizeF(2, 2) * cornerSize);
	_p->setPen(Qt::NoPen);
	_p->setBrush(QColor(0, 0, 0, 128));
	_p->drawRect(QRectF(QPointF(cornerSize, cornerSize), m_size - QSizeF(2, 2) * cornerSize));

	_p->setClipRect(clientArea);
	_p->translate(clientArea.topLeft());
	m_processor->draw(*_p, clientArea.size());
}

void ProcessorItem::fromDom(QDomElement& _element, QGraphicsScene* _scene)
{
	Properties p;
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "property")
			p[n.toElement().attribute("name")] = n.toElement().attribute("value");
	ProcessorItem* pi = new ProcessorItem(ProcessorFactory::create(_element.attribute("type")), p, _element.attribute("name"));
	pi->loadYourself(_element);
	_scene->addItem(pi);
}

void ProcessorItem::loadYourself(QDomElement& _element)
{
	setPos(_element.attribute("x").toDouble(), _element.attribute("y").toDouble());
	m_size = QSizeF(_element.attribute("w").toDouble(), _element.attribute("h").toDouble());
	for (QDomNode n = _element.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "input")
			InputItem::fromDom(n.toElement(), this);
}

void ProcessorItem::saveYourself(QDomElement& _root, QDomDocument& _doc) const
{
	QDomElement proc = _doc.createElement("processor");
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
	foreach (QGraphicsItem* i, childItems())
		if (InputItem* ii = qgraphicsitem_cast<InputItem*>(i))
			ii->saveYourself(proc, _doc);
	_root.appendChild(proc);
}
