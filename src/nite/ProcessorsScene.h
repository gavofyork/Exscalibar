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

#pragma once

#include <QGraphicsScene>
#include <QGraphicsItem>

template<class T> inline QList<T*> filter(QList<QGraphicsItem*> _l)
{
	QList<T*> ret;
	foreach (QGraphicsItem* i, _l)
		if (T* t = dynamic_cast<T*>(i))
			ret << t;
	return ret;
}

class IncompleteConnectionItem;
class IncompleteMultipleConnectionItem;
class OutputItem;
class MultipleOutputItem;
class ProcessorItem;

class ProcessorsScene: public QGraphicsScene
{
	Q_OBJECT

public:
	ProcessorsScene(QObject* _p);

	void						beginConnect(OutputItem* _from);
	void						beginMultipleConnect(MultipleOutputItem* _from);
	IncompleteConnectionItem*	incompleteConnectionItem() const { return m_currentConnect; }
	IncompleteMultipleConnectionItem*	incompleteMultipleConnectionItem() const { return m_currentMultipleConnect; }

	void						setDynamicDisplay(bool _v) { m_dynamicDisplay = _v; }

	void						onStarted();
	void						onStopped();

signals:
	void						changed();

private:
	virtual void				dragEnterEvent(QGraphicsSceneDragDropEvent* _event);
	virtual void				dragMoveEvent(QGraphicsSceneDragDropEvent* _event) { dragEnterEvent(_event); }
	virtual void				dropEvent(QGraphicsSceneDragDropEvent* _event);

	virtual void				keyPressEvent(QKeyEvent* _e);
	virtual void				keyReleaseEvent(QKeyEvent* _e);

	virtual void				mouseMoveEvent(QGraphicsSceneMouseEvent* _e);
	virtual void				mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);
	virtual void				timerEvent(QTimerEvent*);

	IncompleteConnectionItem*	m_currentConnect;
	IncompleteMultipleConnectionItem*	m_currentMultipleConnect;
	int							m_timerId;
	bool						m_dynamicDisplay;
};

