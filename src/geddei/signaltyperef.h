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

#include <qstring.h>

namespace Geddei
{

class SignalType;

/** @ingroup Geddei
 * @brief A container for a SignalType-derived object.
 * @author Gav Wood <gav@kde.org>
 *
 * This is a fairly unusual class; it is designed to be as simple to use as
 * possible but the simplicity does somewhat make light of the ownership
 * semantics. This object is used as (a reference to) a container for a
 * SignalType -based object. The side effect of this is that you cannot create
 * your own instances of this class; you use it to access the SignalType it
 * contains only.
 *
 * This class provides functionality to get at that object directly with the
 * asA() methods. You can also query what type the held object is with the
 * isA() method. It also provides some conveniece methods that just relay
 * queries to the contained object, such as scope() and frequency().
 *
 * Generally that's all you really need to know. Assigning another SignalType
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
 * copied not just the SignalType class; so if the source SignalTypeRef
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
class DLLEXPORT SignalTypeRef
{
	SignalType *&thePtr;

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
	SignalTypeRef(SignalType *&ptr) : thePtr(ptr) {}

public:
	/**
	 * Check to see if the SignalType-based object we reference is actually an
	 * instance of some particular type (@a T).
	 *
	 * @return true iff we reference an object of type @a T.
	 */
	template<class T>
	bool isA() const { return dynamic_cast<const T *>(thePtr); }

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
			qFatal("*** FATAL: Attempting to attain a SignalType %s from an object of type %s.\n"
				   "           Bailing.", typeid(T).name(), typeid(*thePtr).name());
#endif
		return *(dynamic_cast<const T *>(thePtr));
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
			qFatal("*** FATAL: Attempting to attain a SignalType %s from an object of type %s.\n"
				   "           Bailing.", typeid(T).name(), typeid(*thePtr).name());
#endif
		return *(dynamic_cast<T *>(thePtr));
	}

	/**
	 * Convenience function to go through the reference and retrieve the
	 * scope of the SignalType.
	 *
	 * This is equivalent to:
	 *
	 * @code
	 * (*this).asA(SignalType).scope();
	 * @endcode
	 *
	 * @return The scope, or number of single value elements per sample of the
	 * data represented by this SignalType.
	 */
	uint scope() const;

	/**
	 * Convenience function to go through the reference and retrieve the
	 * frequency of the SignalType.
	 *
	 * This is equivalent to:
	 *
	 * @code
	 * (*this).asA(SignalType).frequency();
	 * @endcode
	 *
	 * @return The frequency, or number of samples per Signal-time second of
	 * the data represented by this SignalType.
	 */
	float frequency() const;

	/**
	 * Assignment operator. This will turn us into a copy of the SignalType
	 * given. We will automatically become the true class of @a p, not just
	 * a copy of whatever class it happens to be currently casted as.
	 *
	 * @param p A SignalType instance, it will be left untouched. A copy will
	 * be made and adopted.
	 * @return A reference to this object.
	 */
	SignalTypeRef &operator=(const SignalType &p);

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
	 * Check to see if we are the same as some other SignalType. This not only
	 * checks that we are the same type but also checks that our parameters are
	 * the same.
	 *
	 * @param p The type against which to be compared.
	 * @return true iff we are completely equivalent.
	 */
	bool operator==(const SignalType &p);

	/** @overload
	 * Check to see if we are the same as some other SignalType. This not only
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
	 * assignment operator, where we actually copy the data at thePtr.
	 *
	 * @param src The source pointer reference to be copied.
	 */
	SignalTypeRef(const SignalTypeRef &src) : thePtr(src.thePtr) {}
};

}
