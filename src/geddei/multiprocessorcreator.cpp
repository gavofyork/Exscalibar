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

#include "processorfactory.h"
#include "subprocessorfactory.h"
#include "domprocessor.h"
#include "multiprocessorcreator.h"

namespace Geddei
{

Processor *FactoryCreator::newProcessor() const
{
	return ProcessorFactory::create(theType);
}

Processor *SubFactoryCreator::newProcessor() const
{
	return SubProcessorFactory::createDom(theType);
}

}
