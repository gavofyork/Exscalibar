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

#include "processor.h"
#include "multiprocessor.h"
#include "processorport.h"

namespace Geddei
{

void ProcessorPort::connect(const ProcessorPort &input) const
{
	theParent->connect(thePort, input.theParent, input.thePort, input.theBufferSize);
}

void ProcessorPort::connect(MultiProcessor &input, uint _ip) const
{
	share();
	for (uint i = 0; i < input.multiplicity(); i++)
		connect(input.sourcePort(i, _ip));
}

void ProcessorPort::disconnect() const
{
	theParent->disconnect(thePort);
}

void ProcessorPort::share() const
{
	theParent->share(thePort, theBufferSize);
}

void ProcessorPort::split() const
{
	theParent->split(thePort);
}

ProcessorPort &ProcessorPort::setSize(uint bufferSize)
{
	theBufferSize = bufferSize;
	return *this;
}

}
