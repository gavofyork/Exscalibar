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

#include <cassert>
#include <iostream>
using namespace std;

#include <QDebug>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "signaltyperef.h"
#else
#include <geddei/signaltyperef.h>
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
 * SignalTypeRefs) it DOES own all data it points at. It will be deleted with
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

#if 0

class DLLEXPORT __Old_SignalTypeRefs
{
	friend class Processor;
	friend class DRCoupling;
	friend class RSCoupling;
	friend class SubProcessor;
	friend class xSCoupling;
	friend class DSCoupling;
	friend class DomProcessor;
	friend class Combination;

	uint theCount;
	const TransmissionType **theData;
	bool theImplicitCopying;

	/**
	 * Copies all the TransmissionType objects into the @a dest list.
	 *
	 * These copies are then owned by @a dest and their lives should be
	 * managed. i.e. They must be deleted by the caller before @a dest dies.
	 *
	 * @param dest The list to copy the SignalTypeRefs into as objects. It
	 * is cleared before anything is copied into it. To be safe, pass only
	 * empty lists.
	 */
	void copyInto(QList<TransmissionType*> &dest);

	/**
	 * Makes an identical copy of the list given by @a src . This is a deep
	 * copy - none of the data is shared.
	 *
	 * @param src The list of TransmissionType objects to copy into this object.
	 */
	explicit SignalTypeRefs(QList<TransmissionType*> &src);

	/**
	 * Copy constructor. Constructs an identical copy of the list given by
	 * @a src. This is a deep copy - none of the data is shared.
	 *
	 * @param src The SignalTypeRefs object to make a copy of.
	 */
	SignalTypeRefs(const SignalTypeRefs &src);

	/**
	 * Creates an unpopulated SignalTypeRefs object capable of holding @a count
	 * items.
	 *
	 * @param count The number of SignalTypePtr objects this is to hold.
	 * @param implicitCopying If true, the assignment overload will make deep
	 * copies. This should not be changed without very good reason (and I can't
	 * think of one yet).
	 */
	explicit SignalTypeRefs(uint count = 0, bool implicitCopying = true);

	bool populated(uint i) const
	{
		return theData[i];
	}

	/**
	 * Sets index @a i to point to the TransmissionType pointed to by @a d .
	 *
	 * This does NOT make a copy of it; This object now owns the TransmissionType
	 * object at @a d . From the caller's point of view, the object at @a d is
	 * now invalid.
	 *
	 * Only use this method if you will forget all about @a d after the call.
	 * Otherwise you should probably use copyData().
	 *
	 * Example of (pretty much the only sane) usage:
	 *
	 * @code
	 *   types.setData(0, new MyType(SomeParameter));
	 * @endcode
	 *
	 * @note Be very careful how you use this method, in patrticular, do not
	 * try to delete @a d after this call. You'll get memory corruption.
	 *
	 * @param i The index of this array like object to set.
	 * @param d A pointer to the TransmissionType -derived object which you wish to
	 * give for adoption into index @a i in this object.
	 *
	 * @sa copyData()
	 */
	void setData(uint i, const TransmissionType *d);

	/**
	 * Sets index @a i to point to a TransmissionType equivilent to that pointed to
	 * by @a d. Note this makes a copy of the data at @a d; you must destroy
	 * @a d in a normal manner after making this call.
	 *
	 * Example, given a SignalTypeRefs object "types":
	 *
	 * @code
	 * TransmissionType *d = new SomeType(someParameter);
	 * types.copyData(0, d);
	 * // Use d for something else...
	 * delete d;
	 * @endcode
	 *
	 * If you do not wish to use the TransmissionType after copying it to this object
	 * then setData() may be a better alternative.
	 *
	 * @param i The index of this array like object to set.
	 * @param d A pointer to the TransmissionType -derived object that you wish to
	 * copy to index @a i to.
	 *
	 * @sa setData()
	 */
	void copyData(uint i, const TransmissionType *d);

