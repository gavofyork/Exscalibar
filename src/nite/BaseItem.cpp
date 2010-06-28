/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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
#include "BaseItem.h"
#include "PropertyItem.h"
#include "ConnectionItem.h"

#include "Magnetic.h"
#include "InputItem.h"
#include "OutputItem.h"

static const double cornerSize = 2.0;

BaseItem::BaseItem(Properties const& _pr, QSizeF const& _size):
	WithProperties	(0, _pr),
	m_size			(_size),
	m_timerId		(-1),
	m_resizing		(false)
{
	assert(m_size.height() < 100000);
	setAcceptHoverEvents(true);
	setFlags(ItemClipsToShape | ItemIsFocusable | ItemIsSelectable | ItemIsMovable);
}

BaseItem::~BaseItem()
{
}

void BaseItem::setProperty(QString const& _key, QVariant const& _value)
{
	m_properties[_key] = _value;
	propertiesChanged();
	//dynamic_cast<GeddeiNite*>(scene()->parent())->propertyHasBeenChanged();
}

void BaseItem::setDefaultProperties(PropertiesInfo const& _def)
{
	setPropertiesInfo(_def);

	foreach (PropertyItem* i, filterRelaxed<PropertyItem>(childItems()))
		delete i;

	m_controlsStack = QSizeF(0.f, 0.f);
	m_controlsBar = QSizeF(0.f, 0.f);
	foreach (QString k, m_dynamicKeys)
	{
		PropertyItem* item = new PropertyItem(this, QRectF(0, 0, m_size.width(), 0), k);
		m_controlsStack = QSizeF(max<float>(m_controlsSize.width(), item->minWidth()), m_controlsSize.height() + item->boundingRect().height());
		m_controlsBar = QSizeF(m_controlsSize.width() + item->minWidth(), max<float>(item->boundingRect().height(), item->boundingRect().height()));
	}
	checkWidgets();
}

void BaseItem::checkWidgets()
{
//	QRectF widget(widgetMarginP, m_size.height() - m_controlsSize.height() + widgetMarginP, 0/*m_size.width() - widgetMarginP * 2*/, m_controlsSize.height());
//	foreach (PropertyItem* i, filterRelaxed<PropertyItem>(childItems()))
//	{
//		i->resize(widget);
//		widget.translate(i->boundingRect().width() + 5.f, 0);
//	}
	float oX = controlsRect().left();
	float oY = controlsRect().top();
	float maxWidth = controlsRect().width();
	m_controlsSize = QSizeF(0.f, 0.f);
	float csPos = 0.f;
	float csLine = 0.f;
	float csFixed = 0.f;
	QList<PropertyItem*> line;
	QList<PropertyItem*> items = filterRelaxed<PropertyItem>(childItems());
	for (int ii = 0; ii < items.count(); ii++)
	{
		PropertyItem* item = items[ii];
		line << item;
		item->setPos(csPos + oX, m_controlsSize.height() + oY);
		csLine = max<float>(csLine, item->boundingRect().height());
		csPos += item->minWidth();
		csFixed += item->isExpandable() ? 0 : item->minWidth();
		m_controlsSize.setWidth(item->minWidth());

		if (ii == items.count() - 1 || items[ii + 1]->minWidth() + csPos > maxWidth)
		{
			if (csFixed < csPos)
			{
				float w = (max<float>(item->minWidth(), maxWidth) - csFixed) / (csPos - csFixed);
				float x = 0.f;
				for (int i = 0; i < line.count(); i++)
				{
					line[i]->setPos(floor(x) + oX, line[i]->pos().y());
					float nw = floor(line[i]->minWidth() * (line[i]->isExpandable() ? w : 1));
					line[i]->resize(QRectF(0, 0, nw, csLine));
					x += nw;
				}
			}
			m_controlsSize.setHeight(m_controlsSize.height() + csLine);
			csPos = 0;
			csLine = 0;
			csFixed = 0;
			line.clear();
		}
	}
}

void BaseItem::timerEvent(QTimerEvent*)
{
	update(clientRect().adjusted(2, 2, -2, -2));
}

void BaseItem::propertiesChanged(QString const&)
{
	if (scene())
	{
		dynamic_cast<GeddeiNite*>(scene()->parent())->propertyHasBeenChanged();
		foreach (ConnectionItem* ci, filterRelaxed<ConnectionItem>(scene()->items()))
			if (ci->fromBase() == this || ci->toBase() == this)
				ci->refreshNature();
	}
	geometryChanged();
}

QSizeF BaseItem::controlsSize() const
{
	return m_controlsSize;
}

QSizeF BaseItem::controlsSize(float _minWidth) const
{
	QSizeF ret(0.f, 0.f);
	float cSPos = 0.f;
	float cSLine = 0.f;
	foreach (PropertyItem* item, filter<PropertyItem>(childItems()))
	{
		if (item->minWidth() + cSPos > max<float>(item->minWidth(), _minWidth))
		{
			ret.setHeight(ret.height() + cSLine);
			cSPos = 0;
			cSLine = 0;
		}
		cSLine = max<float>(cSLine, item->boundingRect().height());
		ret.setWidth(max<float>(ret.width(), cSPos += item->minWidth()));
	}
	ret.setHeight(ret.height() + cSLine);
	return ret;
}

QSizeF BaseItem::interiorWith(QSizeF _client) const
{
	QSizeF controls = controlsSize(_client.width() - (isResizable() ? portHeight() : 0));
	return QSizeF(max<float>(controls.width() + (isResizable() ? portHeight() : 0), _client.width()), max<float>(interiorPorts(), _client.height() + max<float>(isResizable() ? portHeight() : 0, controls.height())));
}

