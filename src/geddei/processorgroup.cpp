/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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

#include "processor.h"
#include "domprocessor.h"
#include "processorgroup.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

ProcessorGroup::ProcessorGroup(bool adopt) : theAdopt(adopt)
{
}

ProcessorGroup::~ProcessorGroup()
{
	if (theAdopt)
		deleteAll();
}

bool ProcessorGroup::exists(const QString &name)
{
	return theMembers.count(name);
}

Processor &ProcessorGroup::get(const QString &name)
{
	if (theMembers.count(name))
		return *dynamic_cast<Processor*>(theMembers[name]);
	else
	{	qFatal("*** FATAL: Attempting to attain a Processor object that does not exist.\n"
			   "           You used the non-existant name %s. Bailing.", qPrintable(name));
		return *((Processor*)(0));
	}
}

DomProcessor &ProcessorGroup::dom(const QString &name)
{
	if (theMembers.count(name))
		return *dynamic_cast<DomProcessor*>(theMembers[name]);
	else
	{	qFatal("*** FATAL: Attempting to attain a DomProcessor object that does not exist.\n"
			   "           You used the non-existant name %s. Bailing.", qPrintable(name));
		return *((DomProcessor*)(0));
	}
}

MultiProcessor &ProcessorGroup::multi(const QString &name)
{
	if (theMembers.count(name))
		return *dynamic_cast<MultiProcessor*>(theMembers[name]);
	else
	{	qFatal("*** FATAL: Attempting to attain a MultiProcessor object that does not exist.\n"
			   "           You used the non-existant name %s. Bailing.", qPrintable(name));
		return *((MultiProcessor*)(0));
	}
}

void ProcessorGroup::add(Groupable *o)
{
	if (theMembers.keys().contains(o->name()))
		qDebug("*** ERROR: You are using the same name for multiple Processor objects.\n"
			   "           You have to use a unique name for each object or grouping will not\n"
			   "           work properly.");
	else if (theMembers.values().contains(o))
		return;
	theMembers[o->name()] = o;
	o->setGroup(*this);
}

void ProcessorGroup::remove(Groupable *o)
{
	for (QMap<QString, Groupable*>::Iterator i = theMembers.begin(); i != theMembers.end(); i++)
		if (i.value() == o)
		{	theMembers.erase(i);
			o->setNoGroup();
			return;
		}
}

bool ProcessorGroup::confirmTypes() const
{
	bool ret = true;
	if (MESSAGES) qDebug("ProcessorGroup::confirmTypes()");
	foreach (Groupable* i, theMembers.values())
	{
		if (MESSAGES) qDebug("ProcessorGroup::confirmTypes(): Confirming %p...", i);
		if (!i->confirmTypes())
		{
			m_errorProc = i;
			ret = false;
		}
	}
	return ret;
}

void ProcessorGroup::deleteAll()
{
	foreach (Groupable* i, theMembers.values())
		delete i;
	theMembers.clear();
}

void ProcessorGroup::disconnectAll()
{
	foreach (Groupable* i, theMembers.values())
		i->disconnectAll();
}

bool ProcessorGroup::go(bool _waitUntilGoing) const
{
	bool ret = true;
//	if (!confirmTypes()) return false;
	foreach (Groupable* i, theMembers.values())
		if (!i->go())
			ret = false;
	if (ret && _waitUntilGoing)
		return !ProcessorGroup::waitUntilGoing();
	return ret;
}

Processor::ErrorType ProcessorGroup::waitUntilGoing(Groupable **errorProc, int *errorData) const
{
	Processor::ErrorType ret;
	foreach (Groupable* i, theMembers)
		if ((ret = i->waitUntilGoing(errorData)) != Processor::NoError)
		{	if (errorProc) *errorProc = i;
			foreach (Groupable* j, theMembers)
				if (j == i)
					break;
				else
					j->stop();
			return ret;
		}
	return Processor::NoError;
}

const QString ProcessorGroup::error() const
{
	QString ret;
	foreach (Groupable* i, theMembers)
	{
		Processor::ErrorType e = i->errorType();
		if (e != Processor::NoError && e != Processor::RecursiveFailure && e != Processor::Pending)
		{	if (ret.isEmpty()) ret += "  ";
			ret += i->name() + ": " + i->error();
		}
	}
	return ret;
}

void ProcessorGroup::stop(bool resetToo) const
{
	foreach (Groupable* i, theMembers)
		i->stop();
	if (resetToo)
		reset();
}

void ProcessorGroup::reset() const
{
	foreach (Groupable* i, theMembers)
		i->reset();
}

}

#undef MESSAGES
