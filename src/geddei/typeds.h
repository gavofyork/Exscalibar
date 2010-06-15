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

#include <cassert>
#include <iostream>
using namespace std;

#include <QDebug>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "type.h"
#else
#include <geddei/type.h>
#endif
using namespace Geddei;

namespace Geddei
{

class TransmissionType;

/** @ingroup Geddei
 * @brief Array-like class for containing SignalTypePtr objects.
 * @author Gav Wood <gav@kde.org>
 *
 * Though this class uses pointers in its storage mechanism (hence
 * Types) it DOES own all data it points at. It will be deleted with
 * this class, so don't try to delete any of the pointers you get from it data.
 *
 * Its really a no-brainer: Just set it and forget about it. Copy methods can
 * be used if you want to keep ownership of the data.
 *
 * None of the data is ever shared either implicitly or explicitly.
 *
 * @note The class contains an assignment operator; this copies all data
 * deeply.
 */
template<class TT>
class DLLEXPORT Typeds
{
	friend class Processor;
	friend class DRCoupling;
	friend class RSCoupling;
	friend class SubProcessor;
	friend class xSCoupling;
	friend class DSCoupling;
	friend class DomProcessor;
	friend class Combination;

public:
	/**
	 * Assignment operator. Makes an identical copy of the list given by
	 * @a src .
	 *
	 * @note For internal development: This is a deep copy - none of the data
	 * is shared. This functionality can be overridden by setting
	 * implicitCopying to false in the constructor, though memory management
	 * issues make it somewhat dangerous.
	 *
	 * @param src The Types object to make a copy of.
	 * @return A reference to this object.
	 */
	template<class T> Typeds<TT> &operator=(Typeds<T> const& _src)
	{
		if (_src.m_data.size() == m_data.size())
		{
			m_data.clear();
			foreach (Typed<T> const& t, _src)
				m_data.append(t);
		}
		return *this;
	}

	/**
	 * Assignment operator. Copies one of @a src into each slot.
	 *
	 * @param src The Type to use for the copy.
	 * @return A reference to this object.
	 */
	Typeds<TT> &operator=(TT const& _src)
	{
		for (int i = 0; i < m_data.size(); i++)
			m_data[i] = _src;
		return *this;
	}
	Typeds<TT> &operator=(Typed<TT> const& _src)
	{
		for (int i = 0; i < m_data.size(); i++)
			m_data[i] = _src;
		return *this;
	}

	/**
	 * Default destructor.
	 */
	~Typeds() {}

	Typed<TT> const& operator[](uint i) const { return m_data[i]; }
	Typed<TT>& operator[](uint i) { return m_data[i]; }

	/**
	 * Get the amount of SignalTypePtr spaces currently allocated.
	 *
	 * @return The number of spaces allocated.
	 */
	// TODO: One or t'other!
	uint size() const { return m_data.count(); }
	uint count() const { return m_data.count(); }

	/**
	 * Check if all TransmissionType objects contained in this object are actually
	 * equivalent.
	 *
	 * @return true if all Types contained here point to equivalent
	 * types. If any are null, returns false. If fewer than two objects are
	 * contained it returns true.
	 */
	bool allSame() const
	{
		for (int i = 1; i < m_data.size(); i++)
			if (m_data[i] != m_data[0])
				return false;
		return true;
	}

	bool allSameType() const
	{
		for (int i = 1; i < m_data.size(); i++)
			if (m_data[i].type() != m_data[0].type())
				return false;
		return true;
	}

	void fillEmpty(Typed<TT> const& _f)
	{
		for (int i = 0; i < m_data.size(); i++)
			if (m_data[i].isNull())
				m_data[i] = _f;
	}

//private:
	/**
	 * Copies all the TransmissionType objects into the @a dest list.
	 *
	 * These copies are then owned by @a dest and their lives should be
	 * managed. i.e. They must be deleted by the caller before @a dest dies.
	 *
	 * @param dest The list to copy the Types into as objects. It
	 * is cleared before anything is copied into it. To be safe, pass only
	 * empty lists.
	 */
	void copyInto(QList<TransmissionType*>& _dest)
	{
		_dest.clear();
		foreach (Typed<TT> const& t, m_data)
			_dest.append(t->copy());
	}

	/**
	 * Makes an identical copy of the list given by @a src . This is a deep
	 * copy - none of the data is shared.
	 *
	 * @param src The list of TransmissionType objects to copy into this object.
	 */
	explicit Typeds(QList<TransmissionType*> const& _src)
	{
		foreach (TransmissionType* t, _src)
			m_data.append(*t);
	}

	/**
	 * Copy constructor. Constructs an identical copy of the list given by
	 * @a src. This is a deep copy - none of the data is shared.
	 *
	 * @param src The Types object to make a copy of.
	 */
	template<class T> Typeds(Typeds<T> const& _src)
	{
		foreach (Typed<T> const& t, _src)
			m_data.append(t);
	}

	/**
	 * Creates an unpopulated Types object capable of holding @a count
	 * items.
	 *
	 * @param _n The number of SignalTypePtr objects this is to hold.
	 */
	explicit Typeds(uint _n = 0)
	{
		for (uint i = 0; i < _n; i++)
			m_data.append(TT());
	}

	bool populated(uint _i) const
	{
		return !m_data[_i].isNull();
	}

	/** @internal
	 * Resize the array to @a count items: Any items held are discarded.
	 * This should not need to be used in normal operation.
	 *
	 * @param count The number of SignalTypePtr objects it should store now.
	 */
	void resize(uint _n)
	{
		m_data.clear();
		for (uint i = 0; i < _n; i++)
			m_data.append(TT());
	}

	QList<Typed<TT> > m_data;
};

}
