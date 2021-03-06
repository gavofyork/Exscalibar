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

#include <QString>
#include <QPainter>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "qfastwaitcondition.h"
#include "globals.h"
#include "properties.h"
#include "autoproperties.h"
#include "bufferdatas.h"
#include "types.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <geddei/globals.h>
#include <geddei/properties.h>
#include <geddei/bufferdatas.h>
#include <geddei/autoproperties.h>
#include <geddei/types.h>
#endif

namespace Geddei
{

class ProcessorForwarder;
class DomProcessor;
class LxConnection;
class xSCoupling;
class DSCoupling;
class RSCoupling;

/** @ingroup Geddei
 * @brief Base class that defines a single stateless Geddei signal data processing object.
 * @author Gav Wood <gav@kde.org>
 *
 * The SubProcessor class is similar in profession to the @a Processor class. It
 * may be utilised to the same ends. However, as the name may suggest, it
 * trades some flexibility of the Processor class for simplicity and
 * better performance in some scenarios (e.g. parallelisation).
 *
 * There are far fewer methods open to you for reimplementation in this class
 * and perhaps the biggest difference is how you define what it does. Rather
 * than being given a method in which you may do anything, in this class you
 * specify at initialisation time what size of a chunk of input you want and
 * size of a chunk of output that will produce. Each chunk must be
 * disassociated with any others and thus independent. This makes the
 * SubProcessor object "stateless".
 *
 * You may also notice that there are (essentially) no methods for public use.
 * This is because SubProcessors (unlike Processors), on their own, are not
 * useful for forming Geddei processing networks. Each SubProcessor must be
 * encapsulated in a DomProcessor object. Each DomProcessor must have at least
 * one SubProcessor object and is associated with only one type of SubProcessor,
 * meaning that no other types of SubProcessor may be used with it.
 *
 * DomProcessors have all the neccessary external interface methods as Processor,
 * along with a few more custom ones.
 *
 * Many of the SubProcessor's virtual methods are actually quite similar to
 * those of the same name in the Processor class. initFromProperties(),
 * verifyAndSpecifyTypes() and specifyProperties() are all the same, and setupIO()
 * is very similar but with extra additions to declate the chunk input/output
 * sizes.
 *
 * Exactly one of processChunk() and processChunks() needs to be implemented:
 * processChunk() is the simpler of the two allowing you to specify the input
 * to output process as a single event. processChunks gives you, the developer,
 * slightly morescope for optimisation allowing you to keep an internal state
 * between multiple consecutive chunks. Obviously, use of this state cannot be
 * relied upon for correct creation of the outputs, but if the answers to a
 * previous chunk's calculation may be used to speed up a later chunk's
 * calculation, this allows it.
 *
 * Like the Processor class, SubProcessors support a feature called
 * multiplicity. This allows a SubProcessor to leave undefined the exact number
 * of inputs or outputs it has until the point of connection. In some
 * circumstances this can be implied from its forward or backward connections.
 * A constraint imposed by multiplicity is that all inputs and outputs must be
 * of equivanent SignalTypes.
 *
 * A simple example of a multiplicitive SubProcessor would be a "Sum" class,
 * that has a multiplicitive input and a single output. It would simply add
 * together a sample from each of the inputs to give the sum as an output. The
 * number of inputs could change automatically depending upon how many sources
 * were connected via. a MultiProcessor.
 *
 * @sa DomProcessor @sa MultiProcessor
 */
class DLLEXPORT SubProcessor: public AutoProperties
{
	friend class Combination;
	friend class xSCoupling;
	friend class DSCoupling;
	friend class RSCoupling;
	friend class LxConnection;
	friend class DomProcessor;
	friend class ProcessorForwarder;

public:
	/**
	 * Basic destructor.
	 */
	virtual ~SubProcessor() { }

	virtual QString type() const { return theType; }

	// Note the following are not for real use - only for getting visuals in GUI.

	void init(Properties const& _p) { initFromProperties(_p); }

	PropertiesInfo properties() const { return specifyProperties(); }

	void draw(QPainter& _p) const { paintProcessor(_p); }

	QColor outlineColour() const { return specifyOutlineColour(); }

	/**
	 * Gets the width of the processor's image. Used by the Nite for drawing.
	 *
	 * @return The image's width.
	 */
	uint width() const { return theWidth; }

	/**
	 * Gets the height of the processor's image. Used by the Nite for drawing.
	 *
	 * @return The image's height.
	 */
	uint height() const { return theHeight; }

	/**
	 * Gets the number of inputs this processor has.
	 *
	 * @return The number of inputs.
	 */
	uint numInputs() const { return theNumInputs; }

