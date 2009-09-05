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

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "signaltypeptrs.h"
#include "bufferdata.h"
#include "processor.h"
#include "buffer.h"
using namespace geddei;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
// TODO: Search & replace "Template" with your new class name.
class Template : public Processor
{
	// TODO: Declare any processing variables here.

	virtual void processor();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);

public:
	Template() : Processor("Template") {}
};

void Template::processor()
{
	// TODO: Initialisations.
	while (guard())
	{
		// TODO: Main loop.
	}
}

void Template::processorStopped()
{
	// TODO: Deinitialisations.
}

bool Template::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	// TODO: Check input types and return correct output types.
	// Don't forget to make copy()s!
	outTypes.append(inTypes.first()->copy());
	return true;
}

void Template::initFromProperties(const Properties &properties)
{
	// TODO: setupIO and setupDimensions according to properties if neccessary.
	setupIO(1, 1);
}

PropertiesInfo Template::specifyProperties() const
{
	// TODO: Specify properties.
	return PropertiesInfo();
}

FACTORY_TYPE(Processor);
EXPORT_CLASS(Template, 0,1,0);
