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

#include "GeddeiNite.h"
#include "PauseItem.h"
#include "BaseItem.h"

#include "Magnetic.h"
#include "InputItem.h"
#include "OutputItem.h"

static const double cornerSize = 4.0;
static const double statusHeight = 8.0;
static const double statusMargin = 2.0;

BaseItem::BaseItem(Properties const& _pr, QSizeF const& _size):
	m_size			(_size),
	m_timerId		(-1),
	m_resizing		(false),
	m_properties	(_pr)
{
	m_statusBar = new QGraphicsRectItem(this);
	m_statusBar->setPen(Qt::NoPen);
	m_statusBar->setBrush(QColor(255, 255, 255, 16));

	m_pauseItem = new PauseItem(m_statusBar, this, statusHeight);
	m_pauseItem->setPos(0, 0);

	setAcceptHoverEvents(true);
	setFlags(ItemClipsToShape | ItemIsFocusable | ItemIsSelectable | ItemIsMovable);
}

BaseItem::~BaseItem()
{
}

void BaseItem::forwardEvent(QGraphicsSceneEvent* _e)
{
	switch (_e->type())
	{
		case QEvent::GraphicsSceneMouseMove: mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(_e)); break;
		case QEvent::GraphicsSceneMousePress: mousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(_e)); break;
		case QEvent::GraphicsSceneMouseRelease: mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(_e)); break;
		default:;
	}
}

void BaseItem::setProperty(QString const& _key, QVariant const& _value)
{
	m_properties[_key] = _value;
	propertiesChanged();
}

void BaseItem::setDefaultProperties(PropertiesInfo const& _def)
{
	m_properties.defaultFrom(_def);
}

void BaseItem::tick()
{
	m_pauseItem->tick();
}

void BaseItem::timerEvent(QTimerEvent*)
{
	update(centreRect().adjusted(2, 2, -2, -2));
}

void BaseItem::propertiesChanged(QString const&)
{
	geometryChanged();
}

void BaseItem::geometryChanged()
{
	prepareGeometryChange();
	if (!m_size.isValid())
		m_size = centrePref();
	m_size = QSizeF(max(centreMin().width(), m_size.width()), max(centreMin().height(), m_size.height()));
	m_statusBar->setPos(centreRect().bottomLeft() + QPointF(cornerSize * 2.f, statusMargin));
	m_statusBar->setRect(QRectF(0, 0, m_size.width() - cornerSize * 5.f, statusHeight));

	positionChanged();
}

float BaseItem::marginSize() const
{
	return 10.f;
}

QRectF BaseItem::boundingRect() const
{
	return adjustBounds(basicBoundingRect());
}

QRectF BaseItem::outlineRect() const
{
	// Add one to width & height as we translate up 7 left by half a pixel when drawing.
	return centreRect().adjusted(-cornerSize, -cornerSize, cornerSize + 1, statusHeight + 2 * statusMargin + 1);
}

QRectF BaseItem::resizeRect() const
{
	return QRectF(outlineRect().bottomRight(), QSizeF(-(statusHeight + cornerSize), -(statusHeight + cornerSize)));
}

void BaseItem::prepYourself(ProcessorGroup&)
{
}

bool BaseItem::connectYourself()
{
	return true;
}

void BaseItem::typesConfirmed()
{
	qDebug() << "Redraw period: " << redrawPeriod();
	if (uint i = redrawPeriod())
		m_timerId = startTimer(i);
}

void BaseItem::disconnectYourself()
{
	if (m_timerId > -1)
		this->killTimer(m_timerId);
}

void BaseItem::focusInEvent(QFocusEvent* _e)
{
	foreach (QGraphicsItem* i, scene()->selectedItems())
		if (i != this)
			i->setSelected(false);
	setSelected(true);
	assert(isSelected());
	update();
	QGraphicsItem::focusInEvent(_e);
}

void BaseItem::mousePressEvent(QGraphicsSceneMouseEvent* _e)
{
	m_resizing = resizeRect().contains(_e->pos());
	if (m_resizing)
		m_origPosition = QPointF(m_size.width(), m_size.height()) - _e->pos();
	else if (!outlineRect().contains(_e->pos()))
	{
		_e->setAccepted(false);
		return;
	}
	QGraphicsItem::mousePressEvent(_e);
}

void BaseItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
{
	m_resizing = false;
	QGraphicsItem::mouseReleaseEvent(_e);
}

