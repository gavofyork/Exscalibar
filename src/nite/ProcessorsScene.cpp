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

#include <QtGui>

#include <Geddei>
using namespace Geddei;

#include "IncompleteConnectionItem.h"
#include "IncompleteMultipleConnectionItem.h"
#include "InputItem.h"
#include "OutputItem.h"
#include "ConnectionItem.h"
#include "MultipleInputItem.h"
#include "MultipleOutputItem.h"
#include "MultipleConnectionItem.h"
#include "MultiDomProcessorItem.h"
#include "MultiProcessorItem.h"
#include "DomProcessorItem.h"
#include "ProcessorBasedItem.h"
#include "ProcessorsView.h"

void deepRect(QPainter* _p, QRectF _r, bool _down, QColor const& _fill, bool _rIsInside, float _thickness, bool _radialFill)
{
	_r.adjust(.5f, .5f, -.5f, -.5f);
	if (!_rIsInside)
		_r.adjust(_thickness, _thickness, -_thickness, -_thickness);
	QLinearGradient g(_r.topLeft(), _r.bottomLeft());
	g.setColorAt(_down ? 0 : 1, Qt::transparent);
	g.setColorAt(_down ? 1 : 0, QColor(255, 255, 255, 64));
	_p->setPen(QPen(QBrush(g), 1));
	_p->drawRoundedRect(_r.adjusted(-_thickness, -_thickness, _thickness, _thickness), _thickness * 1, _thickness * 1);
	_p->setPen(QPen(QColor(0, 0, 0, 128), 1));
	_p->drawRoundedRect(_r.adjusted(-_thickness / 2, -_thickness / 2, _thickness / 2, _thickness / 2), _thickness * .5, _thickness * .5);

	if (_fill != Qt::transparent)
	{
		_r.adjust(-.5f, -.5f, .5f, .5f);
		if (_radialFill)
		{
			QRadialGradient g(_r.center(), max(_r.width(), _r.height()) * 2 / 3, _r.center());
			g.setColorAt(0, _fill);
			g.setColorAt(1, _fill.darker(150));
			_p->fillRect(_r, g);
		}
		else
		{
			QLinearGradient g(_r.topLeft(), _r.bottomLeft());
			g.setColorAt(0, _fill.darker(150));
			g.setColorAt(1, _fill);
			_p->fillRect(_r, g);
		}
	}
}

ProcessorsScene::ProcessorsScene(QObject* _p): QGraphicsScene(_p), m_currentConnect(0), m_currentMultipleConnect(0), m_timerId(-1), m_dynamicDisplay(false)
{
}

ProcessorsScene::~ProcessorsScene()
{
	foreach (ConnectionItem* ci, filterRelaxed<ConnectionItem>(items()))
		delete ci;
}

void ProcessorsScene::dragEnterEvent(QGraphicsSceneDragDropEvent* _event)
{
	if (_event->mimeData()->hasFormat("text/plain") && _event->mimeData()->text().startsWith("Processor:"))
		_event->setAccepted(ProcessorFactory::available(_event->mimeData()->text().mid(10)));
	else if (_event->mimeData()->hasFormat("text/plain") && _event->mimeData()->text().startsWith("SubProcessor:"))
		_event->setAccepted(SubProcessorFactory::available(_event->mimeData()->text().mid(13)));
}

void ProcessorsScene::dropEvent(QGraphicsSceneDragDropEvent* _event)
{
	if (_event->mimeData()->hasFormat("text/plain") && _event->mimeData()->text().startsWith("Processor:"))
	{
		if (_event->modifiers() & Qt::ShiftModifier)
		{
			MultiProcessorItem* i = new MultiProcessorItem(_event->mimeData()->text().mid(10));
			i->setPos(_event->scenePos());
			addItem(i);
		}
		else
		{
			ProcessorItem* i = new ProcessorItem(_event->mimeData()->text().mid(10));
			i->setPos(_event->scenePos());
			addItem(i);
		}
	}
	else if (_event->mimeData()->hasFormat("text/plain") && _event->mimeData()->text().startsWith("SubProcessor:"))
	{
		if (_event->modifiers() & Qt::ShiftModifier)
		{
			MultiDomProcessorItem* i = new MultiDomProcessorItem(_event->mimeData()->text().mid(13));
			i->setPos(_event->scenePos());
			addItem(i);
		}
		else
		{
			DomProcessorItem* i = new DomProcessorItem(_event->mimeData()->text().mid(13));
			i->setPos(_event->scenePos());
			addItem(i);
		}
	}
	else
		return;
	changed();
	_event->setAccepted(false);
}

void ProcessorsScene::onStarted()
{
	m_timerId = startTimer(110);
}

void ProcessorsScene::onStopped()
{
	killTimer(m_timerId);
	m_timerId = -1;
}

void ProcessorsScene::timerEvent(QTimerEvent*)
{
	if (m_dynamicDisplay)
		foreach (QGraphicsItem* i, items())
			if (BaseItem* pi = dynamic_cast<BaseItem*>(i))
				pi->tick();
}

void ProcessorsScene::beginConnect(OutputItem* _from)
{
	m_currentConnect = new IncompleteConnectionItem(_from);
}

void ProcessorsScene::beginMultipleConnect(MultipleOutputItem* _from)
{
	m_currentMultipleConnect = new IncompleteMultipleConnectionItem(_from);
}

void ProcessorsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* _e)
{
	if (m_currentConnect)
		m_currentConnect->setTo(_e->scenePos());
	if (m_currentMultipleConnect)
		m_currentMultipleConnect->setTo(_e->scenePos());
	QGraphicsScene::mouseMoveEvent(_e);
}

void ProcessorsScene::keyPressEvent(QKeyEvent* _e)
{
	if (_e->key() == Qt::Key_Shift)
		foreach (ProcessorBasedItem* i, filterRelaxed<ProcessorBasedItem>(items()))
			i->setTryMulti(true);
	update();
}

void ProcessorsScene::keyReleaseEvent(QKeyEvent* _e)
{
	if (_e->key() == Qt::Key_Shift)
		foreach (ProcessorBasedItem* i, filterRelaxed<ProcessorBasedItem>(items()))
			i->setTryMulti(false);
	update();
}

void ProcessorsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
{
	ConnectionItem* ci = 0;
	MultipleConnectionItem* mci = 0;
	if (m_currentConnect)
	{
		OutputItem* oi = m_currentConnect->from();
		delete m_currentConnect;
		m_currentConnect = 0;
		if (InputItem* ii = item_cast<InputItem>(itemAt(_e->scenePos())))
			if (ii->isVisible() && !filter<ConnectionItem>(ii->childItems()).count())
			{
				ci = new ConnectionItem(ii, oi);
				changed();
			}
	}
	if (m_currentMultipleConnect && m_currentMultipleConnect)
	{
		MultipleOutputItem* moi = m_currentMultipleConnect->from();
		delete m_currentMultipleConnect;
		m_currentMultipleConnect = 0;
		if (MultipleInputItem* mii = item_cast<MultipleInputItem>(itemAt(_e->scenePos())))
			if (mii->isVisible() && !filter<MultipleConnectionItem>(mii->childItems()).count())
			{
				mci = new MultipleConnectionItem(mii, moi);
				changed();
			}
	}
	QGraphicsScene::mouseReleaseEvent(_e);

	if (ci)
		ci->rejigEndPoints();
	if (mci)
		mci->rejigEndPoints();
	update();
}

