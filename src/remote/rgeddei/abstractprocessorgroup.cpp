/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
 *
 * This file is part of Exscalibar.
 *
 * Exscalibar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Exscalibar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Exscalibar.  If not, see <http://www.gnu.org/licenses/>.
 */

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

bool AbstractProcessorGroup::exists(const QString &name)
{
	return theProcessors.count(name);
}

AbstractProcessor &AbstractProcessorGroup::get(const QString &name)
{
	if (theProcessors.count(name))
		return *(theProcessors[name]);
	else
	{	qFatal("*** FATAL: Attempting to attain a Processor object that does not exist.\n"
			   "           You used the non-existant name %s. Bailing.", qPrintable(name));
		return *((AbstractProcessor *)0);
	}
}

AbstractDomProcessor &AbstractProcessorGroup::dom(const QString &name)
{
	if (theProcessors.count(name))
		return *dynamic_cast<AbstractDomProcessor *>(theProcessors[name]);
	else
	{	qFatal("*** FATAL: Attempting to attain a DomProcessor object that does not exist.\n"
			   "           You used the non-existant name %s. Bailing.", qPrintable(name));
		return *((AbstractDomProcessor *)0);
	}
}

void AbstractProcessorGroup::add(AbstractProcessor *o)
{
	for (QMap<QString, AbstractProcessor *>::Iterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if (i.value() == o) return;
		else if (i.key() == o->name())
			qDebug("*** ERROR: You are using the same name for multiple AbstractProcessor objects.\n"
				   "           You have to use a unique name for each object or grouping will not\n"
				   "           work properly.");
	theProcessors[o->name()] = o;
	o->setGroup(*this);
}

void AbstractProcessorGroup::remove(AbstractProcessor *o)
{
	for (QMap<QString, AbstractProcessor *>::Iterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if (i.value() == o)
		{	theProcessors.erase(i);
			o->setNoGroup();
			return;
		}
}

void AbstractProcessorGroup::deleteAll()
{
	QMap<QString, AbstractProcessor *> theProcessorsBU = theProcessors;
	for (QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessorsBU.begin(); i != theProcessorsBU.end(); i++)
	{	if (MESSAGES) qDebug("Deleting %s...", qPrintable(i.value()->name()));
		delete i.value();
	}
	theProcessors.clear();
}

void AbstractProcessorGroup::disconnectAll()
{
	for (QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.value()->disconnectAll();
}

void AbstractProcessorGroup::init() const
{
	for (QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.value()->init();
}

bool AbstractProcessorGroup::go(bool waitUntilGoing) const
{
	for (QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if (!i.value()->go())
		{	for (QMap<QString, AbstractProcessor *>::ConstIterator j = theProcessors.begin(); j != i; j++)
				j.value()->stop();
			return false;
		}
	if (!waitUntilGoing) return true;
	AbstractProcessor *errorProc;
	int errorData;
	Processor::ErrorType e = this->waitUntilGoing(&errorProc, &errorData);
	if (e != Processor::NoError)
	{	qWarning("Error starting processors: Code %d (name=%s, data=%d).", (int)e, qPrintable(errorProc->name()), errorData);
		return false;
	}
	return true;
}

Processor::ErrorType AbstractProcessorGroup::waitUntilGoing(AbstractProcessor **errorProc, int *errorData) const
{
	Processor::ErrorType ret;
	for (QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		if ((ret = i.value()->waitUntilGoing(errorData)) != Processor::NoError)
		{	if (errorProc) *errorProc = i.value();
			for (QMap<QString, AbstractProcessor *>::ConstIterator j = theProcessors.begin(); j != i; j++)
				j.value()->stop();
			return ret;
		}
	return Processor::NoError;
}

void AbstractProcessorGroup::stop(bool resetToo) const
{
	for (QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.value()->stop();
	if (resetToo) reset();
}

void AbstractProcessorGroup::reset() const
{
	for (QMap<QString, AbstractProcessor *>::ConstIterator i = theProcessors.begin(); i != theProcessors.end(); i++)
		i.value()->reset();
}

}
