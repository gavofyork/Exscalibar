#pragma once

#define __GEDDEI_BUILD

#include <QGraphicsView>
#include <QGraphicsItem>

template<class T> inline QList<T*> filter(QList<QGraphicsItem*> _l)
{
	QList<T*> ret;
	foreach (QGraphicsItem* i, _l)
		if (T* t = qgraphicsitem_cast<T*>(i))
			ret << t;
	return ret;
}

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class ProcessorsView: public QGraphicsView
{
	Q_OBJECT

public:
	ProcessorsView(QWidget* _parent);
};

class IncompleteConnectionItem;
class OutputItem;

class ProcessorsScene: public QGraphicsScene
{
	Q_OBJECT

public:
	ProcessorsScene();

	void						beginConnect(OutputItem* _from);
	IncompleteConnectionItem*	incompleteConnectionItem() const { return m_currentConnect; }

	void						setDynamicDisplay(bool _v) { m_dynamicDisplay = _v; }

	void						onStarted();
	void						onStopped();

signals:
	void						changed();

private:
	virtual void				dragEnterEvent(QGraphicsSceneDragDropEvent* _event);
	virtual void				dragMoveEvent(QGraphicsSceneDragDropEvent* _event) { dragEnterEvent(_event); }
	virtual void				dropEvent(QGraphicsSceneDragDropEvent* _event);

	virtual void				mouseMoveEvent(QGraphicsSceneMouseEvent* _e);
	virtual void				mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);
	virtual void				timerEvent(QTimerEvent*);

	IncompleteConnectionItem*	m_currentConnect;
	int							m_timerId;
	bool						m_dynamicDisplay;
};

