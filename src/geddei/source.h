/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_SOURCE_H
#define _GEDDEI_SOURCE_H

#include <qstring.h>

#include <exscalibar.h>

namespace Geddei
{

class LxConnection;
class Processor;

/** @internal @ingroup Geddei
 * @brief Abstract class defining operations on an object that produces data for a Connection.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 */
class DLLEXPORT Source
{
public:
	/**
	 * Checks if this object's thread should be stopped, and if so, stops it.
	 */
	virtual void checkExit() = 0;

	/**
	 * Checks the types (recursively if neccessary) and reports if all is ok.
	 * Sets up Connections types on its way.
	 * Note this is done automatically if neccessary at go(), but it's good
	 * practise to call it beforehand anyway.
	 */
	virtual bool confirmTypes() = 0;

	/**
	 * Make a record that @a me is now inhabiting @a port of this Source.
	 * If the source produces only one ubiquitous output stream, use the
	 * default port of 0.
	 */
	virtual void doRegisterOut(LxConnection *me, uint port = 0) = 0;

	/**
	 * Asserts that @a me has been previously registered at @a port on
	 * this source and deregisters it.
	 */
	virtual void undoRegisterOut(LxConnection *me, uint port = 0) = 0;

	/**
	 * @return is the processor who is driving this source. Should be left alone
	 * unless both this isn't a Processor and this can deliver a pointer to the
	 * processor from whom the data is being generated.
	 */
	virtual const Processor *processor() const;

	/**
	 * Virtual destructor.
	 */
	virtual ~Source() {}
};

}

#endif
