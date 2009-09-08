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
