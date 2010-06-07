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

#ifdef EDEBUG
#include <typeinfo>
#endif

#include "signaltyperef.h"
#include <qstring.h>

namespace Geddei
{

class TransmissionType;

/** @ingroup Geddei
 * @brief A container for a TransmissionType-derived object.
 * @author Gav Wood <gav@kde.org>
 *
 * This is a fairly unusual class; it is designed to be as simple to use as
 * possible but the simplicity does somewhat make light of the ownership
 * semantics. This object is used as (a reference to) a container for a
 * TransmissionType -based object. The side effect of this is that you cannot create
 * your own instances of this class; you use it to access the TransmissionType it
 * contains only.
 *
 * This class provides functionality to get at that object directly with the
 * asA() methods. You can also query what type the held object is with the
 * isA() method.
 *
 * Generally that's all you really need to know. Assigning another TransmissionType
 * to this object does what you would expect---it just puts that type into us,
 * the container. Likewise when assigning another SignalTypeRef to us. The
 * following code is completely valid:
 *
 * @code
 * aSignalTypeRef = Wave(44100.0);
 * // ...
 * aSignalTypeRef = anotherSignalTypeRef;
 * // ...
 * aSignalTypeRef = Value();
 * @endcode
 *
 * If you assign from a SignalTypeRef, then cunningly, the actual class is
 * copied not just the TransmissionType class; so if the source SignalTypeRef
 * is actually referencing Spectrum, then that is what this class will
 * reference now.
 *
 * If you need to make a copy of an object for private use, then use asA() to
 * get the real instance of the object and make a copy of it that way:
 *
 * @code
 * // WRONG:
 * SignalTypeRef mySignalType = aSignalTypeRef;
 *
 * // Right:
 * Spectrum mySignalType;
 * mySignalType = aSignalTypeRef.asA<Spectrum>();
 * @endcode
 */
#if 0
class DLLEXPORT SignalTypeRef
{
	TransmissionType *&m_ptr;

	friend class SignalTypeRefs;
	friend class xLConnectionReal;
	friend class LxConnectionNull;
	friend class xLConnection;
	friend class LxConnection;
	friend class LMConnection;
	friend class MLConnection;
	friend class LRConnection;
	friend class Splitter;
	friend class Processor;

	/**
	 * Simple, private constructor.
	 *
	 * @param ptr The pointer to the data to be represented.
	 */
	SignalTypeRef(TransmissionType *&ptr) : m_ptr(ptr) {}

public:
	/**
	 * Check to see if the TransmissionType-based object we reference is actually an
	 * instance of some particular type (@a T).
	 *
	 * @return true iff we reference an object of type @a T.
	 */
	template<class T>
	bool isA() const { return dynamic_cast<const T *>(m_ptr); }

	/**
	 * Return a reference of class @a T that we represent. If we cannot
	 * represent such a class then this will cause Geddei to exit immediately.
	 * To prevent this from happening, always make sure isA() returns true
	 * first.
	 *
	 * @return A "real" reference to our instance of @a T.
	 */
	template<class T>
	const T &asA() const
	{
#ifdef EDEBUG
		if (!isA<T>())
			qFatal("*** FATAL: Attempting to attain a TransmissionType %s from an object of type %s.\n"
				   "           Bailing.", typeid(T).name(), typeid(*m_ptr).name());
#endif
		return *(dynamic_cast<const T *>(m_ptr));
	}

	/** @overload
	 * Return a reference of class @a T that we represent. If we cannot
	 * represent such a class then this will cause Geddei to exit immediately.
	 * To prevent this from happening, always make sure isA() returns true
	 * first.
	 *
	 * @return A "real" reference to our instance of @a T.
	 */
	template<class T>
	T &asA()
	{
#ifdef EDEBUG
		if (!isA<T>())
			qFatal("*** FATAL: Attempting to attain a TransmissionType %s from an object of type %s.\n"
				   "           Bailing.", typeid(T).name(), typeid(*m_ptr).name());
#endif
		return *(dynamic_cast<T *>(m_ptr));
	}

