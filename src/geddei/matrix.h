/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _SIGNALTYPES_MATRIX_H
#define _SIGNALTYPES_MATRIX_H

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
 * @brief A SignalType refinement for describing 2-D matrix data.
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class DLLEXPORT Matrix: public SignalType
{
	virtual void serialise(QSocketSession &sink) const;
	virtual void deserialise(QSocketSession &source);
	virtual const uint id() const { return 3; }
	virtual SignalType *copyBE() const { return new Matrix(theWidth, theHeight, theFrequency); }
	virtual const bool sameAsBE(const SignalType *cmp) const;

protected:
	uint theWidth; ///< Width of the represented matrix in elements.
	uint theHeight; ///< Height of the represented matrix in elements.
	float thePitchWidth; ///< Pitch width of the represented matrix.
	float thePitchHeight; ///< Pitch height of the represented matrix.

public:
	/**
	 * Get the number of columns in the matrix this object represents.
	 *
	 * @return The number of elements in a row (number of columns)
	 */
	const uint width() const { return theWidth; }

	/**
	 * Get the number of rows in the matrix this object represents.
	 *
	 * @return The number of elements in a column (number of rows).
	 */
	const uint height() const { return theHeight; }

	/**
	 * Get the frequency represented by the progression of elements in a row.
	 *
	 * @return The pitch width of the matrix elements.
	 */
	const float pitchWidth() const { return thePitchWidth; }

	/**
	 * Get the frequency represented by the progression of elements in a
	 * column.
	 *
	 * @return The pitch height of the matrix elements.
	 */
	const float pitchHeight() const { return thePitchHeight; }

	/**
	 * Constrictor for a new matrix whose row size is equal to column size.
	 *
	 * @param width The number of columns.
	 * @param height The number of rows.
	 * @param frequency The number of matrices that are required to represent a
	 * second of signal time.
	 * @param pitchWidth The theoretical number of elements in a row that would
	 * represent a second in signal time. Of course this property may be left
	 * as its default (0) if it makes no sense for the data.
	 * @param pitchHeight The theoretical number of elements in a column that
	 * would represent a second in signal time. Of course this property may be
	 * left as its default (0) if it makes no sense for the data.
	 */
	Matrix(const uint width = 1, const uint height = 1, const float frequency = 0, const float pitchWidth = 0, const float pitchHeight = 0) : SignalType(width * height, frequency), theWidth(width), theHeight(height), thePitchWidth(pitchWidth), thePitchHeight(pitchHeight) {}
};

/** @ingroup SignalTypes
 * @brief A SignalType refinement for describing 2-D square matrix data.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * This is very similar to the Matrix class except that it describes only data
 * representing *square* martixes; that is the width and the
 * height are equal.
 */
class DLLEXPORT SquareMatrix: public Matrix
{
	virtual const uint id() const { return 4; }
	virtual SignalType *copyBE() const { return new SquareMatrix(theWidth, theFrequency); }

public:
	/**
	 * Get the size of the matrix this SignalType represents.
	 *
	 * The sample size is therefore equal to size() * size().
	 *
	 * @return The number of elements in every row and column of the matrix.
	 */
	const uint size() const { return theWidth; }

	/**
	 * Get the frequency represented by the progression of elements in either
	 * the row or column.
	 *
	 * @return The pitch of the matrix elements.
	 */
	const float pitch() const { return thePitchWidth; }

	/**
	 * Constrictor for a new matrix whose row size is equal to column size.
	 *
	 * @param size The number of rows (or columns).
	 * @param frequency The number of matrices that are required to represent a
	 * second of signal time.
	 * @param pitch The theoretical number of elements in a row that would
	 * represent a second in signal time. Of course this property may be left
	 * as its default (0) if it makes no sense for the data.
	 */
	SquareMatrix(const uint size = 1, const float frequency = 0., const float pitch = 0.) : Matrix(size, size, frequency, pitch, pitch) {}
};

};

#endif
