#ifndef _GEDDEI_SIGNALTYPE_H
#define _GEDDEI_SIGNALTYPE_H

#include <iostream>
using namespace std;

#include <qglobal.h>

#include <exscalibar.h>

class QSocketSession;

namespace Geddei
{

/** @ingroup Geddei
 * @brief Base class for describing a signal that may be transferred in a Connection.
 * @author Gav Wood <gav@kde.org>
 *
 * Class encapsulating description of a signal. The base class includes
 * attributes for all signals (i.e. scope and frequency). On derivation,
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
class DLLEXPORT SignalType
{
	friend class LRConnection;
	friend class RLConnection;
	friend class DRCoupling;
	friend class RSCoupling;
	friend class SignalTypeRefs;
	friend class SignalTypeRef;
	friend class Buffer;
	friend class Splitter;
	friend class MLConnection;
	friend class LMConnection;
	friend class LxConnection;
	friend class LLConnection;
	friend class Processor;
	friend class SubProcessor;
	friend class LxConnectionNull;

	/**
	 * Sends this SignalType object to the given QSocketSession object.
	 *
	 * @param sink The session for this object to be saved into.
	 */
	void send(QSocketSession &sink) const;

	/**
	 * Retrieves a SignalType object from the given QSocketSession.
	 *
	 * @note A new object will be created by this method. It is left to the
	 * caller to manage (or delegate) its deletion.
	 *
	 * @param source The session from which the object should be read.
	 * @return A new SignalType-derived object which is equivalent to that sent
	 * from the opposite end.
	 */
	static SignalType *receive(QSocketSession &source);

	/**
	 * Duplicate this SignalType object.
	 *
	 * @return A duplicate of this object.
	 */
	SignalType *copy() const { return copyBE(); }

	/**
	 * Compare two SignalType objects for equivalency.
	 *
	 * @param comp The object to be compared to this.
	 * @return true iff they are equivalent.
	 */
	bool sameAs(const SignalType *comp) const { return sameAsBE(comp); }

	/**
	 * Static method for creation of a new SignalType-derived class from its
	 * unique identity.
	 *
	 * This must be ammended each time a new SignalType-derived class is added
	 * to Geddei.
	 *
	 * @param id The unique class identifier of the SignalType to be created.
	 * @return A new SignalType object whose id() is equal to @a id .
	 */
	static SignalType *create(uint id);

protected:
	/**
	 * Number of individual observation elements that this datum contains.
	 */
	uint theScope;

	/**
	 * Reciprocal of the amount of time between this sample and the next.
	 */
	float theFrequency;

	/**
	 * @name Virtual methods for mandatory reimplementation.
	 *
	 * These methods must be reimplemented in any new classes that are derived,
	 * no matter how distantly, from this class.
	 */
	//@{

	/**
	 * Send this SignalType down the given QSocketSession, @a sink . This must
	 * be reimplemented along with its partner, deserialise() if any data
	 * members are added.
	 *
	 * See QSocketSession for reference, but you'll generally just want to use
	 * the method QSocketSession::safeSendWord(). This method only works for int
	 * members natively, so you'll have to cast float members appropriately:
	 *
	 * @code
	 * sink.safeSendWord(theIntMember);
	 * sink.safeSendWord(theFloatMember);
	 * @endcode
	 *
	 * @note You MUST call the parent class's serialise before anything else.
	 * Failure to do this will result in the parent's members going unsaved.
	 *
	 * @param sink The QSocketSession object into which they will be saved.
	 */
	virtual void serialise(QSocketSession &sink) const;

	/**
	 * Retrieve and repopulate this SignalType object from the given
	 * QSocketSession @a source . This must be symmetrical to serialise(). It
	 * must therefore be reimplemented when any data members are added to the
	 * inheritance heirarchy.
	 *
	 * See QSocketSession for reference, but you'll generally only need to use
	 * QSocketSession::safeReceiveWord(). Like serialise() this only actually
	 * works for ints as standard so float will have to be casted by hand:
	 *
	 * @code
	 * theIntMember = source.safeReceiveWord<int>();
	 * theFloatMember = source.safeReceiveWord<float>();
	 * @endcode
	 *
	 * @note You MUST call the parent class's deserialise() before anything
	 * else. Failure to do so will result in the parent's members going
	 * unsaved.
	 *
	 * @param source The QSocketSession object from which this object will
	 * load.
	 */
	virtual void deserialise(QSocketSession &source);

