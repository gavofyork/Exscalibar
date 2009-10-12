/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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

#include <qstring.h>

#include <newmat/newmatap.h>

namespace QtExtra
{

inline float qClamp(float const _v, float const _min = 0.f, float const _max = 1.f)
{
	return _v < _min ? _min : _v > _max ? _max : _v;
}

ReturnMatrix DLLEXPORT readDump(const QString &_fn, uint const _dims);
void DLLEXPORT writeDump(const QString &_fn, const Matrix &_values);

class DLLEXPORT QFeatures
{
public:
	const DiagonalMatrix&eigenValues() const { return m_eigenValues; }
	const Matrix &		eigenVectors() const { return m_eigenVectors; }
	const RowVector &	means() const { return m_means; }

	void				pca(const Matrix &_values);
	ReturnMatrix		project(const Matrix &_values, const uint _count, bool const _normalise = true, bool const _clamp = true);
	ReturnMatrix		extrapolate(const Matrix &_projected);

	void				readVec(QString const& _fn);
	void				writeVec(QString const& _fn);

private:
	RowVector			m_means;
	DiagonalMatrix		m_eigenValues;
	Matrix				m_eigenVectors;
};

}
