/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
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
	return SignalType::sameAsBE(cmp) && dynamic_cast<const Spectrum *>(cmp)->theStep == theStep && dynamic_cast<const Spectrum *>(cmp)->theMin == theMin && dynamic_cast<const Spectrum *>(cmp)->theMax == theMax;
}

void Spectrum::serialise(QSocketSession &sink) const
{
	SignalType::serialise(sink);
	sink.safeSendWord(theStep);
	sink.safeSendWord(theMin);
	sink.safeSendWord(theMax);
}

void Spectrum::deserialise(QSocketSession &source)
{
	SignalType::deserialise(source);
	theStep = source.safeReceiveWord<float>();
	theMin = source.safeReceiveWord<float>();
	theMax = source.safeReceiveWord<float>();
}

}
