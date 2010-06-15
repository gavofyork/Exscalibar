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

#pragma once

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

}