	/**
	 * Retrieve the numeric identity of this class. This is to easily identify
	 * each derivative of SignalType. This must be unique to each class, thus
	 * it must ALWAYS be REIMPLEMENTED.
	 *
	 * When subclassing SignalType (or one of its children), make sure you
	 * reimplement this method. return from this method a previously unused number.
	 *
	 * @return The unique identity of the new class.
	 */
	virtual uint id() const = 0;

	/**
	 * Duplicate this SignalType object.
	 *
	 * In this reimplementation you must simply create a new object of this
	 * class and populate it with this object's parameters. This method must
	 * therefore ALWAYS be REIMPLEMENTED in order for the virtual methods to
	 * work correctly.
	 *
	 * Note this is a backend method to be reimplmented. The front-end, public-
	 * visible method is copy(). You should use that in any interface code.
	 *
	 * @return A duplicate of this object.
	 */
	virtual SignalType *copyBE() const = 0;

	/**
	 * Compare two SignalType objects for equivalency. This must be
	 * reimplemented if any new members are added to the class heirarchy.
	 *
	 * The reimplementation will generally utilise the parent class's
	 * implementation as such:
	 *
	 * @code
	 * return Parent::sameAsBE(comp) &&
	 *        theNewMember == dynamic_cast<ClassName *>(comp)->theNewMember;
	 * @endcode
	 *
	 * Note this is a backend method to be reimplmented. The front-end, public-
	 * visible method is sameAs(). You should use that in any interface code.
	 *
	 * @param comp The object to be compared to this.
	 * @return true iff they are equivalent.
	 */
	virtual bool sameAsBE(const SignalType *comp) const { return id() == comp->id() && theScope == comp->theScope && theFrequency == comp->theFrequency; }

	//@}

	/**
	 * Basic constructor.
	 *
	 * @param scope The scope of the signal. That is, how many elements (single
	 * values) is takes to adequatly define a single reading, or sample. For a
	 * simple time-based offset single, this will be 1, but for more complex
	 * signals such as spectra, matrices or whatever, this will be more.
	 * @param frequency The sampling frequency of the signal. That is, the
	 * inverse of the delay between each sample. e.g. CD audio has a sample
	 * frequency of 44100, i.e. each sample represents a reading in time of
	 * 1/44100th of a second later than the last.
	 */
	SignalType(uint scope = 1, float frequency = 0);

public:
	/**
	 * @return The scope, or number of single value elements per sample of the
	 * data represented by this SignalType.
	 */
	uint scope() const { return theScope; }

	/**
	 * @return The frequency, or number of samples per Signal-time second of
	 * the data represented by this SignalType.
	 */
	float frequency() const { return theFrequency; }

	/**
	 * Sets the scope of this SignalType.
	 *
	 * @param scope The new scope.
	 */
	void setScope(uint scope) { theScope = scope; }

	/**
	 * Sets the frequency of this SignalType.
	 *
	 * @param frequency The new frequency.
	 */
	void setFrequency(float frequency) { theFrequency = frequency; }

	/**
	 * Get the number of samples represented by the given number of elements.
	 *
	 * @param elements The number of elements.
	 * @return The number of samples that @a elements constitute. This will be
	 * less than or equal to @a elements .
	 */
	uint samples(uint elements) const { return elements / theScope; }

	/**
	 * Get the number of seconds represented by the given number of elements.
	 *
	 * @param elements The number of elements.
	 * @return The number of Signal-time seconds that @a elements constitute.
	 */
	float seconds(uint elements) const { return float(elements / theScope) / theFrequency; }

	/**
	 * Get the number of elements a number of samples represents.
	 *
	 * @param samples The number of samples.
	 * @return The number of elements represented by @a samples . This will be
	 * greater than or equal to @a samples .
	 */
	uint elementsFromSamples(uint samples) const { return samples * theScope; }

	/**
	 * Get the number of elements a Signal-time length of seconds represents.
	 *
	 * @param seconds The length of Signal-time.
	 * @return The number of elements.
	 */
	uint elementsFromSeconds(float seconds) const { return (uint)(seconds * theFrequency) * theScope; }

	/**
	 * Virtual destructor.
	 */
	virtual ~SignalType() {}
};

}

#endif

