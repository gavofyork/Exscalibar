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
#ifndef BOBPORT_H
#define BOBPORT_H

#define __GEDDEI_BUILD

#include "processor.h"
using namespace Geddei;

#include <q3canvas.h>
#include <q3ptrlist.h>
#include <qpoint.h>
#include <qobject.h>

#include "refresher.h"

class QPainter;
class QDomElement;
class QDomDocument;

class GeddeiNite;
class Bob;
class BobLink;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class BobPort: public QObject, public Q3CanvasEllipse, public Refresher
{
	Q_OBJECT

	QPoint thePos;
	int theIndex;
	Bob *theBob;
	bool theIsInput;
	Q3PtrList<BobLink> theLinks;
/*	class Info: public QCanvasRectangle
	{
		friend class BobPort;
		float theFill;
		virtual void drawShape(QPainter &p);
	public:
		void setFill(float f) { theFill = f; update(); }
		Info(int x, int y, int w, int h, QCanvas *c) : QCanvasEllipse(x, y, w + 2, h + 2, c) { setZ(1000); theFill = 1; }
	} *theInfo;
*/
	float theFill;
	void updateLinks();
	virtual void drawShape(QPainter &p);

	Processor *processor();

public slots:
	void updateProfile();

public:
	static int RTTI;
	virtual int rtti() const { return 1001; }

	int numLinks() { return theLinks.count(); }

	bool connectYourself();
	void disconnectYourself();
	void saveYourself(QDomElement &element, QDomDocument &doc);
	void loadYourself(QDomElement &element);

	void refresh();
	bool isInput() { return theIsInput; }
	void setPos(QPoint p);
	GeddeiNite *geddeiNite();

	bool isSiblingOf(BobPort *p) { return p->theBob == theBob; }

	void addLink(BobLink *link) { theLinks.append(link); }
	void removeLink(BobLink *link) { theLinks.remove(link); }

	BobPort(Bob *bob, bool isInput, int index, int x = 0, int y = 0);
	~BobPort();
};

#endif
