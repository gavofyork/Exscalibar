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

#pragma once

#include <qstring.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "properties.h"
#else
#include <geddei/properties.h>
#endif
using namespace Geddei;

namespace Geddei
{

class ProcessorGroup;

/** @internal @ingroup Geddei
 * @brief Abstract class for defining interface to a fundamentally multiplicitive object.
 * @author Gav Wood <gav@kde.org>
 *
 */
class Multiplicative
{
protected:
	virtual void setSourceMultiplicity(uint multiplicity) = 0;
	virtual void setSinkMultiplicity(uint multiplicity) = 0;

	virtual void onMultiplicitySet(uint) {}

public:
	virtual bool knowMultiplicity() const = 0;
	virtual uint multiplicity() const = 0;
	virtual void doInit(const QString &name, ProcessorGroup *g = 0, const Properties &properties = Properties()) = 0;

	void setMultiplicity(uint multiplicity);

	Multiplicative() {}
	virtual ~Multiplicative() {}
};

}
