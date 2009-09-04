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

#include <qdatetime.h>

#include "domprocessor.h"
#include "combination.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

Combination::Combination(SubProcessor *x, SubProcessor *y) : SubProcessor(x->theType + "&" + y->theType), theX(x), theY(y), theResident(0)
{
}

Combination::~Combination()
{
	delete theX;
	delete theY;
}

void Combination::processChunks(const BufferDatas &in, BufferDatas &out, const uint ) const
{
	uint xc = (in[0].samples() - theX->theIn) / theX->theStep + 1;
	uint samplesNeeded = xc * theX->theOut;
	uint yc = (samplesNeeded - theY->theIn) / theY->theStep + 1;
	if(!theResident || theResident->samples() < samplesNeeded)
	{
		delete theResident;
		theResident = new BufferData(samplesNeeded * theInterScope, theInterScope);
	}
	BufferDatas d(1);
	d.setData(0, theResident);
	theX->processChunks(in, d, xc);
	theY->processChunks(d, out, yc);
	d.setData(0, 0);
}

PropertiesInfo Combination::specifyProperties() const
{
	return PropertiesInfo(theX->specifyProperties())(theY->specifyProperties());
}

void Combination::initFromProperties(const Properties &p)
{
	// needs some way to deliver to either: namespaces/numeric ids?
	theX->initFromProperties(p);
	theY->initFromProperties(p);
	if(theY->theIn >= theX->theOut && theY->theStep >= theX->theOut && !(theY->theIn % theX->theOut) && !(theY->theStep % theX->theOut) && theX->theNumOutputs == 1 && theY->theNumInputs == 1)
	{
		setupIO(theX->theNumInputs, theY->theNumOutputs, theX->theIn + theX->theStep * (theY->theIn / theX->theOut - 1), theX->theStep * theY->theStep / theX->theOut, theY->theOut);
		if(MESSAGES) qDebug("Setting up IO: %d->%d, %d/%d => %d", theNumInputs, theNumOutputs, theIn, theStep, theOut);
	}
	else
		qDebug("WARNING: Could not initialise - incompatible SubProcessors: %s(%d, %d, %d) === %s(%d, %d, %d).", theX->theType.latin1(), theX->theIn, theX->theStep, theX->theOut, theY->theType.latin1(), theY->theIn, theY->theStep, theY->theOut);
}

const bool Combination::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	SignalTypeRefs r(1);
	if(theX->verifyAndSpecifyTypes(inTypes, r) && r.populated(0))
	{	theInterScope = r[0].scope();
		return theY->verifyAndSpecifyTypes(r, outTypes);
	}
	else return false;
}

}
