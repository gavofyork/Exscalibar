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

#include "bobport.h"

#include <cassert>
//Added by qt3to4:
#include <QtGlobal>
#include <Q3PtrList>
using namespace std;

#include "buffer.h"
using namespace Geddei;

#include <qdom.h>

#include "bob.h"
#include "boblink.h"
#include "geddeinite.h"
#include "watchprocessor.h"

int BobPort::RTTI = 1001;

/*void BobPort::Info::drawShape(QPainter &p)
{
	p.setPen(QColor(0, 0, 0));
	p.setBrush(QColor(255,255,255));
	p.drawRect(rect());

	p.setPen(QColor(255, 0, 0));
	p.setBrush(QColor(255, 0, 0));
	p.drawRect(int(x()) + 1, int(y()) + height() - 1 - int(theFill * (height() - 2)), width() - 2, int(theFill * (height() - 2)));
}
*/
BobPort::BobPort(Bob *bob, bool isInput, int index, int x, int y): Q3CanvasEllipse(8, 8, bob->canvas())
{
	thePos = QPoint(x - (int)bob->x(), y - (int)bob->y());
	theIsInput = isInput;
	theIndex = index;
	theBob = bob;
	setZ(100);
//	if (theIsInput)
//		theInfo = new Info(x, y, 4, 32, bob->canvas());
	refresh();
}

BobPort::~BobPort()
{
//	if (theIsInput)
//		delete theInfo;
	Q3PtrList<BobLink> links = theLinks;
	for (Q3PtrList<BobLink>::iterator i = links.begin(); i != links.end(); i++)
		delete *i;
}

void BobPort::loadYourself(QDomElement &element)
{
	setPos(QPoint(element.attribute("x").toInt(), element.attribute("y").toInt()));
	refresh();
	if (theIsInput) return;
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{	QDomElement link = n.toElement();
		if (link.isNull()) continue;
		if (link.tagName() == "link")
		{	BobLink *l = new BobLink(this, geddeiNite()->getBob(link.attribute("name"))->inputPort(link.attribute("index").toInt()));
			l->loadYourself(link);
		}
	}
}

void BobPort::saveYourself(QDomElement &element, QDomDocument &doc)
{
	element.setAttribute("index", theIndex);
	element.setAttribute("x", thePos.x() + theBob->x());
	element.setAttribute("y", thePos.y() + theBob->y());
	if (theIsInput) return;
	for (Q3PtrList<BobLink>::iterator i = theLinks.begin(); i != theLinks.end(); i++)
	{	QDomElement link = doc.createElement("link");
		element.appendChild(link);
		link.setAttribute("name", (*i)->destination()->theBob->name());
		link.setAttribute("index", (*i)->destination()->theIndex);
		(*i)->saveYourself(link, doc);
	}
}

bool BobPort::connectYourself()
{
	assert(!theIsInput);
	if (!theLinks.count() && !isActive()) return true;

	Processor *source = processor();
	uint sourceIndex = theIndex;

	if (theLinks.count() > 1) source->split(sourceIndex);

	for (Q3PtrList<BobLink>::iterator i = theLinks.begin(); i != theLinks.end(); i++)
	{
		Processor *Nsource = source;
		uint NsourceIndex = sourceIndex;
		const Connection *c = 0;
		bool ret = Nsource->connect(NsourceIndex, (*i)->destination()->processor(), (*i)->destination()->theIndex, (*i)->theBufferSize);
		(*i)->setConnected(ret);
		if (!ret)
		{	if ((*i)->isActive()) delete c;
			disconnectYourself();
			return false;
		}
	}

	refresh();
	return true;
}

void BobPort::disconnectYourself()
{
	assert(!theIsInput);
	if (theLinks.count() == 0 && !isActive()) return;

	for (Q3PtrList<BobLink>::iterator i = theLinks.begin(); i != theLinks.end(); i++)
		if ((*i)->connected())
			(*i)->setConnected(false);

	Processor *source = processor();
	uint sourceIndex = theIndex;

	if (theLinks.count())
		source->disconnect(sourceIndex);

	refresh();
}

Processor *BobPort::processor()
{
	return theBob->processor();
}

GeddeiNite *BobPort::geddeiNite()
{
	return theBob->geddeiNite();
}

void BobPort::updateProfile()
{
	if (theIsInput)
	{	if (geddeiNite()->connected() && (uint)theIndex < processor()->numInputs())
		{	//theInfo->show();
			/*theInfo->setFill*/
			theFill = processor()->bufferCapacity(theIndex);
		}
		else
//			theInfo->hide();
			theFill = -1;
	}
	canvas()->setChanged(boundingRect());
	canvas()->update();
}

void BobPort::refresh()
{
	setSize(isActive() ? 16 : 12, isActive() ? 16 : 12);
	setX((int)theBob->x() + thePos.x());
	setY((int)theBob->y() + thePos.y());
//	if (theIsInput)
//	{	theInfo->setX((int)theBob->x() + thePos.x());
//		theInfo->setY((int)theBob->y() + thePos.y());
//	}
	updateProfile();
	updateLinks();
}

void BobPort::drawShape(QPainter &p)
{
	int x = (int)BobPort::x(), y = (int)BobPort::y();
	p.setPen(QColor(32, 32, 32));
	if (theIsInput)
	{	p.setBrush(QColor(64, 64, 64));
		p.drawEllipse(x - width() / 2, y - height() / 2, width(), height());
		p.setPen(Qt::NoPen);
		p.setBrush(QColor(theIndex * 45, 255, theFill > 0 ? int(128 + theFill * 112) : 224, QColor::Hsv));
		p.drawEllipse(x - width() / 4, y - height() / 4, width() / 2, height() / 2);
	}
	else
	{	p.setBrush(QColor(theIndex * 45, 224, 255, QColor::Hsv));
		p.drawEllipse(x - width() / 2, y - height() / 2, width(), height());
		p.setPen(Qt::NoPen);
		p.setBrush(QColor(64, 64, 64));
		p.drawEllipse(x - width() / 4, y - height() / 4, width() / 2, height() / 2);
	}
}

void BobPort::updateLinks()
{
	for (Q3PtrList<BobLink>::iterator i = theLinks.begin(); i != theLinks.end(); i++)
		(*i)->refresh();
}

void BobPort::setPos(QPoint p)
{
	QRect rope = theBob->rope();

	float mx = float(p.x() - rope.left()) / float(rope.width()), my = float(p.y() - rope.top()) / float(rope.height());
	int x, y;
	if (mx > my)
		if (1-mx > my) // top
		{	y = rope.top();
			x = rope.left() + int((qMax(0.f, (qMin(mx, 1.f)))) * rope.width());
		}
		else // right
		{	y = rope.top() + int((qMax(0.f, (qMin(my, 1.f)))) * rope.height());
			x = rope.left() + rope.width();
		}
	else
		if (1-mx > my) // left
		{	y = rope.top() + int((qMax(0.f, (qMin(my, 1.f)))) * rope.height());
			x = rope.left();
		}
		else // bottom
		{	y = rope.top() + rope.height();
			x = rope.left() + int((qMax(0.f, (qMin(mx, 1.f)))) * rope.width());
		}

	// set thePos to coordinates actually on rope()
	QPoint oldPos = thePos;
	// note the "- int(theBob->x())" - we need position relative to Bob for easy Bob movement resolution.
	thePos.setX(x - int(theBob->x()));
	thePos.setY(y - int(theBob->y()));
	refresh();
	if (theBob->portCollision(this))
	{	thePos = oldPos;
		refresh();
	}
}

