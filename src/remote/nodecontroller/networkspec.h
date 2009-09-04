/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_NETWORKSPEC_H
#define _GEDDEI_NETWORKSPEC_H

#include <qstringlist.h>
#include <qmap.h>
#include <q3valuelist.h>
#include <qdom.h>
#include <qstring.h>

#ifdef __GEDDEI_BUILD
#include "properties.h"
#else
#include <geddei/properties.h>
#endif

class LinkSpec
{
	QString theSinkName;
	uint theSinkIndex;
public:
	friend ostream &operator<<(ostream &out, LinkSpec &me)
	{
		return out << me.theSinkName << "/" << me.theSinkIndex;
	}
	
	LinkSpec(const QString &sinkName = "", uint sinkIndex = 0): theSinkName(sinkName), theSinkIndex(sinkIndex) {}
	LinkSpec(QDomElement &e);
};

class PortSpec
{
	Q3ValueList<LinkSpec> theLinks;
public:
	friend ostream &operator<<(ostream &out, PortSpec &me)
	{
		if(me.theLinks.isEmpty()) return out << "-";
		if(me.theLinks.count() == 1) return out << me.theLinks.front();
		out << "{";
		for(Q3ValueList<LinkSpec>::iterator i = me.theLinks.begin(); i != me.theLinks.end(); i++)
			out << (i==me.theLinks.begin()?"":",") << *i;
		return out << "}";
	}
	
	PortSpec() {}
	PortSpec(QDomElement &e);
};

class ProcessorSpec
{
public:
	enum Family
	{
		None = 0,
		MainSource,
		MainSink,
		Factory,
		SubFactory
	};

private:
	QString theName, theType;
	Family theFamily;
	QMap<uint, PortSpec> theOutputs;
	
public:
	const QString &name() const { return theName; }
	const QString &type() const { return theType; }
	const Family family() const { return theFamily; }
	const Geddei::Properties properties() const { return Geddei::Properties(); }
	
	friend ostream &operator<<(ostream &out, ProcessorSpec &me)
	{
		if(me.theFamily == Factory)
			out << "[ P:" << me.theName << " | ";
		else if(me.theFamily == SubFactory)
			out << "[ S:" << me.theName << " | ";
		else if(me.theFamily == MainSource)
			out << "[ > | ";
		else if(me.theFamily == MainSink)
			out << "[ < ";
		for(uint i = 0; i < me.theOutputs.size(); i++)
			out << me.theOutputs[i] << " ";
		return out << "]";
	}
	
	ProcessorSpec(QDomElement &e);
	ProcessorSpec() { theFamily = None; }
};

class NetworkSpec
{
	QMap<QString, ProcessorSpec> theProcessors;
	uint theRealCount;
	
public:
	void load(const QString &filename);
	const uint realCount() { return theRealCount; }
	
	const QStringList names() const { return theProcessors.keys(); }
	const ProcessorSpec &operator[](const QString name) const { return theProcessors[name]; }
	
	friend ostream &operator<<(ostream &out, NetworkSpec &me)
	{
		for(QMap<QString, ProcessorSpec>::iterator i = me.theProcessors.begin(); i != me.theProcessors.end(); i++)
			out << *i << endl;
		return out;
	}
	
	NetworkSpec(const QString &filename) { load(filename); }
};

class FileOpenException {};

#endif
