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

#include <cstdlib>

#include <QStringList>
#include <QApplication>

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
	{
		theOne = new QFactoryManager<SubProcessor>;
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
			   "           available (%s).", qPrintable(type));
	if (type.contains("&"))
		return new Combination(create(type.section("&", 0, 0)), create(type.section("&", 1)));
	else
		return factory()[type];
}

DomProcessor *SubProcessorFactory::createDom(const QString &type)
{
	if (!available(type))
		qFatal("*** FATAL: You are attempting to create a SubProcessor type that is not\n"
			   "           available (%s).", qPrintable(type));
	return new DomProcessor(create(type));
}

}
