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

#include <stdint.h>
#include <iostream>
using namespace std;

#include <QMap>
#include <QString>
#include <QDebug>

#include <qglobal.h>
#include <memberinfo.h>
#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "typeregistrar.h"
#else
#include <geddei/typeregistrar.h>
#endif

class QSocketSession;

namespace Geddei
{

class Source;
class BufferData;

template<class T> class Typed;
template<class T> class Typeds;

#define TRANSMISSION_TYPE(Name, Super) \
public: \
	Name(NullTransmissionTypeType _n): Super(_n) {} \
	virtual QString type() const { return staticType(); } \
	inline static QString staticType() { return #Name; } \
protected: \
	template<class T> friend class Typed; \
	virtual bool isEqualTo(TransmissionType const* _cmp) const { return _cmp->type() == staticType() && *static_cast<Name const*>(_cmp) == *this; } \
	static TypeRegistration<Name> s_reg; \
	TT_INHERITED_CLASS(Name, Super);

#define TRANSMISSION_TYPE_CPP(Name) \
	TypeRegistration<Name> Name::s_reg(#Name);

/** @ingroup Geddei
 * @brief Base class for describing a signal that may be transferred in a Connection.
 * @author Gav Wood <gav@kde.org>
 *
 * Class encapsulating description of a signal. The base class includes
 * attributes for all signals (i.e. sampleSize and frequency). On derivation,
 * several helper methods must be reimplemented: serialise(), deserialise(),
 * id(), copy() and sameAs(). Other than that you are free to add whatever you
 * feel neccesssary to the class to best describe (and help with decoding) the
 * signal.
 *
 * Usually a derived class adds little more than some extra attributes and the
 * methods for extracting these attributes.
 *
 * The final thing that all derived types must (currently) do is add a line in
 * the static method create() for creation of the new class. This is quite
 * self-explanatory.
 *
 * As good practise for future expansion and derivation you should leave all
 * useful data members in the protected area of the class. Helper methods
 * should all be reimplemented in the private to avoid adding unnecessary
 * bulk to the documentation.
 */
class DLLEXPORT TransmissionType
{
	TT_BASE_CLASS(TransmissionType);

	template<class T> friend class Typeds;
	template<class T> friend class Typed;

protected:
	enum NullTransmissionTypeType { NullTransmissionType };

public:
	/**
	 * Basic constructor.
	 *
	 * @param sampleSize The sampleSize of the signal. That is, how many elements (single
	 * values) is takes to adequatly define a single reading, or sample. For a
	 * simple time-based offset single, this will be 1, but for more complex
	 * signals such as spectra, matrices or whatever, this will be more.
	 */
	inline TransmissionType(uint _size = 1u): m_size(max(_size, 1u)) {}
	inline TransmissionType(NullTransmissionTypeType): m_size(0) {}

	/**
	 * Virtual destructor.
	 */
	virtual ~TransmissionType() {}

	/**
	 * "Official" type name of this transmission type.
	 */
	virtual QString type() const { return staticType(); }
	inline static QString staticType() { return "TransmissionType"; }

	/**
	 * Descriptive bit of HTML about this class.
	 */
	virtual QString info() const { return QString("<div><b>TransmissionType</b></div><div>Size: %2</div><div>Arity: %3</div>").arg(m_size).arg(arity()); }

	/**
	 * @return The number of single value elements per sample of the
	 * data represented by this TransmissionType that the user may assign themselves.
	 */
	inline uint arity() const { return m_size - reserved(); }
	inline void setArity(uint _n) { setSize(_n + reserved()); }

	/**
	 * Get the number of samples represented by the given number of elements.
	 *
	 * @param elements The number of elements.
	 * @return The number of samples that @a elements constitute. This will be
	 * less than or equal to @a elements .
	 */
	uint samples(uint elements) const { return m_size ? (elements / m_size) : 0; }

	/**
	 * Get the number of elements a number of samples represents.
	 *
	 * @param samples The number of samples.
	 * @return The number of elements represented by @a samples . This will be
	 * greater than or equal to @a samples .
	 */
	uint elementsFromSamples(uint samples) const { return samples * m_size; }

	/**
	 * @return The sampleSize, or number of single value elements per sample of the
	 * data represented by this TransmissionType, including Type-specific metadata
	 * that is not explicitly written by the user.
	 */
	uint size() const { return m_size; }

	bool isNull() const { return m_size == 0u; }

	/**
	 * Post-process the output data in some way.
	 *
	 * Used by Marker to put a timestamp on each sample.
	 */
	virtual void polishData(BufferData&, Source*, uint) const {}

	/**
	 * Sends this TransmissionType object to the given QSocketSession object.
	 *
	 * @param sink The session for this object to be saved into.
	 */
	void send(QSocketSession &sink) const;

	/**
	 * Retrieves a TransmissionType object from the given QSocketSession.
	 *
	 * @note A new object will be created by this method. It is left to the
	 * caller to manage (or delegate) its deletion.
	 *
	 * @param source The session from which the object should be read.
	 * @return A new TransmissionType-derived object which is equivalent to that sent
	 * from the opposite end.
	 */
	static Typed<TransmissionType> receive(QSocketSession &source);

protected:
	/**
	 * Number of (metadata) elements in each sample that are reserved for use by
	 * this TransmissionType.
	 */
	virtual uint reserved() const { return 0u; }

	/**
	 * Sets the sampleSize of this TransmissionType.
	 *
	 * @param sampleSize The new sampleSize.
	 */
	void setSize(uint _n) { m_size = max(_n, 1u); }

private:
	/**
	 * Duplicate this TransmissionType object.
	 *
	 * @return A duplicate of this object.
	 */
	inline TransmissionType* copy() const { return TypeRegistrar::get()->copy(this); }

	/**
	 * Determine whether given TT is a duplicate of this object.
	 */
	virtual bool isEqualTo(TransmissionType const* _cmp) const { return _cmp->type() == staticType(); }

	/**
	 * Number of individual observation elements that this datum contains.
	 */
	uint m_size;

	TT_1_MEMBER(m_size);
};

}
