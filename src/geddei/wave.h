/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _SIGNALTYPES_WAVE_H
#define _SIGNALTYPES_WAVE_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "signaltype.h"
#else
#include <geddei/signaltype.h>
#endif
using namespace Geddei;

namespace SignalTypes
{

/** @ingroup SignalTypes
 * @brief A SignalType refinement for describing time-domain wave offset data.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * This class can be used to define a signal stream that comprises samples
 * of a single element, where each element represents an offset from a defined
 * zero.
 */
class DLLEXPORT Wave: public SignalType
{
	virtual uint id() const { return 1; }
	virtual SignalType *copyBE() const { return new Wave(theFrequency); }

public:
	/**
	 * The constructor.
	 *
	 * @param frequency The sampling frequency of the Wave. That is, the
	 * inverse of the delay between each sample. e.g. CD audio has a sample
	 * frequency of 44100, i.e. each sample represents a reading in time of
	 * 1/44100th of a second later than the last.
	 */
	Wave(float frequency = 0) : SignalType(1, frequency) {}
};

};

#endif
