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

#include "processor.h"
#include "domprocessor.h"
#include "processorgroup.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

ProcessorGroup::ProcessorGroup(bool adopt) : theAdopt(adopt)
{
	theProcessors.clear();
}

ProcessorGroup::~ProcessorGroup()
{
	if(theAdopt) deleteAll();
}

bool ProcessorGroup::exists(const QString &name)
{
	return theProcessors.count(name);
}

Processor &ProcessorGroup::get(const QString &name)
{
	if(theProcessors.count(name))
		return *(theProcessors[name]);
	else
	{	qFatal("*** FATAL: Attempting to attain a Processor object that does not exist.\n"
			   "           You used the non-existant name %s. Bailing.", name.latin1());
		return *((Processor *)(NULL));
	}
}

DomProcessor &ProcessorGroup::dom(const QString &name)
{
	if(theProcessors.count(name))
		return *dynamic_cast<DomProcessor *>(theProcessors[name]);
	else
	{	qFatal("*** FATAL: Attempting to attain a Processor object that does not exist.\n"
			   "           You used the non-existant name %s. Bailing.", name.latin1());
		return *((DomProcessor *)(NULL));
	}
}

void ProcessorGroup::add(Processor *o)
{
	for(QMap<QString, Processor *>::Iterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if(i.data() == o) return;
		else if(i.key() == o->name())
			qDebug("*** ERROR: You are using the same name for multiple Processor objects.\n"
				   "           You have to use a unique name for each object or grouping will not\n"
				   "           work properly.");
	theProcessors[o->name()] = o;
	o->setGroup(*this);
}

void ProcessorGroup::remove(Processor *o)
{
	for(QMap<QString, Processor *>::Iterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if(i.data() == o)
		{	theProcessors.erase(i);
			o->setNoGroup();
			return;
		}
}

bool ProcessorGroup::confirmTypes() const
{
	bool ret = true;
	if(MESSAGES) qDebug("ProcessorGroup::confirmTypes()");
	for(QMap<QString, Processor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
	{	if(MESSAGES) qDebug("ProcessorGroup::confirmTypes(): Confirming %p...", i.data());
		if(!i.data()->confirmTypes()) ret = false;
	}
	return ret;
}

void ProcessorGroup::deleteAll()
{
	QMap<QString, Processor *> theProcessorsBU = theProcessors;
	for(QMap<QString, Processor *>::ConstIterator i = theProcessorsBU.begin(); i != theProcessorsBU.end(); i++)
		delete i.data();
	theProcessors.clear();
}

void ProcessorGroup::disconnectAll()
{
	for(QMap<QString, Processor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.data()->disconnectAll();
}

bool ProcessorGroup::go(bool waitUntilGoing) const
{
	bool ret = true;
//	if(!confirmTypes()) return false;
	for(QMap<QString, Processor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if(!i.data()->go()) ret = false;
	if(ret && waitUntilGoing) return !ProcessorGroup::waitUntilGoing();
	return ret;
}

Processor::ErrorType ProcessorGroup::waitUntilGoing(Processor **errorProc, int *errorData) const
{
	Processor::ErrorType ret;
	for(QMap<QString, Processor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if((ret = i.data()->waitUntilGoing(errorData)) != Processor::NoError)
		{	if(errorProc) *errorProc = i.data();
			for(QMap<QString, Processor *>::ConstIterator j = theProcessors.begin(); j != i; j++)
				j.data()->stop();
			return ret;
		}
	return Processor::NoError;
}

const QString ProcessorGroup::error() const
{
	QString ret;
	for(QMap<QString, Processor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
	{
		Processor::ErrorType e = i.data()->errorType();
		if(e != Processor::NoError && e != Processor::RecursiveFailure && e != Processor::Pending)
		{	if(ret.isEmpty()) ret += "  ";
			ret += i.data()->name() + ": " + i.data()->error();
		}
	}
	return ret;
}

void ProcessorGroup::stop(bool resetToo) const
{
	for(QMap<QString, Processor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.data()->stop();
	if(resetToo) reset();
}

void ProcessorGroup::reset() const
{
	for(QMap<QString, Processor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.data()->reset();
}

}
