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

#include <Geddei>
using namespace Geddei;

#include "ControlsItem.h"

ControlsItem::ControlsItem(MultiProcessorItem* _p, float _size):
	QGraphicsRectItem	(_p),
	m_clickedRect		(Undefined),
	m_size				(_size)
{
	setRect(0, 0, _size * 3, _size);
	setCursor(Qt::ArrowCursor);
}

void ControlsItem::init()
{
	setPen(Qt::NoPen);
	setBrush(mpi()->outlineColour());
}

uint ControlsItem::whichRect(QPointF _pos) const
{
	if (rect().contains(_pos))
		return _pos.x() * 3 / rect().width();
	return Undefined;
}

QRectF ControlsItem::rectOf(uint _i) const
{
	if (_i != Undefined)
		return QRectF(rect().left() + rect().width() / 3 * _i, rect().top(), rect().width() / 3, rect().height());
	return QRectF();
}

void ControlsItem::paint(QPainter* _p, QStyleOptionGraphicsItem const*, QWidget*)
{
	_p->save();
	float yo = pos().y();
	_p->translate(0, -yo);
	_p->setBrush(Qt::NoBrush);
	_p->setPen(mpi()->outerPen());
	_p->setClipRect(rect().adjusted(-1, -1, 0.5, -2).translated(0, yo));
	_p->drawRoundedRect(rect().adjusted(0, 0, 0, 3).translated(0, yo), 3, 3);
	_p->setClipping(false);
	_p->drawLine(rect().translated(0, yo).bottomRight(), rect().translated(0, yo).bottomRight() + QPointF(0, -3));
	_p->fillRect(rect().adjusted(1, 1, -1, 0).translated(0, yo), mpi()->fillBrush());
	if (m_clickedRect != Undefined)
		_p->fillRect(rectOf(m_clickedRect).adjusted(0, 0, 0, 1).translated(0, yo), QColor(255, 255, 255, 127));
	_p->setPen(mpi()->innerPen());
	_p->setClipRect(rect().adjusted(-1, -1, 0, -.5).translated(0, yo));
	_p->drawRoundedRect(rect().adjusted(1, 1, -1, 3).translated(0, yo), 1.5f, 1.5f);
	_p->setClipping(false);
	_p->drawLine(rect().translated(-1, yo).bottomRight(), rect().translated(-1, yo - 3).bottomRight());
	_p->restore();

	QPen pen = mpi()->outerPen();
	pen.setStyle(Qt::DotLine);
	_p->setPen(pen);
	for (uint i = 1; i < 3; i++)
		_p->drawLine(rectOf(i).topLeft(), rectOf(i).bottomLeft() + QPointF(0, -2));
	if (mpi()->showingAll())
	{
		_p->setBrush(Qt::NoBrush);
		for (int i = 0; i < 2; i++)
		{
			_p->setPen(QPen(i ? Qt::white : QColor(0, 0, 0, 96), 1));
			_p->drawRect(rectOf(1).adjusted(3, 3, -3, -3).translated(0, 1 - i));
			_p->drawRect(rectOf(0).adjusted(2, 3, -2, -3).translated(0, 1 - i));
			_p->drawRect(rectOf(2).adjusted(3, 2, -3, -2).translated(0, 1 - i));
		}
	}
	else
	{
		_p->setPen(QPen(QColor(Qt::black), 0));
		_p->setBrush(QColor(Qt::white));
		QFont f;
		f.setPixelSize(m_size - 2.f);
		_p->setFont(f);
		QString t = QString::number(mpi()->face());
		_p->setPen(QPen(QColor(0, 0, 0, 96), 0));
		_p->drawText(rectOf(1).translated(0, 1), t, QTextOption(Qt::AlignCenter));
		_p->setPen(QPen(QColor(Qt::white), 0));
		_p->drawText(rectOf(1), t, QTextOption(Qt::AlignCenter));

		_p->setBrush(Qt::NoBrush);
		for (int i = 0; i < 2; i++)
		{
			_p->setPen(QPen(i ? Qt::white : QColor(0, 0, 0, 96), 1));
			QRectF r = rectOf(0).adjusted(3, 3, -3, -3).translated(0, 1 - i);
			_p->drawLine(r.topRight(), QPointF(r.left(), r.center().y()));
			_p->drawLine(r.bottomRight(), QPointF(r.left(), r.center().y()));
			r = rectOf(2).adjusted(3, 3, -3, -3).translated(0, 1 - i);
			_p->drawLine(r.topLeft(), QPointF(r.right(), r.center().y()));
			_p->drawLine(r.bottomLeft(), QPointF(r.right(), r.center().y()));
		}
	}
	//_p->drawLines(QVector<QPointF>() << rect().bottomLeft() << rect().topLeft() << rect().topLeft() << rect().topRight() << rect().topRight() << rect().bottomRight());
	if (parentItem()->isSelected())
		for (int i = 1; i < 4; i++)
		{
			_p->setPen(QPen(QColor::fromHsv(220, 220, 255, 128), i));
			_p->drawPolyline(QVector<QPointF>() << rect().translated(0, -2).bottomLeft() << rect().topLeft() << rect().topRight() << rect().bottomRight());
		}
}

void ControlsItem::mousePressEvent(QGraphicsSceneMouseEvent* _e)
{
	m_clickedRect = whichRect(_e->pos());
	update();
}

void ControlsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
{
	if (whichRect(_e->pos()) == m_clickedRect)
	{
		if (mpi()->showingAll())
			switch (m_clickedRect)
			{
			case 0: mpi()->decRowSize(); break;
			case 1: mpi()->toggleShowAll(); break;
			case 2: mpi()->incRowSize(); break;
			}
		else
			switch (m_clickedRect)
			{
			case 0: mpi()->prevFace(); break;
			case 1: mpi()->toggleShowAll(); break;
			case 2: mpi()->nextFace(); break;
			}
	}
	m_clickedRect = Undefined;
	update();
}
