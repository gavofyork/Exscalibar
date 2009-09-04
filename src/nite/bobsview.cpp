/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <qstatusbar.h>
#include <qaction.h>
#include <q3dragobject.h>
//Added by qt3to4:
#include <Q3CString>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>

#include "geddeinite.h"
#include "bobsview.h"
#include "softbob.h"
#include "domsoftbob.h"
#include "hardbob.h"
#include "bobport.h"
#include "boblink.h"
#include "floatinglink.h"

BobsView::BobsView(QWidget *parent, const char *name, Qt::WFlags f): Q3CanvasView(parent, name, f)
{
	theSnapToGrid = true;
}

void BobsView::setCanvas(Q3Canvas *c)
{
	Q3CanvasView::setCanvas(c);
	theFloatingLink = new FloatingLink(canvas());
}

void BobsView::dragEnterEvent(QDragEnterEvent *e)
{
	e->accept(Q3TextDrag::canDecode(e));
}

void BobsView::dropEvent(QDropEvent *e)
{
	QMimeSource const* ms = e;
	QPoint pos = viewportToContents(e->pos());
	QString type;
	QString subType;
	Q3TextDrag::decode(ms, type, subType);
	if(subType == "processor")
		new SoftBob(pos.x(), pos.y(), type, "", canvas());
	else if(subType == "subprocessor")
		new DomSoftBob(pos.x(), pos.y(), type, "", canvas());
	canvas()->update();
	geddeiNite()->setModified();
}

void BobsView::contentsMousePressEvent(QMouseEvent *e)
{
	theLinking = 0;
	theDragging = 0;
	theActivating = 0;
	thePausing = 0;
	Q3CanvasItemList l = canvas()->collisions(e->pos());
	for (Q3CanvasItemList::Iterator it = l.begin(); it != l.end(); it++)
		if (dynamic_cast<Bob *>(*it))
		{	if (geddeiNite()->modeStop->isOn())
			{	theActivating = *it;
				theDragging = dynamic_cast<Bob *>(*it);
				theDragOffset = QPoint((int)theDragging->x(), (int)theDragging->y()) - e->pos();
				break;
			}
			else if (geddeiNite()->modeRun->isOn())
			{	thePausing = dynamic_cast<Bob *>(*it);
				break;
			}
		}
		else if (dynamic_cast<BobLink *>(*it))
		{	if(geddeiNite()->modeStop->isOn())
			{	theActivating = *it;
				break;
			}
		}
		else if (dynamic_cast<BobPort *>(*it))
		{	if(geddeiNite()->modeStop->isOn())
			{	theDragging = dynamic_cast<BobPort *>(*it);
				theDragOffset = QPoint(0, 0);
				theActivating = *it;
				break;
			}
			else if(geddeiNite()->modeMakeLink->isOn())
			{	if(dynamic_cast<BobPort *>(*it)->isInput())
				{	geddeiNite()->statusBar()->message("Cannot create link from this: Port must be an output.", 2000);
					break;
				}
				theLinking = dynamic_cast<BobPort *>(*it);
				theFloatingLink->setFrom(theLinking);
				theFloatingLink->setTo(e->pos());
				theFloatingLink->show();
				break;
			}
		}

	if(!l.count())
		if(geddeiNite()->modeStop->isOn())
			geddeiNite()->setActive();

	canvas()->update();
}

void BobsView::contentsMouseReleaseEvent(QMouseEvent *e)
{
	if(theActivating)
	{
		Q3CanvasItemList l = canvas()->collisions(e->pos());
		bool reset = true;
		for (Q3CanvasItemList::Iterator it = l.begin(); it != l.end(); it++)
			if(*it == theActivating)
			{	geddeiNite()->setActive(*it);
				reset = false;
				break;
			}
		if(reset)
			geddeiNite()->setActive();
	}
	else if(thePausing)
	{
		Q3CanvasItemList l = canvas()->collisions(e->pos());
		for (Q3CanvasItemList::Iterator it = l.begin(); it != l.end(); it++)
			if(*it == thePausing)
			{	if(thePausing->processor()->paused())
					thePausing->processor()->unpause();
				else
					thePausing->processor()->pause();
				canvas()->setChanged(thePausing->boundingRect());
				break;
			}
	}
	else if(theLinking)
	{
		Q3CanvasItemList l = canvas()->collisions(e->pos());
		for (Q3CanvasItemList::Iterator it = l.begin(); it != l.end(); it++)
		{	if (dynamic_cast<BobPort *>(*it))
			{	BobPort *bobPort = dynamic_cast<BobPort *>(*it);
				if(!bobPort->isInput())
				{	geddeiNite()->statusBar()->message("Cannot link to this: Port must be an input.", 2000);
					break;
				}
				if(bobPort->numLinks())
				{	geddeiNite()->statusBar()->message("Cannot link to this: Port is already connected.", 2000);
					break;
				}
				if(bobPort->isSiblingOf(theLinking))
				{	geddeiNite()->statusBar()->message("Cannot link to this: They share the same processor!", 2000);
					break;
				}
				geddeiNite()->setActive(*it);
				(new BobLink(theLinking, bobPort))->show();
				geddeiNite()->modeStop->setOn(true);
				geddeiNite()->setModified();
				break;
			}
		}
	}
	theFloatingLink->hide();
	theLinking = 0;
	theDragging = 0;
	theActivating = 0;
	thePausing = 0;

	canvas()->update();
}

GeddeiNite *BobsView::geddeiNite()
{
	return dynamic_cast<GeddeiNite *>(topLevelWidget());
}

void BobsView::contentsMouseMoveEvent(QMouseEvent *e)
{
	QPoint p = QPoint(e->pos().x() + theDragOffset.x(), e->pos().y() + theDragOffset.y());
	if(theSnapToGrid) p = p / 10 * 10;

	if(theDragging)
	{	if(dynamic_cast<Bob *>(theDragging))
			dynamic_cast<Bob *>(theDragging)->setPos(p);
		else if(dynamic_cast<BobPort *>(theDragging))
			dynamic_cast<BobPort *>(theDragging)->setPos(p + QPoint(5, 5));
		geddeiNite()->setModified();
	}
	else if(theLinking)
	{
		//TODO: nice grpahics for this...
		theFloatingLink->setTo(e->pos());
	}
	canvas()->update();
}

