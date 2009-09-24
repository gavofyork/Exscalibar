#include <Geddei>

#include "connectionitem.h"
#include "processoritem.h"
#include "inputitem.h"

InputItem::InputItem(int _i, ProcessorItem* _p): QGraphicsItem(_p), m_index(_i)
{
	m_size = QSizeF(10 + cornerSize, portSize);
	setPos(cornerSize - m_size.width() - 1, cornerSize + cornerSize / 2 + portLateralMargin + (portLateralMargin + portSize) * m_index);
}

ProcessorItem* InputItem::processorItem() const
{
	return qgraphicsitem_cast<ProcessorItem*>(parentItem());
}

QRectF InputItem::boundingRect() const
{
	return QRectF(QPointF(0.0, 0.0), m_size);
}

QPointF InputItem::tip() const
{
	return QPointF(portSize / 2, portSize / 2);
}

void InputItem::typesConfirmed()
{
	prepareGeometryChange();
	double secs = processorItem()->m_processor->input(m_index).capacity() / processorItem()->m_processor->input(m_index).type().frequency();
	m_size = QSizeF(log(1.0 + secs) / log(2) * 16 + 10 + cornerSize, portSize);
	update();
	foreach (QGraphicsItem* i, childItems())
		if (ConnectionItem* ci = qgraphicsitem_cast<ConnectionItem*>(i))
			ci->rejigEndPoints();
}

void InputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->setPen(QPen(Qt::black, 0));
	_p->setBrush(QColor::fromHsv(40, 128, 220, 255));
	QPolygonF p;
	p.append(QPointF(0, 0));
	p.append(QPointF(portSize / 2, portSize / 2));
	p.append(QPointF(0, portSize));
	p.append(QPointF(m_size.width(), portSize));
	p.append(QPointF(m_size.width(), 0));
	_p->drawPolygon(p);

	if (processorItem()->m_processor->isRunning())	//  a bit unsafe, since the processor could stop & get reset between these two.
	{
		double fill = processorItem()->m_processor->input(m_index).filled();
		_p->fillRect(QRectF(cornerSize + (m_size.width() - cornerSize) * (1.f - fill), portSize / 4, (m_size.width() - cornerSize) * fill, portSize / 2), QColor(Qt::darkRed));
	}
}
