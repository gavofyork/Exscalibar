#pragma once

#define __GEDDEI_BUILD

#include <QGraphicsView>

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

	void beginConnect(OutputItem* _from);
	IncompleteConnectionItem* incompleteConnectionItem() const { return m_currentConnect; }

signals:
	void changed();

private:
	virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* _event);
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* _event) { dragEnterEvent(_event); }
	virtual void dropEvent(QGraphicsSceneDragDropEvent* _event);

	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);

	IncompleteConnectionItem* m_currentConnect;
};

