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
#include "domprocessor.h"
#include "subprocessorfactory.h"
#include "combination.h"

namespace Geddei
{

QFactoryManager<SubProcessor> *SubProcessorFactory::theOne = 0;

QFactoryManager<SubProcessor> &SubProcessorFactory::factory()
{
	if (!theOne)
	{	theOne = new QFactoryManager<SubProcessor>;
		theOne->reloadAll(getPaths());
	}
	return *theOne;
}

bool SubProcessorFactory::available(const QString &type)
{
	if (type.contains("&"))
		return available(type.section("&", 0, 0)) && available(type.section("&", 1));
	else
		return factory().isAvailable(type);
}

int SubProcessorFactory::versionId(const QString &type)
{
	if (type.contains("&"))
		return min(versionId(type.section("&", 0, 0)), versionId(type.section("&", 1)));
	else
		return factory().getVersion(type);
}


SubProcessor *SubProcessorFactory::create(const QString &type)
{
	if (!available(type))
		qFatal("*** FATAL: You are attempting to create a SubProcessor type that is not\n"
		       "           available (%s).", type.latin1());
	if (type.contains("&"))
		return new Combination(create(type.section("&", 0, 0)), create(type.section("&", 1)));
	else
		return factory()[type];
}

DomProcessor *SubProcessorFactory::createDom(const QString &type)
{
	if (!available(type))
		qFatal("*** FATAL: You are attempting to create a SubProcessor type that is not\n"
		       "           available (%s).", type.latin1());
	return new DomProcessor(create(type));
}

};
