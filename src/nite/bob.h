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
#ifndef BOB_H
#define BOB_H

#define __GEDDEI_BUILD

#include "properties.h"
#include "processor.h"
using namespace Geddei;

#include <q3ptrlist.h>
#include <qstring.h>
#include <qrect.h>
#include <q3canvas.h>

#include "refresher.h"

class QDomDocument;
class QDomElement;
class QPainter;
class QPoint;
class QTimer;

class BobPort;
class GeddeiNite;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class Bob: public QObject, public Q3CanvasRectangle, public Refresher
{
Q_OBJECT
	friend class GeddeiNite;

	Q3PtrList<BobPort> theInputs, theOutputs;
	QTimer *theProfileTimer, *theRedrawTimer;

	virtual void drawShape(QPainter &p);

private slots:
	virtual void slotRedraw();

protected:
	QString theName;
	Processor *theProcessor;
	Q3Canvas *theCanvas;

public:
	static int RTTI;
	virtual int rtti() const { return 1000; }

	void setPos(QPoint p);

	QRect rope();
	const QString name();
	GeddeiNite *geddeiNite();
	Processor *processor() { return theProcessor; }
	BobPort *inputPort(int index) { return theInputs.at(index); }

	bool connectYourself();
	void disconnectYourself();
	bool connected() const { return theProfileTimer != 0; }
	virtual void saveYourself(QDomElement &element, QDomDocument &doc);
	virtual void loadYourselfPre(QDomElement &element);
	virtual void loadYourselfPost(QDomElement &element);

	void updatePorts();
	void redoPorts();
	bool portCollision(BobPort *p);

	void refresh() { updatePorts(); }

	void init(int x, int y);

	Bob(const QString &name, Q3Canvas *c);
	virtual ~Bob();
};

#endif
