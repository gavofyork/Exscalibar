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

class ProcessorsScene: public QGraphicsScene
{
	Q_OBJECT

public:
	virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* _event);
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* _event) { dragEnterEvent(_event); }
	virtual void dropEvent(QGraphicsSceneDragDropEvent* _event);

	void notifyOfFocusChange() { emit focusChanged(focusItem()); }

signals:
	void focusChanged(QGraphicsItem* _focus);
};