	/**
	 * Gets the number of outputs this processor has.
	 *
	 * @return The number of outputs.
	 */
	uint numOutputs() const { return theNumOutputs; }

	void setFlag(int _flag, bool _set = true) { m_flags = (m_flags & ~_flag); if (_set) m_flags |= _flag; }
	bool isInplace() const { return (m_flags & SubInplace) && theIn == 1 && theOut == 1 && theStep == 1 && m_inTypes.size() == m_outTypes.size() && m_inTypes.size() == 1 && m_inTypes[0].size() == m_outTypes[0].size(); }

protected:
	/**
	 * SubProcessor constructor - use this when subclassing.
	 *
	 * @param type This must be set to the name of the subclass.
	 * @param multi The type of multiplicity this SubProcessor supports.
	 */
	SubProcessor(const QString &type, MultiplicityType multi = NotMulti, int _f = 0);

	/**
	 * Reimplement to specify how to process a chunk of data.
	 *
	 * Either this *or* processChunks() *must* be reimplemented.
	 *
	 * @param in The input BufferData objects as a BufferDatas object, which acts
	 * similarly to an array of BufferData objects.
	 * @param out An output BufferDatas object into which the results may be placed.
	 * You can treat is as a preallocated array of BufferData objects. Each BufferData
	 * object has also been preallocated to the exact size requested with setupIO().
	 *
	 * @sa processChunks()
	 */
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;

	/**
	 * Reimplement to specify how to process a chunk of data. With this method
	 * several chunks may be processed at once. Though the results must not depend
	 * on data other that that of their corresponding input, some SubProcessors may
	 * find it advantageous to be able to process multiple consecutive chunks
	 * together. This methods allows that.
	 *
	 * @note Either this *or* processChunk() *must* be reimplemented.
	 *
	 * @param in The input BufferData objects as a BufferDatas object, which acts
	 * similarly to an array of BufferData objects.
	 * @param out An output BufferDatas object into which the results may be placed.
	 * You can treat is as a preallocated array of BufferData objects. Each BufferData
	 * object has also been preallocated to the exact size requested with setupIO().
	 * @param chunks The number of chunks worth of data that each BufferData object
	 * contains. If samplesStep is less than samplesIn in setupIO, then care must be
	 * taken to read the data correctly, since chunks will be naturally overlapped to
	 * save bandwidth.
	 *
	 * @sa processChunk()
	 */
	virtual void processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const;
	virtual void processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks);

	/**
	 * Reimplement to provide property specifications and default values for this
	 * SubProcessor.
	 *
	 * @return The Properties object that contains the property definitions and defaults.
	 */
	virtual PropertiesInfo specifyProperties() const;

