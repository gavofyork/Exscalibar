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

#include "bob.h"

#include "subprocessorfactory.h"
#include "domprocessor.h"
#include "processor.h"
#include "processorfactory.h"
#include "processorgroup.h"
//Added by qt3to4:
#include <Q3PtrList>
using namespace Geddei;

#include <qdom.h>
#include <qtimer.h>

#include "geddeinite.h"
#include "bobport.h"
#include "boblink.h"

int Bob::RTTI = 1000;

Bob::Bob(const QString &name, Q3Canvas *c): QObject(0, name), Q3CanvasRectangle(c)
{
	theName = name;
	geddeiNite()->addBob(this);
	theProfileTimer = 0;
	theRedrawTimer = 0;
	theCanvas = c;
}

void Bob::init(const int x, const int y)
{
	theProcessor->init(theName, geddeiNite()->group());
	int w = theProcessor->width(), h = theProcessor->height();
	setX(x - w/2 - 10);
	setY(y - h/2 - 10);
	setSize(w + 20, h + 20);
	redoPorts();
	show();
}

Bob::~Bob()
{
	geddeiNite()->removeBob(this);
	for(Q3PtrList<BobPort>::iterator i = theInputs.begin(); i != theInputs.end(); i++) delete *i;
	for(Q3PtrList<BobPort>::iterator i = theOutputs.begin(); i != theOutputs.end(); i++) delete *i;
}

const QString Bob::name()
{
	return theProcessor->name();
}

GeddeiNite *Bob::geddeiNite()
{
	return dynamic_cast<GeddeiNite *>(canvas()->parent());
}

bool Bob::portCollision(BobPort *p)
{
	for(Q3PtrList<BobPort>::iterator i = theInputs.begin(); i != theInputs.end(); i++)
		if(p->collidesWith(*i) && p != *i)
			return true;
	for(Q3PtrList<BobPort>::iterator i = theOutputs.begin(); i != theOutputs.end(); i++)
		if(p->collidesWith(*i) && p != *i)
			return true;
 	return false;
}

QRect Bob::rope()
{
	return QRect(int(x()) + 5, int(y()) + 5, int(width()) - 10, int(height()) - 10);
}

void Bob::updatePorts()
{
	for(Q3PtrList<BobPort>::iterator i = theInputs.begin(); i != theInputs.end(); i++)
		(*i)->refresh();
	for(Q3PtrList<BobPort>::iterator i = theOutputs.begin(); i != theOutputs.end(); i++)
		(*i)->refresh();
}

void drawRect(QPainter &p, const int width, const int height, const int offset, const int val)
{
	p.setPen(QColor(val, val, val));
	p.drawRect(offset, offset, width - offset * 2, height - offset * 2);
}

void Bob::drawShape(QPainter &p)
{
	int width = Q3CanvasRectangle::width(), height = Q3CanvasRectangle::height();
	p.save();

	p.translate(x(), y());
	// draw selection rectangle
	if(isActive())
	{	drawRect(p, width, height, 0, 240);
		drawRect(p, width, height, 1, 232);
		drawRect(p, width, height, 2, 216);
		drawRect(p, width, height, 3, 184);
		drawRect(p, width, height, 4, 132);
		drawRect(p, width, height, 5, 0);
		drawRect(p, width, height, 6, 144);
		drawRect(p, width, height, 7, 192);
		drawRect(p, width, height, 8, 224);
		drawRect(p, width, height, 9, 240);
	}
	else
	{	drawRect(p, width, height, 0, 248);
		drawRect(p, width, height, 1, 240);
		drawRect(p, width, height, 2, 232);
		drawRect(p, width, height, 3, 224);
		drawRect(p, width, height, 4, 176);
		drawRect(p, width, height, 5, 132);
		drawRect(p, width, height, 6, 224);
		drawRect(p, width, height, 7, 232);
		drawRect(p, width, height, 8, 240);
		drawRect(p, width, height, 9, 240);
	}

	p.fillRect(10, 10, width - 20, height - 20, QColor(240, 240, 240));

	p.save();

	p.translate(10, 10);
	width -= 20; height -= 20;

	// draw bob internals
	theProcessor->draw(p);

	p.restore();

	if(theProcessor->paused())
	{
		p.setPen(QColor(212, 170, 150));
		p.setBrush(QColor(232, 202, 182));
		p.drawRect(5, 5, 16, 16);
		p.fillRect(8, 8, 4, 10, QColor(48, 48, 48));
		p.fillRect(14, 8, 4, 10, QColor(48, 48, 48));
	}

	p.restore();
}