	/**
	 * Convenience function to go through the reference and retrieve the
	 * sampleSize of the TransmissionType.
	 *
	 * This is equivalent to:
	 *
	 * @code
	 * asA<TransmissionType>().size();
	 * @endcode
	 *
	 * @return The number of single value elements per sample of the
	 * data represented by this TransmissionType.
	 */
	uint size() const;

	QString info() const;

	/**
	 * Assignment operator. This will turn us into a copy of the TransmissionType
	 * given. We will automatically become the true class of @a p, not just
	 * a copy of whatever class it happens to be currently casted as.
	 *
	 * @param p A TransmissionType instance, it will be left untouched. A copy will
	 * be made and adopted.
	 * @return A reference to this object.
	 */
	SignalTypeRef &operator=(const TransmissionType &p);

	/**
	 * Assignment operator. This will turn us into a copy of the SignalTypeRef
	 * given. We will automatically become the true class of @a p, not just
	 * a copy of whatever class it happens to be currently casted as.
	 *
	 * @param p A SignalTypeRef instance, it will be left untouched. A copy
	 * will be made and adopted.
	 * @return A reference to this object.
	 */
	SignalTypeRef &operator=(const SignalTypeRef &p);

	/**
	 * Check to see if we are the same as some other TransmissionType. This not only
	 * checks that we are the same type but also checks that our parameters are
	 * the same.
	 *
	 * @param p The type against which to be compared.
	 * @return true iff we are completely equivalent.
	 */
	bool operator==(const TransmissionType &p);

	/** @overload
	 * Check to see if we are the same as some other TransmissionType. This not only
	 * checks that we are the same type but also checks that our parameters are
	 * the same.
	 *
	 * @param p The type against which to be compared.
	 * @return true iff we are completely equivalent.
	 */
	bool operator==(const SignalTypeRef &p);

	/** @internal
	 * For the explicit copy constructor we actually make a copy of ourselves.
	 *
	 * @note This is semantically the opposite of what happens in the
	 * assignment operator, where we actually copy the data at m_ptr.
	 *
	 * @param src The source pointer reference to be copied.
	 */
	SignalTypeRef(const SignalTypeRef &src) : m_ptr(src.m_ptr) {}
};
#else
template<class TT>
class DLLEXPORT Type
{
	template<class T> friend class Types;
	friend class TransmissionType;
	friend class xLConnectionReal;
	friend class LxConnectionNull;
	friend class xLConnection;
	friend class LxConnection;
	friend class LMConnection;
	friend class MLConnection;
	friend class LRConnection;
	friend class Splitter;
	friend class Processor;

public:
	/** @internal
	 * For the explicit copy constructor we actually make a copy of ourselves.
	 *
	 * @note This is semantically the opposite of what happens in the
	 * assignment operator, where we actually copy the data at m_ptr.
	 *
	 * @param src The source pointer reference to be copied.
	 */
	Type(TT const& src = TT()) : m_ptr(src.copy()) {}
	Type(Type<TT> const& _p) : m_ptr(_p.m_ptr->copy()) {}
	template<class T> Type(Type<T> const& _p) : m_ptr((_p.type() == TT::staticType()) ? static_cast<TT*>(_p.m_ptr->copy()) : new TT) {}
	~Type() { delete m_ptr; }

	/**
	 * Check to see if the TransmissionType-based object we reference is actually an
	 * instance of some particular type (@a T).
	 *
	 * @return true iff we reference an object of type @a T.
	 */
	template<class T>
	bool isA() const { return dynamic_cast<const T *>(m_ptr); }

