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

#include "matrix.h"

namespace SignalTypes
{

bool Matrix::sameAsBE(const SignalType *cmp) const
{
	return SignalType::sameAsBE(cmp) &&
		theWidth == dynamic_cast<const Matrix *>(cmp)->theWidth &&
		theHeight == dynamic_cast<const Matrix *>(cmp)->theHeight &&
		thePitchWidth == dynamic_cast<const Matrix *>(cmp)->thePitchWidth &&
		thePitchHeight == dynamic_cast<const Matrix *>(cmp)->thePitchHeight;
}

void Matrix::serialise(QSocketSession &sink) const
{
	SignalType::serialise(sink);
	sink.safeSendWord((uint32_t)theWidth);
	sink.safeSendWord((uint32_t)theHeight);
}

void Matrix::deserialise(QSocketSession &source)
{
	SignalType::deserialise(source);
	theWidth = source.safeReceiveWord<uint32_t>();
	theHeight = source.safeReceiveWord<uint32_t>();
}

};
