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

#include <QVarLengthArray>

namespace QtExtra
{

template<class T>
class DLLEXPORT QRing
{
public:
	inline QRing(int _size = 0) { resize(_size); }

	inline void resize(int _size) { m_data.resize(_size); m_capacity = _size; clear(); }

	inline T const& operator[](int _i) const { return m_data[(m_start + _i) % m_capacity]; }
	inline T& operator[](int _i) { return m_data[(m_start + _i) % m_capacity]; }

	inline int count() const { return m_count; }
	inline void shift(T const& _t) { if (m_count < m_capacity) { m_data[(m_start + m_count) % m_capacity] = _t; m_count++; } else { m_data[m_start] = _t; m_start = (m_start + 1) % m_capacity; } }
	inline T unshift() { T ret; if (m_count > 0) { ret = m_data[m_start]; m_start = (m_start + 1) % m_capacity; } return ret; }

	inline void clear() { m_count = 0; m_start = 0; }

private:
	QVarLengthArray<T> m_data;
	int m_capacity;
	int m_start;
	int m_count;
};

}
