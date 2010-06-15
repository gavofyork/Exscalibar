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

#include <qstring.h>

#define SETUP_C_SUBSCRIPTS
#include <newmat/newmat.h>
#include <newmat/newmatap.h>
using namespace NEWMAT;

namespace QtExtra
{

inline float qClamp(float const _v, float const _min = 0.f, float const _max = 1.f)
{
	return _v < _min ? _min : _v > _max ? _max : _v;
}

ReturnMatrix DLLEXPORT readDump(const QString &_fn, uint const _dims);
void DLLEXPORT writeDump(const QString &_fn, const NEWMAT::Matrix &_values);

class DLLEXPORT QFeatures
{
public:
	QFeatures() {}
	QFeatures(NEWMAT::Matrix const& _eigenVectors):
		m_means(RowVector(_eigenVectors.Ncols())), m_eigenValues(DiagonalMatrix(_eigenVectors.Ncols())), m_eigenVectors(_eigenVectors) {}
	QFeatures(RowVector const& _means, DiagonalMatrix const& _eigenValues, NEWMAT::Matrix const& _eigenVectors):
		m_means(_means), m_eigenValues(_eigenValues), m_eigenVectors(_eigenVectors) {}

	bool				isEmpty() const { return !m_eigenVectors.Nrows(); }

	const DiagonalMatrix&eigenValues() const { return m_eigenValues; }
	const NEWMAT::Matrix &		eigenVectors() const { return m_eigenVectors; }
	const RowVector &	means() const { return m_means; }

	void				pca(const NEWMAT::Matrix &_values);
	ReturnMatrix		project(const NEWMAT::Matrix &_values, const uint _count, bool const _normalise = true, bool const _clamp = true) const;
	ReturnMatrix		extrapolate(const NEWMAT::Matrix &_projected) const;

	void				readVec(QString const& _fn);
	void				writeVec(QString const& _fn);

private:
	RowVector			m_means;
	DiagonalMatrix		m_eigenValues;
	NEWMAT::Matrix				m_eigenVectors;
};

}
