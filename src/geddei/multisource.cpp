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

#include <cassert>
using namespace std;

#include "multisink.h"
#include "multisource.h"
#include "processor.h"

#define MESSAGES 0

namespace Geddei
{

void MultiSource::disconnect()
{
	while (theDeferreds.size())
		if (MultiSink* s = theDeferreds.takeLast().s)
			s->removeDeferral(this);
	if (knowMultiplicity())
		for (uint i = 0; i < multiplicity(); i++)
			for (uint j = 0; j < numMultiOutputs(); j++)
				sourcePort(i, j).disconnect();
}

Connection::Tristate MultiSource::deferConnect(uint _op, MultiSink* _sink, uint _ip, uint _bufferSize)
{
	// If our multiplicity is explicitly defined
	if (knowMultiplicity())
		// If the sink's multiplicity is explicitly defined
		if (_sink->knowMultiplicity())
		{	if (multiplicity() != _sink->multiplicity())
			{	qWarning("*** MultiSource::deferConnect(): Multiplicity incompatibility: Attempting to connect %d outputs to %d inputs!", multiplicity(), _sink->multiplicity());
				if (dynamic_cast<Processor *>(this)) qWarning("                                 Name of source: %s", qPrintable(dynamic_cast<Processor *>(this)->name()));
				if (dynamic_cast<Processor *>(_sink)) qWarning("                                 Name of sink:   %s", qPrintable(dynamic_cast<Processor *>(_sink)->name()));
				// TODO: error reporting code.
				return Connection::Failed;
			}
		}
		else
			_sink->setMultiplicity(multiplicity());
	else
		if (_sink->knowMultiplicity())
			setMultiplicity(_sink->multiplicity());
		else
		{	theDeferreds << Deferred(_op, _sink, _ip, _bufferSize);
			_sink->appendDeferral(this);
			return Connection::Pending;
		}
	return Connection::Succeeded;
}

void MultiSource::setSourceMultiplicity(uint multiplicity)
{
	if (MESSAGES) qDebug("MultiSource::setSourceMultiplicity(%d) DC=%d", multiplicity, theDeferreds.count());
	assert(knowMultiplicity());

	QList<Deferred> ds = theDeferreds;
	theDeferreds.clear();
	foreach (Deferred d, ds)
		if (d.s)
			connect(d.op, d.s, d.ip, d.bs);
		else if (d.ip == 0)
			share(d.op);
		else if (d.ip == 1)
			split(d.op);
}

Connection::Tristate MultiSource::connect(uint _op, MultiSink* _sink, uint _ip, uint _bufferSize)
{
	// TODO: Warn & exit.
	assert(!theConnected);

	if (Connection::Tristate t = deferConnect(_op, _sink, _ip, _bufferSize))
		return t;

	assert(_sink->knowMultiplicity());
	assert(knowMultiplicity());
	connectCheck();

	if (MESSAGES) qDebug("MultiSource::connect(): %d -> %d", multiplicity(), _sink->multiplicity());

	if (_sink->multiplicity() != multiplicity())
		qFatal("MultiProcessor: Error in connecting to sink.");

	for (uint i = 0; i < multiplicity(); i++)
		sourcePort(i, _op) >>= _sink->sinkPort(i, _ip);

	return Connection::Succeeded;
}

Connection::Tristate MultiSource::split(uint _op)
{
	if (knowMultiplicity())
	{
		for (uint i = 0; i < multiplicity(); i++)
			sourcePort(i, _op).split();
		return Connection::Succeeded;
	}
	else
	{
		theDeferreds << Deferred(_op, 0, 1, 0);
		return Connection::Pending;
	}
}

Connection::Tristate MultiSource::share(uint _op)
{
	if (knowMultiplicity())
	{
		for (uint i = 0; i < multiplicity(); i++)
			sourcePort(i, _op).share();
		return Connection::Succeeded;
	}
	else
	{
		theDeferreds << Deferred(_op, 0, 0, 0);
		return Connection::Pending;
	}
}

}

#undef MESSAGES