void BaseItem::hoverMoveEvent(QGraphicsSceneHoverEvent* _e)
{
	if (resizeRect().contains(_e->pos()))
		setCursor(Qt::SizeFDiagCursor);
	else
		setCursor(Qt::ArrowCursor);
}

QList<QPointF> BaseItem::magnetism(BaseItem const* _b, bool _moving) const
{
	QList<QPointF> ret;
	if (_b == this && !_moving)
		ret << QPointF(centrePref().width() - centreRect().width(), centrePref().height() - centreRect().height());

	if (_b == this)
		return ret;

	QRectF us = outlineRect().translated(pos());
	QRectF them = _b->outlineRect().translated(_b->pos());

	if (_moving)
	{
		// our left, their left.
		ret << QPointF(us.left() - them.left(), us.top() - them.top());
		// our right, their left.
		ret << QPointF(us.right() - them.left(), us.bottom() - them.top());
	}
	// our left, their right.
	ret << QPointF(us.left() - them.right(), us.top() - them.bottom());
	// our right, their right.
	ret << QPointF(us.right() - them.right(), us.bottom() - them.bottom());
	return ret;
}

void BaseItem::mouseMoveEvent(QGraphicsSceneMouseEvent* _e)
{
	if (m_resizing)
	{
		prepareGeometryChange();
		QPointF d = _e->pos() + m_origPosition;
		m_size = QSizeF(d.x(), d.y());
	}
	else
		QGraphicsItem::mouseMoveEvent(_e);

	QPointF best = QPointF(1.0e99, 1.0e99);
	foreach (Magnetic const* m, filterRelaxed<Magnetic>(scene()->items()))
		foreach (QPointF wa, m->magnetism(this, !m_resizing))
		{
			if (fabs(best.x()) > fabs(wa.x()))
				best.setX(wa.x());
			if (fabs(best.y()) > fabs(wa.y()))
				best.setY(wa.y());
		}

	if (m_resizing)
	{
		if (fabs(best.x()) < 5)
			m_size = m_size + QSizeF(best.x(), 0);
		if (fabs(best.y()) < 5)
			m_size = m_size + QSizeF(0, best.y());
		m_size = QSizeF(max(centreMin().width(), m_size.width()), max(centreMin().height(), m_size.height()));
		geometryChanged();
	}
	else
	{
		if (fabs(best.x()) < 5)
			setPos(pos() + QPointF(best.x(), 0));
		if (fabs(best.y()) < 5)
			setPos(pos() + QPointF(0, best.y()));
		positionChanged();
	}
}

void BaseItem::paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*)
{
	_p->setClipRect(boundingRect());
	paintOutline(_p);
	_p->setClipRect(centreRect().adjusted(0, 0, .5f, .5f));
	_p->translate(.5f, .5f);
	paintCentre(_p);
}

void BaseItem::paintCentre(QPainter*)
{
}

