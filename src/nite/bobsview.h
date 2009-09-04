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
#ifndef BOBSVIEW_H
#define BOBSVIEW_H

#define __GEDDEI_BUILD

#include <qpoint.h>
#include <q3canvas.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>

/**
@author Gav Wood <gav@cs.york.ac.uk>
*/

class Bob;
class BobPort;
class FloatingLink;
class Q3CanvasItem;
class GeddeiNite;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class BobsView: public Q3CanvasView
{
Q_OBJECT

	bool theSnapToGrid;
	Q3CanvasItem *theDragging, *theActivating;
	Bob *thePausing;
	BobPort *theLinking;
	FloatingLink *theFloatingLink;
	QPoint theDragOffset;

	void contentsMousePressEvent(QMouseEvent *);
	void contentsMouseReleaseEvent(QMouseEvent *);
	void contentsMouseMoveEvent(QMouseEvent *);
	void dropEvent(QDropEvent *);
	void dragEnterEvent(QDragEnterEvent *);

	GeddeiNite *geddeiNite();

public slots:
	void setSnapToGrid(bool y) { theSnapToGrid = y; }
	void setCanvas(Q3Canvas *c);

public:
	bool snapToGrid() { return theSnapToGrid; }

	BobsView(QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
};

#endif