void Bob::setPos(QPoint p)
{
	double oldX = x(), oldY = y();
	move(p.x(), p.y());
	if(dynamic_cast<GeddeiNite *>(canvas()->parent())->bobCollision(this))
		move(oldX, oldY);
	refresh();
}

void Bob::saveYourself(QDomElement &element, QDomDocument &doc)
{
	element.setAttribute("name", name());
	element.setAttribute("x", x());
	element.setAttribute("y", y());

	for(Q3PtrList<BobPort>::iterator i = theInputs.begin(); i != theInputs.end(); i++)
	{	QDomElement out = doc.createElement("input");
		element.appendChild(out);
		(*i)->saveYourself(out, doc);
	}
	for(Q3PtrList<BobPort>::iterator i = theOutputs.begin(); i != theOutputs.end(); i++)
	{	QDomElement out = doc.createElement("output");
		element.appendChild(out);
		(*i)->saveYourself(out, doc);
	}
}

void Bob::loadYourselfPre(QDomElement &element)
{
	setX(element.attribute("x").toFloat());
	setY(element.attribute("y").toFloat());
}

void Bob::loadYourselfPost(QDomElement &element)
{
	for(QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{	QDomElement port = n.toElement();
		if(port.isNull()) continue;
		if(port.tagName() == "output")
			theOutputs.at(port.attribute("index").toInt())->loadYourself(port);
		else if(port.tagName() == "input")
			theInputs.at(port.attribute("index").toInt())->loadYourself(port);
	}
}

void Bob::redoPorts()
{
	int inputs = theProcessor->numInputs(), outputs = theProcessor->numOutputs();
	for(int i = theInputs.count(); i < inputs; i++)
	{	BobPort *p = new BobPort(this, true, i, rope().left() + i * 4, rope().top());
		p->show();
		theInputs.append(p);
	}
	for(int i = theInputs.count(); i > inputs; i--)
	{	theInputs.last()->hide();
		delete theInputs.last();
		theInputs.removeLast();
	}

	for(int i = theOutputs.count(); i < outputs; i++)
	{	BobPort *p = new BobPort(this, false, i, rope().left() + rope().width() - i * 4, rope().top() + rope().height());
		p->show();
		theOutputs.append(p);
	}
	for(int i = theOutputs.count(); i > outputs; i--)
	{	theOutputs.last()->hide();
		delete theOutputs.last();
		theOutputs.removeLast();
	}
	updatePorts();
	canvas()->update();
}

void Bob::slotRedraw()
{
	theCanvas->setChanged(boundingRect());
	theCanvas->update();
}

bool Bob::connectYourself()
{
	if(theProcessor->redrawPeriod())
	{	theRedrawTimer = new QTimer();
		QObject::connect(theRedrawTimer, SIGNAL(timeout()), this, SLOT(slotRedraw()));
		theRedrawTimer->start(theProcessor->redrawPeriod());
	}

	for(Q3PtrList<BobPort>::iterator i = theOutputs.begin(); i != theOutputs.end(); i++)
		if(!(*i)->connectYourself()) return false;

	theProfileTimer = new QTimer();
	for(Q3PtrList<BobPort>::iterator i = theInputs.begin(); i != theInputs.end(); i++)
		QObject::connect(theProfileTimer, SIGNAL(timeout()), *i, SLOT(updateProfile()));

	refresh();
	theProfileTimer->start(100);
	return true;
}

void Bob::disconnectYourself()
{
	delete theProfileTimer;
	theProfileTimer = 0;

	delete theRedrawTimer;
	theRedrawTimer = 0;

	for(Q3PtrList<BobPort>::iterator i = theOutputs.begin(); i != theOutputs.end(); i++)
		(*i)->disconnectYourself();
	refresh();
	slotRedraw();
}

