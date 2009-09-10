/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef RGEDDEIHOSTPROCESSORFORWARDER_H
#define RGEDDEIHOSTPROCESSORFORWARDER_H

#include <string>
using namespace std;

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "commcodes.h"
#include "processorforwarder.h"
#else
#include <rgeddei/commcodes.h>
#include <geddei/processorforwarder.h>
#endif
using namespace Geddei;

namespace rGeddei
{

/** @internal @ingroup rGeddei
 * @brief Realisation of a ProcessorForwarder object.
 * @author Gav Wood <gav@kde.org>
 *
 * This implements the virtual void method from ProcessorForwarder in order to
 * use the resident SessionServer object to determine Processors from key and
 * name.
 */
class DLLEXPORT HostProcessorForwarder: public ProcessorForwarder
{
	virtual Processor *lookup(uint key, const QString &name);
	
public:
	HostProcessorForwarder(uint port = RGEDDEI_PORT) : ProcessorForwarder(port) {}
};

};

#endif
