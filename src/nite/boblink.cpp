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

#include "boblink.h"

#include <qdom.h>

#include "bobport.h"

int BobLink::RTTI = 1002;

BobLink::BobLink(BobPort *from, BobPort *to): Q3CanvasLine(from->canvas())
{
	theFrom = from;
	theTo = to;
	theFrom->addLink(this);
	theTo->addLink(this);
	theConnected = false;
	theBufferSize = 1;
	theProximity = 10;
	refresh();
	show();
}

BobLink::~BobLink()
{
	hide();
	theFrom->removeLink(this);
	theTo->removeLink(this);
}

void BobLink::loadYourself(QDomElement &element)
{
	theProximity = element.attribute("proximity").toInt();
	theBufferSize = element.attribute("bufferSize").toInt();
}

void BobLink::saveYourself(QDomElement &element, QDomDocument &)
{
	element.setAttribute("proximity", theProximity);
	element.setAttribute("bufferSize", theBufferSize);
}

void BobLink::refresh()
{
	int size = 2;
	if(isActive())
	{	size = 4;
		setZ(500);
	}
	else
		setZ(50);
	if(theConnected)
		setPen(QPen(QColor(192, 128, 128), size, Qt::SolidLine));
	else
		setPen(QPen(QColor(128, 128, 128), size, Qt::DashLine));
	setPoints((int)theFrom->x(), (int)theFrom->y(), (int)theTo->x(), (int)theTo->y());
}