	/** @overload
	 * Retrieve a const SignalTypePtr for the object at index @a i.
	 *
	 * @param i The index of the SignalTypePtr object to be referenced.
	 * @return A const SignalTypePtr object representing the data at index
	 * @a i .
	 */
	const TransmissionType *ptrAt(uint i) const
	{
#ifdef EDEBUG
		if (i >= theCount)
		{	qDebug("Asked for %d th element when count=%d", i, theCount);
			return (((TransmissionType **)theData)[0]);
		}
#endif
		return theData[i];
	}

	/**
	 * Retrieve SignalTypePtr for the object at index @a i.
	 *
	 * Usual usage is with the assignment operator, where you would set this
	 * to point at some other TransmissionType. If you did this, it would take
	 * ownership over the data it now points at, similar to setData(). See
	 * SignalTypePtr::operator=() for more information.
	 *
	 * If you wish the assignment to copy the TransmissionType object, you should
	 * explicity use either copyData() instead of assignment or assign it to a
	 * copy() of the assignee.
	 *
	 * Example 1; these statements are equivalent:
	 * @code
	 * void foo(const SignalTypeRefs &in, SignalTypeRefs &out) { out[0].copyData(in[0]); }
	 * void foo(const SignalTypeRefs &in, SignalTypeRefs &out) { out[0] = in[0].copy(); }
	 * @endcode
	 *
	 * Example 2; these statements are equivalent:
	 * @code
	 * void foo(const SignalTypeRefs &in, SignalTypeRefs &out) { out[0].setData(new Wave(2600)); }
	 * void foo(const SignalTypeRefs &in, SignalTypeRefs &out) { out[0] = new Wave(2600); }
	 * @endcode
	 *
	 * @param i The index of the SignalTypePtr object to be referenced.
	 * @return A SignalTypePtr object representing the data at index @a i .
	 */
	TransmissionType *&mutablePtrAt(uint i)
	{
#ifdef EDEBUG
		if (i >= theCount)
		{	qDebug("Asked for %d th element when count=%d", i, theCount);
			return (((TransmissionType **)theData)[0]);
		}
		if (theData[i])
			qWarning("*** WARNING: You're probably attempting to write over an existing TransmissionType *.\n"
					 "             Doing so will likely cause a memory leak. Delete this entry first.");
#endif
		return (((TransmissionType **)theData)[i]);
	}

	/** @internal
	 * Resize the array to @a count items: Any items held are discarded.
	 * This should not need to be used in normal operation.
	 *
	 * @param count The number of SignalTypePtr objects it should store now.
	 */
	void resize(uint count);

	/**
	 * Fills all entries with the TransmissionType @a d. Deletes any entries before
	 * overwriting. This method works correctly even if @a d points to a
	 * current entry in this array.
	 *
	 * @note The data at @a d is now invalid. Only use this method if you will
	 * forget all about @a d directly after the call. Do not try to delete
	 * @a d . It is now owned by this object.
	 *
	 * If you do wish to use @a d after this call, copyFill() would be better
	 * suited.
	 *
	 * @param d A pointer to the TransmissionType with which to fill the array.
	 * @param replaceExisting true iff all entries are to be replaced by copies
	 * of @a d even if they're already holding another SignalTypePtr.
	 *
	 * @sa copyFill()
	 */
	void setFill(const TransmissionType *d, bool replaceExisting = true);

public:
	const SignalTypeRef operator[](uint i) const { return SignalTypeRef((TransmissionType *&)(theData[i])); }
	SignalTypeRef operator[](uint i) { return SignalTypeRef((TransmissionType *&)(theData[i])); }

	/**
	 * Get the amount of SignalTypePtr spaces currently allocated.
	 *
	 * @return The number of spaces allocated.
	 */
	uint count() const { return theCount; }

	/**
	 * Check if all TransmissionType objects contained in this object are actually
	 * equivalent.
	 *
	 * @return true if all SignalTypeRefs contained here point to equivalent
	 * types. If any are null, returns false. If fewer than two objects are
	 * contained it returns true.
	 */
	bool allSame() const;