void BaseItem::geometryChanged()
{
	prepareGeometryChange();
	if (!m_size.isValid() || m_size.isEmpty() || !isResizable())
		m_size = interiorMin();
	m_controlsSize = controlsSize(m_size.width() - (isResizable() ? portHeight() : 0));
	checkWidgets();
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

QRectF BaseItem::exteriorRect() const
{
	// Add one to width & height as we translate up 7 left by half a pixel when drawing.
	return interiorRect().adjusted(-cornerSize, -cornerSize, cornerSize + 1, cornerSize + 1);
}

QRectF BaseItem::resizeRect() const
{
	if (isResizable())
		return QRectF(exteriorRect().bottomRight(), QSizeF(-(portHeight() + cornerSize), -(portHeight() + cornerSize)));
	else
		return QRectF(0, 0, 0, 0);
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
	if (uint i = redrawPeriod())
		m_timerId = startTimer(i);
}

void BaseItem::disconnectYourself()
{
	if (m_timerId > -1)
		this->killTimer(m_timerId);
}

void BaseItem::unprepYourself()
{
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
	else if (!exteriorRect().contains(_e->pos()))
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
	QGraphicsItem::hoverMoveEvent(_e);
}

QList<QPointF> BaseItem::magnetism(BaseItem const* _b, bool _moving) const
{
	QList<QPointF> ret;
	if (_b == this && !_moving)
		ret << QPointF(clientPref().width() - clientRect().width(), clientPref().height() - clientRect().height());

	if (_b == this)
		return ret;

	QRectF us = exteriorRect().translated(pos());
	QRectF them = _b->exteriorRect().translated(_b->pos());

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
		m_size = QSizeF(max(interiorMin().width(), m_size.width()), max(interiorMin().height(), m_size.height()));
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
	_p->setClipRect(clientRect().adjusted(0, 0, .5f, .5f));
	_p->translate(.5f, .5f);
	paintCentre(_p);
}

void BaseItem::paintCentre(QPainter*)
{
}

QBrush BaseItem::fillBrush() const
{
	QLinearGradient cg(exteriorRect().topLeft(), exteriorRect().bottomLeft());
	cg.setColorAt(0, outlineColour().lighter(125));
	cg.setColorAt(1, outlineColour().darker(150));
	return QBrush(cg);
}
QPen BaseItem::innerPen() const
{
	QLinearGradient cg(exteriorRect().topLeft(), exteriorRect().bottomLeft());
	cg.setColorAt(0, outlineColour().lighter(175));
	cg.setColorAt(1, outlineColour().darker(150));
	return QPen(cg, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
}
QPen BaseItem::outerPen() const
{
	return QPen(Qt::black, 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
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

	_p->fillRect(exteriorRect().adjusted(1, 1, -1, -1), fillBrush());
	_p->setPen(outerPen());
	_p->drawRoundedRect(exteriorRect(), 3, 3);
	_p->setPen(innerPen());
	_p->drawRoundedRect(exteriorRect().adjusted(1, 1, -1, -1), 1.5, 1.5);

	QRectF o = exteriorRect().adjusted(1, 1, -1, -1);
/*	_p->setPen(QPen(QColor(0, 0, 0, 32), 1));
	for (int i = 0; i < 4; i++)
	{
		double mp = cornerSize * 2.f * (4.0 - i) / 4.0;
		_p->drawLine(o.left() + mp, o.bottom(), o.left(), o.bottom() - mp);
		_p->drawLine(o.left() + mp, o.top(), o.left(), o.top() + mp);
		_p->drawLine(o.right(), o.top() + mp, o.right() - mp, o.top());
	}*/
	if (isResizable())
	{
		_p->setPen(QPen(outlineColour().darker(), 1));
		for (int i = 0; i < 4; i++)
		{
			double mp = (portHeight()) * (4.0 - i) / 4.0;
			_p->drawLine(o.right() - cornerSize, o.bottom() - mp, o.right() - mp, o.bottom() - cornerSize);
		}
	}

	if (isSelected())
	{
		_p->setBrush(Qt::NoBrush);
		for (int i = 1; i < 4; i++)
		{
			_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), i));
			_p->drawRect(exteriorRect());
		}
	}
}

void BaseItem::importDom(QDomElement& _item, QGraphicsScene* _scene)
{
	for (QDomNode n = _item.firstChild(); !n.isNull(); n = n.nextSibling())
		if (n.toElement().tagName() == "property")
			m_properties[n.toElement().attribute("name")] = n.toElement().attribute("value");
	prepareGeometryChange();
	if (isResizable())
		m_size = QSizeF(_item.attribute("w").toDouble(), _item.attribute("h").toDouble());
	else
		m_size = QSizeF(0, 0);
	_scene->addItem(this);
	setPos(_item.attribute("x").toDouble(), _item.attribute("y").toDouble());
	setName(_item.attribute("name"));
}

void BaseItem::exportDom(QDomElement& _item, QDomDocument& _doc) const
{
	_item.setAttribute("x", pos().x());
	_item.setAttribute("y", pos().y());
	if (isResizable())
	{
		_item.setAttribute("w", m_size.width());
		_item.setAttribute("h", m_size.height());
	}
	_item.setAttribute("name", name());

	foreach (QString k, m_properties.keys())
	{
		QDomElement prop = _doc.createElement("property");
		_item.appendChild(prop);
		prop.setAttribute("name", k);
		prop.setAttribute("value", m_properties[k].toString());
	}
}
