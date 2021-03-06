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
 * @brief A TransmissionType refinement for describing 2-D matrix data.
 * @author Gav Wood <gav@kde.org>
 */
class DLLEXPORT Matrix: public Contiguous
{
	TRANSMISSION_TYPE(Matrix, Contiguous);

public:
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
	Matrix(uint width = 1, uint height = 1, float frequency = 0, float pitchWidth = 0, float pitchHeight = 0) : Contiguous(width * height, frequency), theWidth(width), theHeight(height), thePitchWidth(pitchWidth), thePitchHeight(pitchHeight) {}

	/**
	 * Get the number of columns in the matrix this object represents.
	 *
	 * @return The number of elements in a row (number of columns)
	 */
	uint width() const { return theWidth; }

	/**
	 * Get the number of rows in the matrix this object represents.
	 *
	 * @return The number of elements in a column (number of rows).
	 */
	uint height() const { return theHeight; }

	/**
	 * Get the frequency represented by the progression of elements in a row.
	 *
	 * @return The pitch width of the matrix elements.
	 */
	float pitchWidth() const { return thePitchWidth; }

	/**
	 * Get the frequency represented by the progression of elements in a
	 * column.
	 *
	 * @return The pitch height of the matrix elements.
	 */
	float pitchHeight() const { return thePitchHeight; }

	virtual QString info() const { return QString("<div><b>Matrix</b></div><div>Dimensions: %1x%2</div><div>Pitch: %3x%4 s</div>").arg(theWidth).arg(theHeight).arg(thePitchWidth).arg(thePitchHeight) + Contiguous::info(); }

protected:
	uint theWidth; ///< Width of the represented matrix in elements.
	uint theHeight; ///< Height of the represented matrix in elements.
	float thePitchWidth; ///< Pitch width of the represented matrix.
	float thePitchHeight; ///< Pitch height of the represented matrix.

	TT_4_MEMBERS(theWidth, theHeight, thePitchWidth, thePitchHeight);
};

/** @ingroup SignalTypes
 * @brief A TransmissionType refinement for describing 2-D square matrix data.
 * @author Gav Wood <gav@kde.org>
 *
 * This is very similar to the Matrix class except that it describes only data
 * representing *square* martixes; that is the width and the
 * height are equal.
 */
class DLLEXPORT SquareMatrix: public Matrix
{
	TRANSMISSION_TYPE(SquareMatrix, Matrix);

public:
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
	SquareMatrix(uint size = 1, float frequency = 0., float pitch = 0.) : Matrix(size, size, frequency, pitch, pitch) {}

	/**
	 * Get the size of the matrix this TransmissionType represents.
	 *
	 * The sample size is therefore equal to size() * size().
	 *
	 * @return The number of elements in every row and column of the matrix.
	 */
	uint size() const { return theWidth; }

	virtual QString info() const { return QString("<div><b>SquareMatrix</b></div>") + Matrix::info(); }

	/**
	 * Get the frequency represented by the progression of elements in either
	 * the row or column.
	 *
	 * @return The pitch of the matrix elements.
	 */
	float pitch() const { return thePitchWidth; }

	TT_NO_MEMBERS;
};

}
