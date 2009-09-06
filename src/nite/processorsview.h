#pragma once

#define __GEDDEI_BUILD

#include <QGraphicsView>

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class ProcessorsView: public QGraphicsView
{
public:
	ProcessorsView(QWidget* _parent);
};

class ProcessorsScene: public QGraphicsScene
{
public:
	virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* _event);
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* _event) { dragEnterEvent(_event); }
	virtual void dropEvent(QGraphicsSceneDragDropEvent* _event);
};