	/**
	 * Return a reference of class @a T that we represent. If we cannot
	 * represent such a class then this will cause Geddei to exit immediately.
	 * To prevent this from happening, always make sure isA() returns true
	 * first.
	 *
	 * @return A "real" reference to our instance of @a T.
	 */
	template<class T>
	const T &asA() const
	{
#ifdef EDEBUG
		if (!isA<T>())
			qFatal("*** FATAL: Attempting to attain a TransmissionType %s from an object of type %s.\n"
				   "           Bailing.", typeid(T).name(), typeid(*m_ptr).name());
#endif
		return *(dynamic_cast<const T *>(m_ptr));
	}

	/** @overload
	 * Return a reference of class @a T that we represent. If we cannot
	 * represent such a class then this will cause Geddei to exit immediately.
	 * To prevent this from happening, always make sure isA() returns true
	 * first.
	 *
	 * @return A "real" reference to our instance of @a T.
	 */
	template<class T>
	T &asA()
	{
#ifdef EDEBUG
		if (!isA<T>())
			qFatal("*** FATAL: Attempting to attain a TransmissionType %s from an object of type %s.\n"
				   "           Bailing.", typeid(T).name(), typeid(*m_ptr).name());
#endif
		return *(dynamic_cast<T *>(m_ptr));
	}

	TT* operator*() const { return m_ptr; }
	TT* operator->() const { return m_ptr; }

	/**
	 * Assignment operator. This will turn us into a copy of the TransmissionType
	 * given. We will automatically become the true class of @a p, not just
	 * a copy of whatever class it happens to be currently casted as.
	 *
	 * @param p A TransmissionType instance, it will be left untouched. A copy will
	 * be made and adopted.
	 * @return A reference to this object.
	 */
	Type<TT>& operator=(TransmissionType const& _p)
	{
		if (dynamic_cast<TT const*>(&_p))
		{
			delete m_ptr;
			TransmissionType* c = _p.copy();
			assert(dynamic_cast<TT*>(c));
			m_ptr = static_cast<TT*>(c);
		}
		return *this;
	}

	/**
	 * Assignment operator. This will turn us into a copy of the SignalTypeRef
	 * given. We will automatically become the true class of @a p, not just
	 * a copy of whatever class it happens to be currently casted as.
	 *
	 * @param p A SignalTypeRef instance, it will be left untouched. A copy
	 * will be made and adopted.
	 * @return A reference to this object.
	 */
	Type<TT> &operator=(Type<TT> const& _p) { if (_p.m_ptr != m_ptr) { delete m_ptr; m_ptr = _p.m_ptr->copy(); assert(_p == *this); } return *this; }

	/**
	 * Check to see if we are the same as some other TransmissionType. This not only
	 * checks that we are the same type but also checks that our parameters are
	 * the same.
	 *
	 * @param p The type against which to be compared.
	 * @return true iff we are completely equivalent.
	 */
	bool operator==(TransmissionType const& _p) const { return m_ptr && m_ptr->isEqualTo(&_p); }
	bool operator!=(TransmissionType const& _p) const { return !operator==(_p); }

	/** @overload
	 * Check to see if we are the same as some other TransmissionType. This not only
	 * checks that we are the same type but also checks that our parameters are
	 * the same.
	 *
	 * @param p The type against which to be compared.
	 * @return true iff we are completely equivalent.
	 */
	template<class T> bool operator==(Type<T> const& _p) const { return m_ptr && _p.m_ptr && m_ptr->isEqualTo(_p.m_ptr); }
	template<class T> bool operator!=(Type<T> const& _p) const { return !operator==(_p); }

	void nullify() { operator=(TransmissionType()); }
	uint isNull() const { return m_ptr->isNull(); }

	/// Convenience methods.
	uint size() const { return m_ptr->size(); }
	uint arity() const { return m_ptr->arity(); }
	QString type() const { return m_ptr->type(); }
	QString info() const { return m_ptr->info(); }

private:
	Type<TT>& operator=(TT* _toBeAdopted) { assert(m_ptr); m_ptr = _toBeAdopted; return *this; }

	TT* m_ptr;
};

typedef Type<TransmissionType> SignalTypeRef;

#endif
}