	/**
	 * Reimplement to restrict signal types this class can handle, and define signaltypes
	 * it will output.
	 *
	 * Examples of correct usage:
	 *   @code outTypes[0] = new Wave(2600.0); @endcode
	 *   @code outTypes[1] = inTypes[0]->copy(); @endcode
	 *
	 * TIP: Don't forget inTypes is const, so if you're dynamic_cast<...>'ing a member,
	 * you'll have to cast it something else const!
	 *
	 * @param inTypes An array-like type populated with the input types of the connections.
	 * If the Processor is declared as a MultiIn, then you are guaranteed that all input
	 * types are the same basic class (parameters may be different); you don't need an
	 * extra test.
	 * @param outTypes An array-like type to contain the output types of the connections.
	 * They are all initially null, and if this method returns true, they must be defined.
	 * The array is large enough to store all the connections' types in. If the Processor
	 * is declared as a MultiOut, then you only have to define the first type. Any entries
	 * left undefined will be populated by copies of the first entry. They must all be of
	 * the same basic class.
	 * @return true if @a inTypes is valid and @a outTypes is populated correctly.
	 *
	 * @sa proxyVSTypes(), for internal development.
	 */
	//TODO: enforce the same basic class rule.
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes) = 0;

	/**
	 * Initialises from the given @a properties. Should call setupIO() (and
	 * setupVisual() too, really) at least.
	 *
	 * Any neccessary preprocessing can be done here, such as lookup table construction.
	 * This is a more useful place to do it than the constructor since you are given
	 * the property list here. You are guaranteed that this will be called before
	 * processChunk() or processChunks().
	 *
	 * @param properties The given Properties for the SubProcessor to be initialised
	 * with.
	 */
	virtual void initFromProperties(Properties const& _p);
	virtual void updateFromProperties(Properties const& _p);
	virtual void initFromProperties() {}
	virtual void updateFromProperties() {}

	/**
	 * Determine the multiplicity of the connection. Only valid after setupIO() has
	 * has been called.
	 *
	 * @return The number of outputs in the case of an Output Multiplicitive object
	 * or the number of inputs in the case of an Input Multiplicitive object. Undefined
	 * is given otherwise.
	 */
	uint multiplicity() const { return theMulti & In ? theNumInputs : theMulti & Out ? theNumOutputs : Undefined; }

	/**
	 * Call from initFromProperties(). Use it to set up the number of inputs/outputs,
	 * and the chunking parameters.
	 *
	 * The shown defaults apply (even if you don't call setupIO), but you should
	 * call it anyway with full parameter spec for code readability if nothing else.
	 * Don't count on this behaviour for later versions.
	 *
	 * The defaults imply a single sample for input will correspond to a single
	 * sample of output and each sample will be used exactly once, and there would be
	 * only one input and one output exhibiting this behaviour.
	 * Changing @a samplesIn to 2 would mean the 2 samples in would correspond to
	 * one sample out and that each input chunk would overlap by 50%, meaning that
	 * generally each sample would be used twice.
	 *
	 * @param numInputs The number of input connection ports. This must be > 0. In
	 * the case of an input multiplicative SubProcessor, you may use Undefined to
	 * leave it for the connection to determine.
	 * @param numOutputs The number of output connection ports. This must be > 0.
	 * In the case of an output multiplicative SubProcessor, you may use Undefined
	 * to leave if for the connection to determine.
	 * @param samplesIn The number of samples for an input chunk. This must be >=
	 * @a samplesStep. If not it will be incresed to @a samplesStep. (If you're
	 * interested, the reason for this is that the plunging code works less
	 * efficiently otherwise.)
	 * @param samplesStep The number of samples between consecutive starts of input
	 * chunks.
	 * @param samplesOut The number of samples in the output chunks.
	 *
	 * @sa setupVisual()
	 */
	void setupIO(uint numInputs, uint numOutputs);

	void setupSamplesIO(uint samplesIn, uint samplesStep, uint samplesOut);

	/**
	 * Call this from initFromProperties to initialise the visual properties of
	 * the SubProcessor.
	 *
	 * If this is not called, the size will default to 32x32.
	 *
	 * @param width The width of the drawing canvas. Should be a multiple of 10.
	 * @param height The height of the drawing canvas. Should be a multiple of 10.
	 *
	 * @sa setupIO() paintProcessor()
	 */
	void setupVisual(uint width = 32, uint height = 32);

	/**
	 * Reimplement for to define how the SubProcessor should be drawn visually.
	 * @param p The painting canvas onto which the visual may be drawn.
	 *
	 * @sa setupVisual()
	 */
	virtual void paintProcessor(QPainter& _p) const;
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(0, 0, 80); }
	virtual QString simpleText() const { return "S"; }

private:
	/** @internal
	 * Use to find associated coupling. Used by ProcessorForwarder for deletion
	 * management.
	 *
	 * @return A pointer to the associated Coupling.
	 */
	xSCoupling *coupling() { return theCoupling; }

	/** @internal
	 * A quick'n'dirty proxy method, that basically just calls the virtual function
	 * verifyAndSpecifyTypes(...). This should be used as an abstract interface. It,
	 * in fact does an extra thing, namely records the outTypes for later use.
	 * DO NOT call verifyAndSpecifyTypes(...) directly: Use proxyVSTypes(...) instead.
	 */
	bool proxyVSTypes(const Types &inTypes, Types &outTypes);

	/** @internal
	 * (Re)defines the number of inputs/outputs after any multiplicity concerns are
	 * accounted for.
	 */
	void defineIO(uint numInputs, uint numOutputs);

	//@{
	/** @internal
	 * Records the DomProcessor that we are the primary of. Used only by
	 * DomProcessor itself.
	 *
	 * @param primaryOf The DomProcessor we are to be the primary of.
	 */
	void setPrimaryOf(DomProcessor *primaryOf) { thePrimaryOf = primaryOf; }

	/** DomProcessor relationship. */
	DomProcessor *thePrimaryOf;
	//@}

	//@{
	/** Basic properties. */
	QString theType;
	uint theNumInputs, theNumOutputs, theIn, theOut, theStep;
	MultiplicityType theMulti;
	Types m_inTypes;
	Types m_outTypes;
	//@}

	//@{
	/** Graphics stuff. */
	uint theWidth;
	uint theHeight;
	//@}

	//* Association and deletion management.
	xSCoupling *theCoupling;

	int m_flags;

	QStringList m_dynamics;
};

class StatefulSubProcessor: public SubProcessor
{
public:
	StatefulSubProcessor(const QString &type, MultiplicityType multi = NotMulti): SubProcessor(type, multi) {}
};

}
