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

ProcessorsScene::ProcessorsScene(): m_currentConnect(0), m_currentMultipleConnect(0), m_timerId(-1), m_dynamicDisplay(false)
{
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
		ProcessorItem* i = new ProcessorItem(ProcessorFactory::create(_event->mimeData()->text().mid(10)));
		i->setPos(_event->scenePos());
		addItem(i);
	}
	else if (_event->mimeData()->hasFormat("text/plain") && _event->mimeData()->text().startsWith("SubProcessor:"))
	{
		bool doInit = false;
		DomProcessorItem* dpi;
		foreach (dpi, filter<DomProcessorItem>(items()))
			if (dpi->boundingRect().contains(dpi->mapFromScene(_event->scenePos())))
				goto OK;
		doInit = true;
		dpi = new DomProcessorItem;
		OK:
		new SubProcessorItem(dpi, _event->mimeData()->text().mid(13), filter<SubProcessorItem>(dpi->childItems()).count());
		if (doInit)
		{
			dpi->setPos(_event->scenePos());
			addItem(dpi);
		}
	}
	else
		return;
	changed();
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
			if (ProcessorItem* pi = qgraphicsitem_cast<ProcessorItem*>(i))
				pi->tick();
}

void ProcessorsScene::beginConnect(OutputItem* _from)
{
	m_currentConnect = new IncompleteConnectionItem(_from);
}

void ProcessorsScene::beginMultipleConnect(ProcessorItem* _from)
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

void ProcessorsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* _e)
{
	ConnectionItem* ci = 0;
	MultipleConnectionItem* mci = 0;
	if (m_currentConnect)
	{
		OutputItem* oi = m_currentConnect->from();
		delete m_currentConnect;
		m_currentConnect = 0;
		if (InputItem* ii = qgraphicsitem_cast<InputItem*>(itemAt(_e->scenePos())))
			if (!filter<ConnectionItem>(ii->childItems()).count())
			{
				ci = new ConnectionItem(ii, oi);
				changed();
			}
	}
	if (m_currentMultipleConnect)
	{
		ProcessorItem* op = m_currentMultipleConnect->from();
		delete m_currentMultipleConnect;
		m_currentMultipleConnect = 0;

		if (filter<ProcessorItem>(items(_e->scenePos())).count())
		{
			ProcessorItem* ip = filter<ProcessorItem>(items(_e->scenePos()))[0];
			if (filter<MultipleConnectionItem>(ip->childItems()).isEmpty())
			{
				foreach (InputItem* ii, filter<InputItem>(ip->childItems()))
					if (filter<ConnectionItem>(ii->childItems()).count())
						goto BAD;
				mci = new MultipleConnectionItem(ip, op);
				changed();
				BAD:;
			}
		}
	}
	QGraphicsScene::mouseReleaseEvent(_e);

	if (ci)
		ci->rejigEndPoints();
	if (mci)
		mci->rejigEndPoints();
	update();
}
