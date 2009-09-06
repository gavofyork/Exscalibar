#include <QtGui>
#include <Geddei>
using namespace Geddei;

#include "processoritem.h"
#include "processorsview.h"

ProcessorsView::ProcessorsView(QWidget* _parent): QGraphicsView(_parent)
{
}

void ProcessorsScene::dragEnterEvent(QGraphicsSceneDragDropEvent* _event)
{
	_event->setAccepted(_event->mimeData()->hasFormat("text/plain") && ProcessorFactory::available(_event->mimeData()->text()));
}

void ProcessorsScene::dropEvent(QGraphicsSceneDragDropEvent* _event)
{
	ProcessorItem* i = new ProcessorItem(ProcessorFactory::create(_event->mimeData()->text()));
	i->setPos(_event->scenePos());
	addItem(i);
}
