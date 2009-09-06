#include <Geddei>

#include "processoritem.h"
#include "inputitem.h"

InputItem::InputItem(int _i, ProcessorItem* _p): QGraphicsItem(_p), m_index(_i)
{
	double secs = 0.1;
	if (_p->m_processor->isRunning())
	{
		double f = _p->m_processor->input(m_index).type().frequency();
		secs = _p->m_processor->input(m_index).capacity() / f;
	}
	m_size = QSizeF(secs * 100 + cornerSize, portSize);
	setPos(cornerSize - m_size.width(), cornerSize + cornerSize / 2 + portLateralMargin + (portLateralMargin + portSize) * m_index);
}

ProcessorItem* InputItem::processorItem() const
{
	return qgraphicsitem_cast<ProcessorItem*>(parentItem());
}

QRectF InputItem::boundingRect() const
{
	return QRectF(QPointF(0.0, 0.0), m_size);
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

	if (processorItem()->m_processor->isRunning())
	{
		double f = processorItem()->m_processor->input(m_index).type().frequency();
		double cap = processorItem()->m_processor->input(m_index).filled() / f;
		_p->fillRect(QRectF(m_size.width() - cap * 100, portSize / 4, m_size.width(), portSize * 3 / 4), QColor(Qt::darkRed));
	}
}
