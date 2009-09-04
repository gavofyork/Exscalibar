/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_SUBPROCESSOR_H
#define _GEDDEI_SUBPROCESSOR_H

#include <qstring.h>
#include <qmutex.h>
#include <qthread.h>
#include <qpainter.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "qfastwaitcondition.h"
#include "globals.h"
#include "properties.h"
#include "bufferdatas.h"
#include "signaltyperefs.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <geddei/globals.h>
#include <geddei/properties.h>
#include <geddei/bufferdatas.h>
#include <geddei/signaltyperefs.h>
#endif

namespace Geddei
{

class ProcessorForwarder;
class DomProcessor;
class xSCoupling;

/** @ingroup Geddei
 * @brief Base class that defines a single stateless Geddei signal data processing object.
 * @author Gav Wood <gav@cs.york.ac.uk>
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
 * slightly more scope for optimisation allowing you to keep an internal state
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
class DLLEXPORT SubProcessor: public QThread
{
	friend class Combination;

	//@{
	/** Basic properties. */
	QString theType;
	uint theNumInputs, theNumOutputs, theIn, theOut, theStep;
	MultiplicityType theMulti;
	//@}

	//@{
	/** Data passing mechanism. */
	BufferDatas theCurrentIn, theCurrentOut;
	SignalTypeRefs theOutTypes;
	bool theReturned, theLoaded;
	uint theChunks;
	mutable QMutex theDataInUse;
	mutable QFastWaitCondition theDataChanged;
	//@}

	//@{
	/** Timing mechanism. */
	uint theTimeTaken;
	//@}

	//@{
	/** Thread subsystem. */
	virtual void run();
	//@}

	/** @internal
	 * theStopping means that all processing must now stop.
	 */
	bool theStopping;

	/** @internal
	 * theNoMoreTransactions means that no more transactions may be taken or given,
	 * but processing may continue for now.
	 * Distinction between this and theStopping is neccessary to allow eater/demux
	 * to drop out when neccessary. theNoMoreTransactions is protected by
	 * theDataInUse/theDataChanged.
	 */
	bool theNoMoreTransactions;

	//@{
	/** DomProcessor relationship. */
	DomProcessor *thePrimaryOf;
	friend class DomProcessor;

	/** @internal
	 * Records the DomProcessor that we are the primary of. Used only by
	 * DomProcessor itself.
	 *
	 * @param primaryOf The DomProcessor we are to be the primary of.
	 */
	void setPrimaryOf(DomProcessor *primaryOf) { thePrimaryOf = primaryOf; }
	//@}

	//* Association and deletion management.
	xSCoupling *theCoupling;
	friend class xSCoupling;

	/** @internal
	 * Use to find associated coupling. Used by ProcessorForwarder for deletion
	 * management.
	 *
	 * @return A pointer to the associated Coupling.
	 */
	xSCoupling *coupling() { return theCoupling; }
	friend class ProcessorForwarder;

	//* xSCoupling Interface
	/** @internal
	 * Starts the transaction processor.
	 */
	void go();

	/** @internal
	 * Stops the transaction processor.
	 */
	void stop();

	/** @internal
	 * Registers a new transaction. If one is already being processed it blocks
	 * until completed and its results are returned.
	 *
	 * @param i The input data.
	 * @param chunks The number of chunks that the data represents. Note if
	 * chunks == 0, we implicitly mean a plunger.
	 */
	void transact(const BufferDatas &i, const uint chunks);

	/** @internal
	 * Blocks until the last transaction is completed. Then retrieves the results
	 * and returns then. It is up to the caller to know what to do with the results.
	 *
	 * @param timeTaken An int pointer which if not zero will be filled with the
	 * (wall) time taken to process the chunks. If last was a plunger, it is filled
	 * with the last actual computation or zero if there wasn't one.
	 * @return The results of the transaction. Note if BufferDatas.size() == 0,
	 * we implicitly mean a plunger.
	 */
	BufferDatas deliverResults(uint *timeTaken = 0);

	/** @internal
	 * A quick'n'dirty proxy method, that basically just calls the virtual function
	 * verifyAndSpecifyTypes(...). This should be used as an abstract interface. It,
	 * in fact does an extra thing, namely records the outTypes for later use.
	 * DO NOT call verifyAndSpecifyTypes(...) directly: Use proxyVSTypes(...) instead.
	 */
	const bool proxyVSTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) { return verifyAndSpecifyTypes(inTypes, outTypes); }

	/** @internal
	 * (Re)defines the number of inputs/outputs after any multiplicity concerns are
	 * accounted for.
	 */
	void defineIO(const uint numInputs, const uint numOutputs);

protected:
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
	virtual void processChunks(const BufferDatas &in, BufferDatas &out, const uint chunks) const;

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
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes) = 0;

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
	virtual void initFromProperties(const Properties &properties) = 0;

	/**
	 * Determine the multiplicity of the connection. Only valid after setupIO() has
	 * has been called.
	 *
	 * @return The number of outputs in the case of an Output Multiplicitive object
	 * or the number of inputs in the case of an Input Multiplicitive object. Undefined
	 * is given otherwise.
	 */
	const uint multiplicity() const { return theMulti == In ? theNumInputs : theMulti == Out ? theNumOutputs : Undefined; }

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
	 * one sample out and that each ionput chunk would overlap by 50%, meaning that
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
	void setupIO(const uint numInputs = 1, const uint numOutputs = 1, const uint samplesIn = 1, const uint samplesStep = 1, const uint samplesOut = 1);

	void setupSamplesIO(const uint samplesIn, const uint samplesStep, const uint samplesOut);
	
	/**
	 * Call this from initFromProperties to initialise the visual properties of
	 * the SubProcessor.
	 *
	 * If this is not called, the size will default to 50x30 and no redraw.
	 *
	 * @param width The width of the drawing canvas. Should be a multiple of 10.
	 * @param height The height of the drawing canvas. Should be a multiple of 10.
	 * @param redrawPeriod The rate for which the processor's visual should
	 * be redrawn in milliseconds. A value of zero means no explicit redraw.
	 *
	 * @sa setupIO() paintProcessor()
	 */
	void setupVisual(const uint width = 50, const uint height = 30, const uint redrawPeriod = 0);

	/**
	 * Reimplement for to define how the SubProcessor should be drawn visually.
	 * @param p The painting canvas onto which the visual may be drawn.
	 *
	 * @sa setupVisual()
	 */
	virtual void paintProcessor(QPainter &p);

	/**
	 * SubProcessor constructor - use this when subclassing.
	 *
	 * @param type This must be set to the name of the subclass.
	 * @param multi The type of multiplicity this SubProcessor supports.
	 */
	SubProcessor(const QString &type, const MultiplicityType &multi = NotMulti);

public:
	/**
	 * Basic destructor.
	 */
	virtual ~SubProcessor() { }
};

};

#endif
