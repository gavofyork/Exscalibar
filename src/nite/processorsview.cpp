#include <QtGui>
#include <Geddei>
using namespace Geddei;

#include "outputitem.h"
#include "inputitem.h"
#include "connectionitem.h"
#include "incompleteconnectionitem.h"
#include "processoritem.h"
#include "processorsview.h"

ProcessorsView::ProcessorsView(QWidget* _parent): QGraphicsView(_parent)
{
}

ProcessorsScene::ProcessorsScene(): m_currentConnect(0)
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

void ProcessorsScene::beginConnect(OutputItem* _from)
{
	m_currentConnect = new IncompleteConnectionItem(_from);
}

void ProcessorsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* _e)
{
	if (m_currentConnect)
		m_currentConnect->setTo(_e->scenePos());
	QGraphicsScene::mouseMoveEvent(_e);
}

void ProcessorsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
{
	if (m_currentConnect)
	{
		if (InputItem* i = qgraphicsitem_cast<InputItem*>(itemAt(_e->scenePos())))
			new ConnectionItem(i, m_currentConnect->from());

		delete m_currentConnect;
		m_currentConnect = 0;
	}
	update();
	QGraphicsScene::mouseReleaseEvent(_e);
}
