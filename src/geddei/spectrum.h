/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _SIGNALTYPES_SPECTRUM_H
#define _SIGNALTYPES_SPECTRUM_H

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
 * @brief A SignalType refinement for describing 1-D spectral data.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be used to describe a signal type that is a vector of
 * elements. The vector can be any size (dimensionality), and has two
 * significant attributes, the frequency by which each individual spectrum
 * arrives from the source, and the stepping between spectral bands.
 */
class DLLEXPORT Spectrum: public SignalType
{
	virtual void serialise(QSocketSession &sink) const;
	virtual void deserialise(QSocketSession &source);
	virtual uint id() const { return 2; }
	virtual bool sameAsBE(const SignalType *cmp) const;
	virtual SignalType *copyBE() const { return new Spectrum(theScope, theFrequency, theStep, theMax, theMin); }

protected:
	float theStep; ///< Step between spectral bands of represented Spectrum.

public:
	/**
	 * Determines the frequency represented by a specific band.
	 *
	 * @param band The band index.
	 * @return The midpoint frequency of band @a band.
	 */
	float bandFrequency(uint band) const { return band * theStep; }

	/**
	 * Gets the number of bands in the spectra of the signal to which this
	 * type refers.
	 *
	 * @return The number of bands.
	 */
	uint size() const { return theScope; }

	/**
	 * Gets the difference in audio frequency between each band in the
	 * spectra.
	 *
	 * @return The frequency stepping between bands.
	 */
	float step() const { return theStep; }

	/**
	 * Gets the Nyquist frequency (the highest frequency that can be
	 * represented by this spectrum.
	 *
	 * @return The Nyquist frequency.
	 */
	float nyquist() const { return float(theScope) * theStep; }

	/**
	 * Create a new SignalType to represent a spectrum.
	 *
	 * @param size The number of bands in each spectrum.
	 * @param frequency The number of times the source will emit a spectrum
	 * per second of signal time (in Hz).
	 * @param step The increase in audio frequency (in Hz) per band. It is
	 * currently assumed that the signal is a monotonically stepped spectrum.
	 */
	Spectrum(uint size = 1, float frequency = 0, float step = 1, float _max = 1.f, float _min = 0.f) : SignalType(size, frequency, _max, _min), theStep(step) {}
};

};

#endif
