#include "processoritem.h"

ProcessorItem::ProcessorItem(Processor* _p, Properties const& _pr): QGraphicsItem(), m_processor(_p), m_properties(_pr)
{
	m_processor->init(QString::number(uint(this)), m_properties);
	double minHeight = cornerSize + cornerSize / 2 + portLateralMargin + max(m_processor->numInputs(), m_processor->numOutputs()) * (portLateralMargin + portSize) + cornerSize / 2 + cornerSize;
	minHeight = max(minHeight, cornerSize * 2 + m_processor->height());
	double minWidth = cornerSize * 3 + m_processor->width();
	m_size = QSizeF(minWidth, minHeight);
	for (uint i = 0; i < m_processor->numInputs(); i++)
		new InputItem(i, this);
	for (uint i = 0; i < m_processor->numOutputs(); i++)
		new OutputItem(i, this);
	setFlags(ItemClipsToShape | ItemIsFocusable | ItemIsMovable);
}

ProcessorItem::ProcessorItem(Processor* _p, Properties const& _pr, QString const& _name): QGraphicsItem(), m_processor(_p), m_properties(_pr)
{
	m_processor->init(_name, m_properties);
	m_size = QSizeF(100, 100);
	qDebug() << m_processor->numInputs() << m_processor->numOutputs();
}

void ProcessorItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	if (hasFocus())
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

	_p->setPen(Qt::NoPen);
	_p->setBrush(QColor(0, 0, 0, 128));
	_p->drawRect(QRectF(QPointF(cornerSize, cornerSize), m_size - QSizeF(2, 2) * cornerSize));
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
