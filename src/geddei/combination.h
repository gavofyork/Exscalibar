/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_COMBINATION_H
#define _GEDDEI_COMBINATION_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "subprocessor.h"
#else
#include <geddei/subprocessor.h>
#endif

namespace Geddei
{

class DLLEXPORT Combination: public SubProcessor
{
	SubProcessor *theX, *theY;
	
	uint theInterScope;
	mutable BufferData *theResident;
	
	virtual void processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const;
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &p);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	
public:
	Combination(SubProcessor *x, SubProcessor *y);
	~Combination();
};

};

#endif
