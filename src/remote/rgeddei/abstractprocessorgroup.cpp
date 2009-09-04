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

#include "abstractprocessor.h"
#include "abstractdomprocessor.h"
#include "abstractprocessorgroup.h"
using namespace rGeddei;

#define MESSAGES 0

namespace rGeddei
{

AbstractProcessorGroup::AbstractProcessorGroup()
{
	theProcessors.clear();
}

AbstractProcessorGroup::~AbstractProcessorGroup()
{
}

const bool AbstractProcessorGroup::exists(const QString &name)
{
	return theProcessors.count(name);
}

AbstractProcessor &AbstractProcessorGroup::get(const QString &name)
{
	if(theProcessors.count(name))
		return *(theProcessors[name]);
	else
	{	qFatal("*** FATAL: Attempting to attain a Processor object that does not exist.\n"
			   "           You used the non-existant name %s. Bailing.", name.latin1());
		return *((AbstractProcessor *)0);
	}
}

AbstractDomProcessor &AbstractProcessorGroup::dom(const QString &name)
{
	if(theProcessors.count(name))
		return *dynamic_cast<AbstractDomProcessor *>(theProcessors[name]);
	else
	{	qFatal("*** FATAL: Attempting to attain a DomProcessor object that does not exist.\n"
			   "           You used the non-existant name %s. Bailing.", name.latin1());
		return *((AbstractDomProcessor *)0);
	}
}

void AbstractProcessorGroup::add(AbstractProcessor *o)
{
	for(QMap<QString, AbstractProcessor *>::Iterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if(i.data() == o) return;
		else if(i.key() == o->name())
			qDebug("*** ERROR: You are using the same name for multiple AbstractProcessor objects.\n"
				   "           You have to use a unique name for each object or grouping will not\n"
				   "           work properly.");
	theProcessors[o->name()] = o;
	o->setGroup(*this);
}

void AbstractProcessorGroup::remove(AbstractProcessor *o)
{
	for(QMap<QString, AbstractProcessor *>::Iterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if(i.data() == o)
		{	theProcessors.erase(i);
			o->setNoGroup();
			return;
		}
}

void AbstractProcessorGroup::deleteAll()
{
	QMap<QString, AbstractProcessor *> theProcessorsBU = theProcessors;
	for(QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessorsBU.begin(); i != theProcessorsBU.end(); i++)
	{	if(MESSAGES) qDebug("Deleting %s...", i.data()->name().latin1());
		delete i.data();
	}
	theProcessors.clear();
}

void AbstractProcessorGroup::disconnectAll()
{
	for(QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.data()->disconnectAll();
}

void AbstractProcessorGroup::init() const
{
	for(QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.data()->init();
}

const bool AbstractProcessorGroup::go(const bool waitUntilGoing) const
{
	for(QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if(!i.data()->go())
		{	for(QMap<QString, AbstractProcessor *>::ConstIterator j = theProcessors.begin(); j != i; j++)
				j.data()->stop();
			return false;
		}
	if(!waitUntilGoing) return true;
	AbstractProcessor *errorProc;
	int errorData;
	Processor::ErrorType e = this->waitUntilGoing(&errorProc, &errorData);
	if(e != Processor::NoError)
	{	qWarning("Error starting processors: Code %d (name=%s, data=%d).", (int)e, errorProc->name().latin1(), errorData);
		return false;
	}
	return true;
}

const Processor::ErrorType AbstractProcessorGroup::waitUntilGoing(AbstractProcessor **errorProc, int *errorData) const
{
	Processor::ErrorType ret;
	for(QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if((ret = i.data()->waitUntilGoing(errorData)) != Processor::NoError)
		{	if(errorProc) *errorProc = i.data();
			for(QMap<QString, AbstractProcessor *>::ConstIterator j = theProcessors.begin(); j != i; j++)
				j.data()->stop();
			return ret;
		}
	return Processor::NoError;
}

void AbstractProcessorGroup::stop(const bool resetToo) const
{
	for(QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.data()->stop();
	if(resetToo) reset();
}

void AbstractProcessorGroup::reset() const
{
	for(QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.data()->reset();
}

};
