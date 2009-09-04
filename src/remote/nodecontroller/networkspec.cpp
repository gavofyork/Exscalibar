/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <cassert>
using namespace std;

#include <qfile.h>

#include "networkspec.h"

LinkSpec::LinkSpec(QDomElement &e)
{
	theSinkIndex = e.attribute("index").toInt();
	theSinkName = e.attribute("name");
}

PortSpec::PortSpec(QDomElement &e)
{
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
	{	QDomElement link = n.toElement();
	if(link.isNull()) continue;
	if(link.tagName() == "link")
		theLinks.push_back(LinkSpec(link));
	}
}

ProcessorSpec::ProcessorSpec(QDomElement &e)
{
	theName = e.attribute("name");
	if(e.tagName() == "processor")
	{
		theFamily = Factory;
		theType = e.attribute("type");
	}
	else if(e.tagName() == "subprocessor") theFamily = SubFactory;
	else if(e.tagName() == "builtin")
	{	if(e.attribute("type") == "Player")
		theFamily = MainSource;
	}
	
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
	{	QDomElement port = n.toElement();
	if(port.isNull()) continue;
	if(port.tagName() == "output")
		theOutputs[port.attribute("index").toInt()] = PortSpec(port);
	}
}


void NetworkSpec::load(const QString &filename)
{
	QFile f(filename);
	if(!f.open(QIODevice::ReadOnly)) throw FileOpenException();
	QDomDocument doc;
	doc.setContent(&f, false);
	
	theRealCount = 0;
	theProcessors.clear();
	
	QDomElement root = doc.documentElement();
	for(QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling())
	{	QDomElement elem = n.toElement();
	if(elem.isNull()) continue;
	if(elem.tagName() == "processor" || elem.tagName() == "subprocessor")
		theRealCount++;
	if(elem.tagName() == "processor" || elem.tagName() == "subprocessor" || elem.tagName() == "builtin")
		theProcessors[elem.attribute("name")] = ProcessorSpec(elem);
	}
}
