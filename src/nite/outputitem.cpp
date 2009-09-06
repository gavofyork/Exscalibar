#include "processoritem.h"
#include "outputitem.h"

OutputItem::OutputItem(int _i, ProcessorItem* _p): QGraphicsPolygonItem(_p), m_index(_i)
{
	setPen(QPen(Qt::black, 0));
	setBrush(QColor::fromHsv(40, 128, 220, 255));
	QPolygonF p;
	p.append(QPointF(0, 0));
	p.append(QPointF(0, portSize));
	p.append(QPointF(cornerSize, portSize));
	p.append(QPointF(cornerSize + portSize / 2, portSize / 2));
	p.append(QPointF(cornerSize, 0));
	setPolygon(p);
	setPos(_p->m_size.width() - cornerSize, cornerSize + cornerSize / 2 + portLateralMargin + (portLateralMargin + portSize) * m_index);
}

ProcessorItem* OutputItem::processorItem() const { return qgraphicsitem_cast<ProcessorItem*>(parentItem()); }
