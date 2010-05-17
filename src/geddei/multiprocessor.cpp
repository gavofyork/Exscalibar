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

#include "multiprocessorcreator.h"
#include "processor.h"
#include "multiprocessor.h"

#define MESSAGES 0

namespace Geddei
{

MultiProcessor::MultiProcessor(MultiProcessorCreator *c):
	Groupable				(),
	theDeferredInit			(false),
	theGivenMultiplicity	(Undefined),
	theCreator				(c),
	theSource				(0),
	theIsInitialised		(false)
{
}

MultiProcessor::~MultiProcessor()
{
	// delete all processors
	for (uint i = 0; i < (uint)theProcessors.count(); i++)
		delete theProcessors[i];
	delete theCreator;
}

QString MultiProcessor::name() const
{
	return theDeferredName;
}

bool MultiProcessor::confirmTypes()
{
	foreach (Processor* i, theProcessors)
		if (!i->confirmTypes())
			return false;
	return true;
}

bool MultiProcessor::go()
{
	foreach (Processor* i, theProcessors)
		if (!i->go())
			return false;
	return true;
}

void MultiProcessor::stop()
{
	foreach (Processor* i, theProcessors)
		i->stop();
}

void MultiProcessor::update(Properties const& _p)
{
	foreach (Processor* i, theProcessors)
		i->update(_p);
}

bool MultiProcessor::isRunning() const
{
	foreach (Processor* i, theProcessors)
		if (i->isRunning())
			return true;
	return false;
}

void MultiProcessor::reset()
{
	foreach (Processor* i, theProcessors)
		i->reset();
}

void MultiProcessor::disconnectAll()
{
	foreach (Processor* i, theProcessors)
		i->disconnectAll();
}

Processor::ErrorType MultiProcessor::waitUntilGoing(int *errorData)
{
	Processor::ErrorType ret;
	foreach (Processor* i, theProcessors)
		if ((ret = i->waitUntilGoing(errorData)) != Processor::NoError)
			return ret;
	return Processor::NoError;
}

Connection::Tristate MultiProcessor::isGoingYet()
{
	Connection::Tristate ret = Connection::Succeeded;
	foreach (Processor* i, theProcessors)
		if (i->isGoingYet() == Connection::Failed)
			return Connection::Failed;
		else if (i->isGoingYet() == Connection::Pending)
			ret = Connection::Pending;
	return ret;
}

	/*
		return _processors_ composed _errorType_ avoiding NoError [defaulting NoError]

			   .#=  _processors_
		return {  errorType()
			   `#=  != NoError

	*/

long int MultiProcessor::errorData() const
{
	foreach (Processor* i, theProcessors)
		if (!i->errorType() != NoError)
			return i->errorData();
	return 0;
}

Groupable::ErrorType MultiProcessor::errorType() const
{
	foreach (Processor* i, theProcessors)
		if (!i->errorType() != NoError)
			return i->errorType();
	return NoError;
}

QString MultiProcessor::error() const
{
	foreach (Processor* i, theProcessors)
		if (!i->error().isEmpty())
			return i->error();
	return QString::null;
}

void MultiProcessor::onMultiplicitySet(uint _m)
{
	if (MESSAGES) qDebug("MultiProcessor::onMultiplicitySet(%d)", _m);
	if (theDeferredInit)
	{	if (MESSAGES) qDebug("Deferred init - reinitialising...");
		theGivenMultiplicity = _m;
		doInit(theDeferredName, 0, theDeferredProperties);
	}
}

void MultiProcessor::resetMulti()
{
	if (MESSAGES) qDebug("MultiProcessor::init()");
	stop();
	reset();
	disconnectAll();
	if (!theDeferredProperties.keys().contains("Processors Multiplicity") || theDeferredProperties["Processors Multiplicity"] == 0)
	{
		theIsInitialised = false;
		theDeferredInit = true;
		theGivenMultiplicity = Undefined;
		doInit(theDeferredName, 0, theDeferredProperties);
		foreach (Processor* i, theProcessors)
			delete i;
		theProcessors.resize(0);
	}
}

/*
{
	return _is_initialised_ && _processors_ composed _confirm_types_ with &&
}
confirmTypes returns bool: <= theIsInitialised N N { theProcessors->confirmTypes }
*/


void MultiProcessor::doInit(QString const& _name, ProcessorGroup* _g, Properties const& _properties)
{
	if (MESSAGES) qDebug("MultiProcessor::init()");
	assert(!theIsInitialised);

	theDeferredName = _name;
	theDeferredProperties = _properties;

	if (!theDeferredInit)
	{
		Processor* p = theCreator->newProcessor();
		if (theDeferredProperties["Multiplicity"] == 0)
			theDeferredProperties["Multiplicity"] = 1;
		p->init("", theDeferredProperties);
		theNumMultiInputs = p->numInputs();
		theNumMultiOutputs = p->numOutputs();
		if (theNumMultiInputs == Undefined)
		{
			qWarning("Given an Undefined result for number of inputs on a multiproc'd processor (%s).", qPrintable(p->type()));
			theNumMultiInputs = 0;
		}
		if (theNumMultiOutputs == Undefined)
		{
			qWarning("Given an undefined result for number of outputs on a multiproc'd processor (%s).", qPrintable(p->type()));
			theNumMultiOutputs = 0;
		}
		delete p;
		if (_g)
			setGroup(*_g);
	}

	if (_properties.keys().contains("Processors Multiplicity") && _properties["Processors Multiplicity"].toInt() > 0)
		theGivenMultiplicity = _properties["Processors Multiplicity"].toInt();

	if (theGivenMultiplicity == Undefined)
	{
		if (MESSAGES) qDebug("Deferring...");
		theDeferredInit = true;
		return;
	}

	if (MESSAGES) qDebug("Initialising (M=%d)...", theGivenMultiplicity);
	theProcessors.resize(theGivenMultiplicity);
	for (uint i = 0; i < (uint)theProcessors.count(); i++)
	{
		theProcessors[i] = theCreator->newProcessor();
		theProcessors[i]->doInit(_name + QString::number(i), 0, _properties);
	}
	theIsInitialised = true;
	theDeferredInit = false;
}

}

#undef MESSAGES
