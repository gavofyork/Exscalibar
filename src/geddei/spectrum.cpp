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

#include "qsocketsession.h"

#include "spectrum.h"

namespace SignalTypes
{

bool Spectrum::sameAsBE(const SignalType *cmp) const
{
	return SignalType::sameAsBE(cmp) && dynamic_cast<const Spectrum *>(cmp)->theStep == theStep;
}

void Spectrum::serialise(QSocketSession &sink) const
{
	SignalType::serialise(sink);
	sink.safeSendWord(theStep);
}

void Spectrum::deserialise(QSocketSession &source)
{
	SignalType::deserialise(source);
	theStep = source.safeReceiveWord<float>();
}

};
