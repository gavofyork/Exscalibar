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

#include <cstdlib>

#include <qstringlist.h>
#include <qapplication.h>

#include "globals.h"
#include "processorfactory.h"

namespace Geddei
{

QFactoryManager<Processor> *ProcessorFactory::theOne = 0;

QFactoryManager<Processor> &ProcessorFactory::factory()
{
	if (!theOne)
	{	theOne = new QFactoryManager<Processor>;
		theOne->reloadAll(getPaths());
	}
	return *theOne;
}

Processor *ProcessorFactory::create(const QString &type)
{
	if (!available(type))
		qWarning("*** ERROR: You are attempting to create a Processor type that is not\n"
				 "           available (%s).", qPrintable(type));
	return factory()[type];
}

}
