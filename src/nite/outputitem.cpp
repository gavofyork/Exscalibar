#include "processorsview.h"
#include "processoritem.h"
#include "outputitem.h"

OutputItem::OutputItem(int _i, ProcessorItem* _p): QGraphicsItem(_p), m_index(_i), m_hover(false), m_inputItem(0)
{
	setCursor(Qt::CrossCursor);
	setAcceptHoverEvents(true);
}

QPolygonF OutputItem::polygon() const
{
	QPolygonF p;
	double psot = portSize / 2 * (m_hover ? 2 : 1);
	double cs = cornerSize * (m_hover ? 2 : 1);
	p.append(QPointF(0, -psot));
	p.append(QPointF(0, psot));
	p.append(QPointF(cs, psot));
	p.append(QPointF(cs + psot, 0));
	p.append(QPointF(cs, -psot));
	return p;
}

QPointF OutputItem::tip() const
{
	double psot = portSize / 2 * (m_hover ? 2 : 1);
	double cs = cornerSize * (m_hover ? 2 : 1);
	return QPointF(cs + psot, 0);
}

void OutputItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->setPen(QPen(Qt::black, 0));
	_p->setBrush(QColor::fromHsv(40, 128, 220, 255));
	_p->drawPolygon(polygon());
}

void OutputItem::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	prepareGeometryChange();
	m_hover = true;
	update();
}

void OutputItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	prepareGeometryChange();
	m_hover = false;
	update();
}

void OutputItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
	qobject_cast<ProcessorsScene*>(scene())->beginConnect(this);
}

ProcessorItem* OutputItem::processorItem() const
{
	return qgraphicsitem_cast<ProcessorItem*>(parentItem());
}
