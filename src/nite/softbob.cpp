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

#include "softbob.h"

#include "processor.h"
#include "processorfactory.h"
#include "processorgroup.h"
using namespace Geddei;

#include <qdom.h>
#include <qtimer.h>

#include "geddeinite.h"
#include "bobport.h"
#include "boblink.h"

int SoftBob::RTTI = 1005;

SoftBob::SoftBob(int x, int y, const QString &type, const QString &name, Q3Canvas *c) : Bob(name, c)
{
	theType = type;

	if (theName == "") theName = geddeiNite()->makeUniqueName(type);
	theProcessor = ProcessorFactory::create(type);

	init(x, y);
	theProperties = Properties(theProcessor->properties());
}

SoftBob::SoftBob(const QString &name, Q3Canvas *c) : Bob(name, c)
{
}

SoftBob::~SoftBob()
{
	delete theProcessor;
}

void SoftBob::saveYourself(QDomElement &element, QDomDocument &doc)
{
	element.setAttribute("type", theType);
	for (uint i = 0; i < theProperties.size(); i++)
	{	QDomElement prop = doc.createElement("property");
		element.appendChild(prop);
		prop.setAttribute("name", theProperties.keys()[i]);
		prop.setAttribute("value", theProperties[theProperties.keys()[i]].toString());
	}
	Bob::saveYourself(element, doc);
}

void SoftBob::loadYourselfPre(QDomElement &element)
{
	Bob::loadYourselfPre(element);
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{	QDomElement property = n.toElement();
		if (property.isNull()) continue;
		if (property.tagName() == "property")
			theProperties.set(property.attribute("name"), property.attribute("value"));
	}
	propertiesChanged();
}

// NEXT TWO ARE BROKEN FOR domsoftbob - NEED VIRTUAL THAT DOES CREATION/INIT...

void SoftBob::rebuild()
{
	delete theProcessor;
	theProcessor = ProcessorFactory::create(theType);
}

void SoftBob::setName(const QString &name)
{
	rebuild();
	theProcessor->init(name, geddeiNite()->group(), theProperties);
	redoPorts();
	update();
	canvas()->update();
}

void SoftBob::propertiesChanged()
{
	QString oldName = theProcessor->name();
	rebuild();
	theProcessor->init(oldName, geddeiNite()->group(), theProperties);
	redoPorts();
	update();
	canvas()->update();
}