QBrush BaseItem::fillBrush() const
{
	QLinearGradient cg(outlineRect().topLeft(), outlineRect().bottomLeft());
	cg.setColorAt(0, outlineColour().lighter(125));
	cg.setColorAt(1, outlineColour().darker(150));
	return QBrush(cg);
}
QPen BaseItem::innerPen() const
{
	QLinearGradient cg(outlineRect().topLeft(), outlineRect().bottomLeft());
	cg.setColorAt(0, outlineColour().lighter(175));
	cg.setColorAt(1, outlineColour().darker(150));
	return QPen(cg, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}
QPen BaseItem::outerPen() const
{
	QLinearGradient cg(outlineRect().topLeft(), outlineRect().bottomLeft());
	cg.setColorAt(0, outlineColour().darker(250));
	cg.setColorAt(1, outlineColour().darker(350));
	return QPen(cg, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}

void BaseItem::paintOutline(QPainter* _p)
{
	_p->save();
	{
		float cornerRadius = round(marginSize() * (isSelected() ? 1.25f : 1.25f));
		float adj = round(marginSize() / (isSelected() ? 4.f : 2.f));
		QRectF br = basicBoundingRect().adjusted(adj, adj, -adj, -adj);
		GeddeiNite* gn = qobject_cast<GeddeiNite*>(scene()->parent());
		QColor shadow = gn->myColour(primaryTask()).darker(isSelected() ? 400 : 300);

		_p->translate(-0.5f, -0.5f + round(marginSize() / (isSelected() ? 5.f : 6.f)));
		{
			QRadialGradient cg(0, 0, cornerRadius, 0, 0);
			cg.setColorAt(0.f, shadow);
			cg.setColorAt(1.f, Qt::transparent);
			#define CORNER(name, xs, ys) \
			{ \
				cg.setCenter(br.translated(xs * cornerRadius, ys * cornerRadius).name()); \
				cg.setFocalPoint(cg.center()); \
				_p->fillRect(QRectF(br.name(), QSizeF(xs * cornerRadius, ys * cornerRadius)), QBrush(cg)); \
			}
			CORNER(topLeft, +1, +1)
			CORNER(topRight, -1, +1)
			CORNER(bottomLeft, +1, -1)
			CORNER(bottomRight, -1, -1)
			#undef CORNER
		}
		{
			QLinearGradient cg;
			cg.setColorAt(0.f, shadow);
			cg.setColorAt(1.f, Qt::transparent);
			#define EDGE(name, x, y, xs, ys) \
			{ \
				cg.setStart(br.translated(y * cornerRadius, x * cornerRadius).name()); \
				cg.setFinalStop(br.name()); \
				_p->fillRect(QRectF(br.translated(x * cornerRadius, y * cornerRadius).name(), QSizeF(xs * cornerRadius, ys * cornerRadius)), QBrush(cg)); \
			}
			EDGE(topLeft, 1, 0, br.width() - 2, 1)
			EDGE(bottomRight, -1, 0, -br.width() + 2, -1)
			EDGE(topLeft, 0, 1, 1, br.height() - 2)
			EDGE(bottomRight, 0, -1, -1, -br.height() + 2)
			#undef EDGE
		}
	}
	_p->restore();

	_p->fillRect(outlineRect().adjusted(1, 1, -1, -1), fillBrush());
	_p->setPen(outerPen());
	_p->drawRoundedRect(outlineRect(), 3, 3);
	_p->setPen(innerPen());
	_p->drawRoundedRect(outlineRect().adjusted(1, 1, -1, -1), 1.5, 1.5);

	QRectF o = outlineRect().adjusted(1, 1, -1, -1);
/*	_p->setPen(QPen(QColor(0, 0, 0, 32), 1));
	for (int i = 0; i < 4; i++)
	{
		double mp = cornerSize * 2.f * (4.0 - i) / 4.0;
		_p->drawLine(o.left() + mp, o.bottom(), o.left(), o.bottom() - mp);
		_p->drawLine(o.left() + mp, o.top(), o.left(), o.top() + mp);
		_p->drawLine(o.right(), o.top() + mp, o.right() - mp, o.top());
	}*/
	_p->setPen(QPen(outlineColour().darker(), 1));
	for (int i = 0; i < 4; i++)
	{
		double mp = (statusHeight + cornerSize) * (4.0 - i) / 4.0;
		_p->drawLine(o.right() - statusMargin, o.bottom() - mp, o.right() - mp, o.bottom() - statusMargin);
	}

	if (isSelected())
	{
		_p->setBrush(Qt::NoBrush);
		for (int i = 1; i < 4; i++)
		{
			_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), i));
			_p->drawRect(outlineRect());
		}
	}
}

void BaseItem::importDom(QDomElement& _item, QGraphicsScene* _scene)
{
	for (QDomNode n = _item.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "property")
			m_properties[n.toElement().attribute("name")] = n.toElement().attribute("value");
	prepareGeometryChange();
	m_size = QSizeF(_item.attribute("w").toDouble(), _item.attribute("h").toDouble());
	_scene->addItem(this);
	setPos(_item.attribute("x").toDouble(), _item.attribute("y").toDouble());
	setName(_item.attribute("name"));
}

void BaseItem::exportDom(QDomElement& _item, QDomDocument& _doc) const
{
	_item.setAttribute("x", pos().x());
	_item.setAttribute("y", pos().y());
	_item.setAttribute("w", m_size.width());
	_item.setAttribute("h", m_size.height());
	_item.setAttribute("name", name());

	foreach (QString k, m_properties.keys())
	{
		QDomElement prop = _doc.createElement("property");
		_item.appendChild(prop);
		prop.setAttribute("name", k);
		prop.setAttribute("value", m_properties[k].toString());
	}
}
