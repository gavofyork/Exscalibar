/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

#include "contiguous.h"
#else
#include <geddei/contiguous.h>
#endif
using namespace Geddei;

namespace Geddei
{

/** @ingroup SignalTypes
 * @brief A TransmissionType refinement for describing 1-D spectral data.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be used to describe a signal type that is a vector of
 * elements. The vector can be any size (dimensionality), and has two
 * significant attributes, the frequency by which each individual spectrum
 * arrives from the source, and the stepping between spectral bands.
 */
class DLLEXPORT Spectrum: public Contiguous
{
	TRANSMISSION_TYPE(Spectrum, Contiguous);

public:
	/**
	 * Create a new TransmissionType to represent a spectrum.
	 *
	 * @param size The number of bands in each spectrum.
	 * @param frequency The number of times the source will emit a spectrum
	 * per second of signal time (in Hz).
	 * @param step The increase in audio frequency (in Hz) per band. It is
	 * currently assumed that the signal is a monotonically stepped spectrum.
	 */
	inline Spectrum(uint bins = 1, float frequency = 1, float _max = 1.f, float _min = 0.f): Contiguous(bins, frequency, _max, _min) {}

	/**
	 * Determines the frequency represented by a specific band.
	 *
	 * @param band The band index.
	 * @return The midpoint frequency of band @a band.
	 */
	virtual float bandFrequency(float) const { return 0.f; }

	/**
	 * Determines the frequency represented by a specific band.
	 *
	 * @param band The band index.
	 * @return The midpoint frequency of band @a band.
	 */
	virtual uint frequencyBand(float) const { return 0; }

	/**
	 * Gets the number of bands in the spectra of the signal to which this
	 * type refers.
	 *
	 * @return The number of band bins.
	 */
	virtual uint bins() const { return arity(); }
	virtual void setBins(uint _n) { setArity(_n); }

	/**
	 * Gets the Nyquist frequency (the highest frequency that can be
	 * represented by this spectrum.
	 *
	 * @return The Nyquist frequency.
	 */
	virtual float nyquist() const { return 0.f; }

	virtual QString info() const { return QString("<div><b>Spectrum</b></div><div>Bin range: %1,%2-%3,%4 Hz</div>").arg(bandFrequency(0)).arg(bandFrequency(1)).arg(bandFrequency(size() - 2)).arg(bandFrequency(size() - 1)) + Contiguous::info(); }
};

/** @ingroup SignalTypes
 * @brief A TransmissionType refinement for describing 1-D spectral data.
 * @author Gav Wood <gav@kde.org>
 *
 * This class can be used to describe a signal type that is a vector of
 * elements. The vector can be any size (dimensionality), and has two
 * significant attributes, the frequency by which each individual spectrum
 * arrives from the source, and the stepping between spectral bands.
 */
class DLLEXPORT FreqSteppedSpectrum: public Spectrum
{
	TRANSMISSION_TYPE(FreqSteppedSpectrum, Spectrum);

public:
	/**
	 * Create a new TransmissionType to represent a spectrum.
	 *
	 * @param size The number of bands in each spectrum.
	 * @param frequency The number of times the source will emit a spectrum
	 * per second of signal time (in Hz).
	 * @param step The increase in audio frequency (in Hz) per band. It is
	 * currently assumed that the signal is a monotonically stepped spectrum.
	 */
	FreqSteppedSpectrum(uint bins = 1, float frequency = 1, float step = 1, float _max = 1.f, float _min = 0.f) : Spectrum(bins, frequency, _max, _min), theStep(step) {}

	/**
	 * Determines the frequency represented by a specific band.
	 *
	 * @param band The band index.
	 * @return The midpoint frequency of band @a band.
	 */
	virtual float bandFrequency(float _band) const { return _band * theStep; }

	/**
	 * Determines the frequency represented by a specific band.
	 *
	 * @param band The band index.
	 * @return The midpoint frequency of band @a band.
	 */
	virtual uint frequencyBand(float _freq) const { return ::min((uint)(_freq / theStep), (uint)(bins() - 1)); }

	/**
	 * Gets the Nyquist frequency (the highest frequency that can be
	 * represented by this spectrum.
	 *
	 * @return The Nyquist frequency.
	 */
	virtual float nyquist() const { return float(bins()) * theStep; }

	/**
	 * Gets the difference in audio frequency between each band in the
	 * spectra.
	 *
	 * @return The frequency stepping between bands.
	 */
	float step() const { return theStep; }

	virtual QString info() const { return QString("<div><b>FreqSteppedSpectrum</b></div><div>Step: %1 Hz</div>").arg(theStep) + Spectrum::info(); }

protected:
	float theStep; ///< Step between spectral bands of represented Spectrum in Hz.

	TT_1_MEMBER(theStep);
};

class DLLEXPORT PeriodSteppedSpectrum: public Spectrum
{
	TRANSMISSION_TYPE(PeriodSteppedSpectrum, Spectrum);

public:
	/**
	 * Create a new TransmissionType to represent a spectrum.
	 *
	 * @param size The number of bands in each spectrum.
	 * @param frequency The number of times the source will emit a spectrum
	 * per second of signal time (in Hz).
	 * @param step The increase in audio frequency (in s) per band. It is
	 * currently assumed that the signal is a monotonically stepped spectrum.
	 */
	PeriodSteppedSpectrum(uint bins = 1, float frequency = 1, float step = 1, float _max = 1.f, float _min = 0.f) : Spectrum(bins, frequency, _max, _min), theStep(step) {}

	/**
	 * Determines the frequency represented by a specific band.
	 *
	 * @param band The band index.
	 * @return The midpoint frequency of band @a band.
	 */
	virtual float bandFrequency(float _band) const { return 1.f / (_band * theStep); }

	/**
	 * Determines the frequency represented by a specific band.
	 *
	 * @param band The band index.
	 * @return The midpoint frequency of band @a band.
	 */
	virtual uint frequencyBand(float _freq) const { return clamp((uint)(bins() - 1), 0u, (uint)(theStep / ::max(0.0001f, _freq))); }

	/**
	 * Gets the Nyquist frequency (the highest frequency that can be
	 * represented by this spectrum.
	 *
	 * @return The Nyquist frequency.
	 */
	virtual float nyquist() const { return 1 / theStep; }

	/**
	 * Gets the difference in audio frequency between each band in the
	 * spectra.
	 *
	 * @return The period stepping between bands.
	 */
	float step() const { return theStep; }

	virtual QString info() const { return QString("<div><b>PeriodSteppedSpectrum</b></div><div>Step: %1 s</div>").arg(theStep) + Spectrum::info(); }

protected:
	float theStep; ///< Step between spectral bands of represented Spectrum in s.

	TT_1_MEMBER(theStep);
};


}
