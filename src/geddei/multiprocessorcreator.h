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

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "processor.h"
#else
#include <geddei/processor.h>
#endif
using namespace Geddei;

namespace Geddei
{

/** @ingroup Geddei
 * @brief Abstract class defining interface for creating Processor objects.
 * @author Gav Wood <gav@kde.org>
 *
 * This class facilitates creation of new Processor objects in customisable
 * manners. It provides one virtual method which when overridden can be coded
 * to provide a new Processor object of the class writers choice.
 *
 * Four simple derivations are available: two for creation of objects by
 * templating the creation class according to the Processor class you want.
 * The other two allow creation of objects through factories and thus a string
 * pertaining to the class's type must be provided at object construction.
 *
 * Two of each are provided for creation of basic Processor objects as well
 * as DomProcessor objects with a specific SubProcessor.
 */
class DLLEXPORT MultiProcessorCreator
{
	friend class MultiProcessor;
	virtual Processor *newProcessor() const = 0;
protected:
	virtual ~MultiProcessorCreator() {}
};

/** @ingroup Geddei
 * @brief MultiProcessorCreator-derivation for creating templated Processor class.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be templated so as to allow creation of new Processor objects
 * of the given class.
 */
template<class X>
class DLLEXPORT BasicCreator: public MultiProcessorCreator
{
	virtual Processor *newProcessor() const { return dynamic_cast<Processor *>(new X); }
};

/** @ingroup Geddei
 * @brief MultiProcessorCreator-derivation for creating templated SubProcessor class.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be templated so as to allow creation of new DomProcessor
 * objects whose primary SubProcessor is of the given class.
 */
template<class X>
class DLLEXPORT BasicSubCreator: public MultiProcessorCreator
{
	virtual Processor *newProcessor() const { return dynamic_cast<Processor *>(new DomProcessor(new X)); }
};

/** @ingroup Geddei
 * @brief MultiProcessorCreator-derivation for Factory creation of Processors.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be constructed so as to allow creation of new Processor
 * objects of the class given by the type @a type . @a type must be an
 * available type according to the resident ProcessorFactory singleton.
 */
class DLLEXPORT FactoryCreator: public MultiProcessorCreator
{
	QString theType;
	virtual Processor *newProcessor() const;

public:
	/**
	 * Basic constructor. Creates a new FactoryCreator object which will
	 * function as a MultiProcessorCreator class to produce objects from the
	 * ProcessorFactory of type @a type.
	 *
	 * @param type The type of Processor class that this will create. @a type
	 * must be available according to the relevant ProcessorFactory singleton.
	 */
	FactoryCreator(const QString &type) : theType(type) {}
};

/** @ingroup Geddei
 * @brief MultiProcessorCreator-derivation for Factory creation of SubProcessors.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be constructed so as to allow creation of new DomProcessor
 * objects whose primary SubProcessor object is of the class given by the type
 * @a type . @a type must be an available type according to the resident
 * SubProcessorFactory singleton.
 */
class DLLEXPORT SubFactoryCreator: public MultiProcessorCreator
{
	QString theType;
	virtual Processor *newProcessor() const;

public:
	/**
	 * Basic constructor. Creates a new SubFactoryCreator object which will
	 * function as a MultiProcessorCreator class to produce DomProcessor objects
	 * whose primary SubProcessor is from the SubProcessorFactory of type
	 * @a type.
	 *
	 * @param type The type of SubProcessor class that will be the primary of
	 * the DomProcessor object this will create. This must be available
	 * according to the relevant SubProcessorFactory singleton.
	 */
	SubFactoryCreator(const QString &type) : theType(type) {}
};

}