	/**
	 * Assignment operator. Makes an identical copy of the list given by
	 * @a src .
	 *
	 * @note For internal development: This is a deep copy - none of the data
	 * is shared. This functionality can be overridden by setting
	 * implicitCopying to false in the constructor, though memory management
	 * issues make it somewhat dangerous.
	 *
	 * @param src The SignalTypeRefs object to make a copy of.
	 * @return A reference to this object.
	 */
	SignalTypeRefs &operator=(const SignalTypeRefs &src);

	/**
	 * Assignment operator. Copies one of @a src into each slot.
	 *
	 * @param src The SignalTypeRef to use for the copy.
	 * @return A reference to this object.
	 */
	SignalTypeRefs &operator=(const SignalTypeRef &src);

	/**
	 * Assignment operator. Copies one of @a src into each slot.
	 *
	 * @param src The TransmissionType object to make copies of.
	 * @return A reference to this object.
	 */
	SignalTypeRefs &operator=(const TransmissionType &src);

	/**
	 * Default destructor.
	 */
	~SignalTypeRefs();
};

#else

template<class TT>
class DLLEXPORT Types
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
	 * @param src The SignalTypeRefs object to make a copy of.
	 * @return A reference to this object.
	 */
	template<class T> Types<TT> &operator=(Types<T> const& _src)
	{
		if (_src.m_data.size() == m_data.size())
		{
			m_data.clear();
			foreach (Type<T> const& t, _src)
				m_data.append(t);
		}
		return *this;
	}

	/**
	 * Assignment operator. Copies one of @a src into each slot.
	 *
	 * @param src The SignalTypeRef to use for the copy.
	 * @return A reference to this object.
	 */
	Types<TT> &operator=(TT const& _src)
	{
		for (int i = 0; i < m_data.size(); i++)
			m_data[i] = _src;
		return *this;
	}
	Types<TT> &operator=(Type<TT> const& _src)
	{
		for (int i = 0; i < m_data.size(); i++)
			m_data[i] = _src;
		return *this;
	}

	/**
	 * Default destructor.
	 */
	~Types() {}

	Type<TT> const& operator[](uint i) const { return m_data[i]; }
	Type<TT>& operator[](uint i) { return m_data[i]; }

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
	 * @return true if all SignalTypeRefs contained here point to equivalent
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

	void fillEmpty(Type<TT> const& _f)
	{
		for (int i = 0; i < m_data.size(); i++)
			if (m_data[i].isNull())
				m_data[i] = _f;
	}

private:
	/**
	 * Copies all the TransmissionType objects into the @a dest list.
	 *
	 * These copies are then owned by @a dest and their lives should be
	 * managed. i.e. They must be deleted by the caller before @a dest dies.
	 *
	 * @param dest The list to copy the SignalTypeRefs into as objects. It
	 * is cleared before anything is copied into it. To be safe, pass only
	 * empty lists.
	 */
	void copyInto(QList<TransmissionType*>& _dest)
	{
		_dest.clear();
		foreach (Type<TT> const& t, m_data)
			_dest.append(t->copy());
	}

	/**
	 * Makes an identical copy of the list given by @a src . This is a deep
	 * copy - none of the data is shared.
	 *
	 * @param src The list of TransmissionType objects to copy into this object.
	 */
	explicit Types(QList<TransmissionType*> const& _src)
	{
		foreach (TransmissionType* t, _src)
			m_data.append(*t);
	}

	/**
	 * Copy constructor. Constructs an identical copy of the list given by
	 * @a src. This is a deep copy - none of the data is shared.
	 *
	 * @param src The SignalTypeRefs object to make a copy of.
	 */
	template<class T> Types(Types<T> const& _src)
	{
		foreach (Type<T> const& t, _src)
			m_data.append(t);
	}

	/**
	 * Creates an unpopulated SignalTypeRefs object capable of holding @a count
	 * items.
	 *
	 * @param _n The number of SignalTypePtr objects this is to hold.
	 */
	explicit Types(uint _n = 0)
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

	QList<Type<TT> > m_data;
};

typedef Types<TransmissionType> SignalTypeRefs;

#endif

}
