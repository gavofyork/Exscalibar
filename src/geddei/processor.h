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

#include <QMutex>
#include <QThread>
#include <QThreadStorage>
#include <QPainter>
#include <QString>

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "qtask.h"
#include "qcleaner.h"
#include "globals.h"
#include "bufferdata.h"
#include "lxconnection.h"
#include "xlconnection.h"
#include "rlconnection.h"
#include "properties.h"
#include "autoproperties.h"
#include "source.h"
#include "sink.h"
#include "multisource.h"
#include "multisink.h"
#include "types.h"
#include "processorport.h"
#include "groupable.h"
#else
#include <qtextra/qtask.h>
#include <qtextra/qcleaner.h>
#include <geddei/globals.h>
#include <geddei/bufferdata.h>
#include <geddei/lxconnection.h>
#include <geddei/xlconnection.h>
#include <geddei/rlconnection.h>
#include <geddei/properties.h>
#include <geddei/autoproperties.h>
#include <geddei/source.h>
#include <geddei/sink.h>
#include <geddei/multisource.h>
#include <geddei/multisink.h>
#include <geddei/types.h>
#include <geddei/processorport.h>
#include <geddei/groupable.h>
#endif
using namespace QtExtra;
using namespace Geddei;

namespace Geddei
{

class DomProcessor;
class Buffer;
class ProcessorGroup;

/** @internal @ingroup Geddei
 * @brief Void class to allow usable exception objects.
 * @author Gav Wood <gav@kde.org>
 *
 * This was only made so ints don't have to be used, thereby not polluting the
 * exception namespace.
 */
class DLLEXPORT BailException
{
};

/** @ingroup Geddei
 * @brief Base class that defines a single Geddei signal data processing object.
 * @author Gav Wood <gav@kde.org>
 *
 * This is the most important class in Geddei. It embodies a multi-purpose
 * component in a data-flow network. Processor objects may be used as signal
 * sources, signal sinks or a signal throughput. In this way they may have
 * multiple inputs and/or outputs.
 *
 * The methods can be generally split into two types. Those (wholly
 * non-virtual) methods for external (to the Processor class) control of the
 * object --- these are all public. The second type are the methods used in
 * creating new classes of Processor. These comprise those that may (or must)
 * be reimplemented and those that are used for internal control of the object,
 *  e.g. for setting up the internals. These are all protected.
 *
 * Using Processor objects externally:
 *
 * A Processor object must be initialised before use, but then may be started
 * with go() and stop()ped, pause()d, and unpause()d. Between stopping a
 * processor and starting it again, all processors must be reset(), in order to
 * accomplish a synchronised restart.
 *
 * All Processor objects should have a name, set on init(). All Processor-
 * derived classes must have a type, set on class declaration. Either of these
 * may be retrieved with the eponymous methods.
 *
 * Processor objects may be put into groups with the setGroup and setNoGroup
 * methods. This can also be done on init().
 *
 * Connections between Processors may be created and destroyed with the
 * connect() and disconnect() methods. The TransmissionType validity and consistency
 * of the network may be checked (before the processors have been started) with
 * the confirmTypes() method. split() and share() may be used to fork outputs
 * for connection to multiple destinations.
 *
 * Processor objects may inform other such Processor objects about the current
 * plunger situation in the data stream. Plungers may be thought of as
 * seperators in the signal data stream. They segment one portion of the stream
 * and allow equivalent points in the data stream to be defined. You might
 * liken them to the "Next customer please" signs at supermarket checkouts.
 *
 * All Processor (-derived) objects are potential informers, however some
 * objects may be able to "become informed". Such objects are said to have the
 * "Guarded" attribute. In such a case they can be told to cease processing
 * once they have been informed that (after a certain plunger) no more data
 * will arrive. This is useful since there is a blocking method that can be
 * called on such an object, waitUntilDone(), which waits until the object
 * enters such a state.
 *
 * See the file testfileout.cpp for a concrete example of this usage.
 *
 * Creating new Processor classes:
 *
 * To create your own Processor class, you must subclass this and reimplement
 * all virtual void methods, and optionally the other virtual methods too.
 *
 * The main methods to implement are: specifyProperties(),
 * initFromProperties(), verifyAndSpecifyTypes() and processor(). It is
 * generally sensible to implement the other methods like specifyOutputSpace()
 * and specifyInputSpace(). See the documentation for more information.
 *
 * process() is the most significant method, as it is here you define what job
 * the Processor actually "does".
 *
 * Each input or output may have an arbitrary TransmissionType to describe the data
 * that flows down it. Outputs' SignalTypes must be specified with the
 * verifyAndSpecifyTypes() and inputs' SignalTypes may be constrained also.
 *
 * It can specify a number of typed Properties, and be initialised with some
 * values for those properties. It may specify a visual size and method for
 * drawing in a GUI.
 */
class DLLEXPORT Processor: public AutoProperties, virtual public Source, virtual public Sink, public MultiSource, public MultiSink, public Groupable
{
private:
	/**
	 * Has to be of type Processor ** since it takes ownership of and deletes the data at
	 * the * at end of thread. Clearly we don't want it to delete the actual processor!
	 */
	static QThreadStorage<Processor **> theOwningProcessor;

	//@{
	/** Basic properties. */
	QString theName;
	const QString theType;
	int theWidth, theHeight, theRedrawPeriod, theMinWidth, theMinHeight;
	bool m_isResizable;
	//@}

	//@{
	/** Start/stop subsystem. @sa wantToStopNow() haveStoppedNow() */
	friend class DomProcessor;
	friend class MultiProcessor;
	mutable QFastMutex theStop;
	QFastWaitCondition theAllDoneChanged;
	bool theIOSetup, theStopping, theIsInitialised, theAllDone, theIsActive;
	void doInit(const QString &name, ProcessorGroup *group, const Properties &properties);
	//@}

protected:
	/** @internal
	 * Causes a bail exception. Used to exit from the processor thread in a controlled
	 * manner.
	 */
	void bail();

	virtual void start() {}
	virtual void wait() {}

	virtual void getReadyForStopping() {}

	/** @internal
	 * Override to execute useful commands for when the processor should be stopped.
	 */
	virtual void wantToStopNow() {}

	/** @internal
	 * Override to execute useful commands for when the processor has stopped, in the stop()
	 * thread.
	 *
	 * This differs from processorStopped, which is executed in the processor() thread
	 * directly after exit from processor(), and thus is meant for executing deinit code
	 * for processor(). This is meant for general controller code as it is executed after
	 * the "stopping" process has finished when the state of both threads is known. It is
	 * used by Demux (for neccessary buffer trapdoor code) since processorStopped cannot
	 * guarantee what state the stop() code is in (it may still be in wantToStopNow(), for
	 * example).
	 */
	virtual void haveStoppedNow() {}

private:
	//@{
	/** Typing subsystem. */
	mutable QFastMutex theConfirming;
	Types theTypesCache;
	QVector<uint> theSizesCache;
	bool theTypesConfirmed;
	//@}

protected:
	mutable uint theGuardsCrossed;
public:
	uint guardsCrossed() const { return theGuardsCrossed; }
private:

	//@{
	/** Error subsystem. */
	mutable QFastMutex theErrorSystem;
	QFastWaitCondition theErrorWritten;
	ErrorType theError;
	int theErrorData;
	QString theCustomError;
	//@}

public:
	/** @internal
	 * @return Returns the Processor that the calling thread is operating under, or 0
	 * if no Processor is associated with calling thread.
	 */
	static Processor *threadProcessor();
	static void unsetThreadProcessor();
	void setThreadProcessor();

private:
	//@{
	/** Connection subsystem. */
	uint m_inputSpace;
	uint m_outputSpace;
	QVector<xLConnection *> theInputs;
	QVector<LxConnection *> theOutputs;
	friend class RLConnection;
	friend class ProcessorForwarder;
	void dropInput(uint index);
	//@}

	//@{
	/** Multiplicity subsystem. */
	MultiplicityType theMulti;
protected:
	virtual void onMultiplicitySet(uint _m);
private:
	uint theGivenMultiplicity;
	uint theHardMultiplicity;
	//@}

	//@{
	/** Reimplementations from Multiplicative. */
public:
	virtual void resetMulti();
	bool knowMultiplicity() const { return theGivenMultiplicity != Undefined ; }
	uint multiplicity() const { return theGivenMultiplicity; }
	MultiplicityType multi() const { return theMulti; }
private:
	//@}

	//@{
	/** Reimplementations from MultiSource. */
	virtual ProcessorPort sourcePort(uint _i, uint) { return (*this)[_i]; }
	virtual uint numMultiOutputs() const { return 1; }
	virtual void connectCheck() const;
	//@}

	//@{
	/** Reimplementation from MultiSink. */
	virtual ProcessorPort sinkPort(uint _i, uint) { return (*this)[_i]; }
	virtual uint numMultiInputs() const { return 1; }
	//@}

	//@{
	/** Plunger subsystem, in collaboration with Buffer. */
	friend class LLConnection;
	friend class LRConnection;
	friend class xLConnectionReal;
	friend class MLConnection;
	friend class HeavyProcessor;
	friend class CoProcessor;
	QVector<uint> thePlungedInputs;
	mutable QFastMutex thePlungerSystem;
	QVector<uint> thePlungersLeft, thePlungersNotified;
	bool thePlungersStarted, thePlungersEnded;
	virtual void startPlungers();
	virtual void plungerSent(uint index);
	virtual void noMorePlungers();
	//@}

	//@{
	/** Reimplementation from Sink. */
	virtual void plunged(uint index);
	virtual void resetTypes();
	//@}

	//@{
	/** Reimplementations from Source. */
	virtual void doRegisterOut(LxConnection *me, uint port);
	virtual void undoRegisterOut(LxConnection *me, uint port);
	//@}

	//@{
	/** Reimplementations from Sink. */
	virtual void doRegisterIn(xLConnection *me, uint port);
	virtual void undoRegisterIn(xLConnection *me, uint port);
	virtual bool readyRegisterIn(uint sinkIndex) const;
	virtual void waitToStop() { wait(); }
	virtual bool waitUntilReady();
	//@}

	//@{
	/** Reimplementations from Source and Sink. */
protected:
	virtual void checkExit();
	//@}

public:
	/**
	 * @name Derived Methods for use in derived classes.
	 * These methods are protected to allow only new (derived) types of Processor
	 * class to use them.
	 *
	 * They are not used for manipulating Processor objects externally.
	 */
	//@{

	/**
	 * Used from Processor::processor to read from the data source.
	 * Asserts @a index is less than the number of inputs and @a index is connected.
	 *
	 * @param index The input port index for which Connection is required.
	 * @return Returns the input Connection object.
	 * @sa numInputs() output()
	 */
	xLConnection &input(uint index)
	{
#ifdef EDEBUG
		assert(index < theInputs.size());
		assert(theInputs[index]);
#endif
		return *(theInputs[index]);
	}

	/**
	 * Used from Processor::processor to write to the data sink(s).
	 * Asserts @a index is less than the number of outputs and @a index is connected.
	 *
	 * @param index The output port index for which Connection is required.
	 * @return The output Connection object at output port @a index.
	 * @sa numOutputs() input()
	 */
	LxConnection &output(uint index)
	{
#ifdef EDEBUG
		assert(index < theOutputs.size());
		assert(theOutputs[index]);
#endif
		return *(theOutputs[index]);
	}

protected:
	/**
	 * Insert a plunger into all output connections immediately.
	 *
	 * Plungers can be thought of a stream seperators, not disimilar from those
	 * "Next customer please" signs you get at supermarket checkouts to
	 * determine where your items start and the previous person's stops.
	 *
	 * @note This should never be called when there is any unpushed data on any
	 * of the outputs. Doing so will may in stream corruption.
	 */
	void plunge();

	/**
	 * Call this from initFromProperties to initialise I/O connections.
	 *
	 * @param inputs The number of inputs this Processor should have. May be Undefined iff
	 * the Processor has been declared as a multi of Input (not ConstInput though).
	 * @param outputs The number of outputs this Processor should have. May be Undefined iff
	 * the Processor has been declared as a multi of Output (not ConstOutput though).
	 * @sa setupVisual()
	 */
	void setupIO(uint _inputs, uint _outputs, uint _inputSpace = 1, uint _outputSpace = 1);

	/**
	 * Call this from initFromProperties to initialise the visual properties of
	 * the class.
	 *
	 * If this is not called, the size will default to 32x32 and no redraw.
	 *
	 * @param width The width of the drawing canvas. Should be a multiple of 10.
	 * @param height The height of the drawing canvas. Should be a multiple of 10.
	 * @param redrawPeriod The rate for which the processor's visual should
	 * be redrawn in milliseconds. A value of zero means no explicit redraw.
	 * @sa setupIO()
	 */
	void setupVisual(uint width, uint height, uint redrawPeriod = 0, uint minWidth = 32, uint minHeight = 32, bool _isResizable = false);

	/**
	 * Reimplement for to define how the processor should be drawn visually.
	 *
	 * @param _p The painting canvas onto which the visual may be drawn.
	 * @param _s The size of the area to draw onto. This may be different
	 * from the size specified in setupVisual(). You need not honour this
	 * paramater - if you elect to honour it, return true.
	 * @return true if @a _s was honoured. false if setupVisual() was honoured.
	 */
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
	virtual QString simpleText() const { return "?"; }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(0, 0, 160); }

	/**
	 * Reimplement to initialise any stuff that processor may need to be open/
	 * locked/whatever. Does nothing by default.
	 *
	 * Note this is executed inside the go() call, and as such MUST NEVER BLOCK
	 * under any circumstances, as doing so will almost certainly break the
	 * main program.
	 *
	 * This different from initFromProperties, since that represents a one-off
	 * initialiser for the object, like the constructor. This is called
	 * potentially many times in an object's life, each time it is started.
	 *
	 * Generally you wont have to use this since you'll be able to do any
	 * internal initialisations inside processor() much more easily. However
	 * this is useful if you need to reset some external or shared value that
	 * must happen before the main program gets past the starting stage.
	 *
	 * The other reason for using this method to specify initialisation
	 * procedures is because it facilitates error detection and reporting. By
	 * returning false, all startup can be simply aborted.
	 *
	 * @return false iff the object could not be started for some reason. The
	 * ErrorType will automatically be filled with ErrorType::Custom.
	 *
	 * @sa initFromProperties() processor() processorStopped()
	 */
	virtual bool processorStarted() { return true; }

	/** @overload
	 * Reimplement to initialise any stuff that processor may need to be open/
	 * locked/whatever. Does nothing by default.
	 *
	 * Note this is executed inside the go() call, and as such MUST NEVER BLOCK
	 * under any circumstances, as doing so will almost certainly break the
	 * main program.
	 *
	 * This different from initFromProperties, since that represents a one-off
	 * initialiser for the object, like the constructor. This is called
	 * potentially many times in an object's life, each time it is started.
	 *
	 * Generally you wont have to use this since you'll be able to do any
	 * internal initialisations inside processor() much more easily. However
	 * this is useful if you need to reset some external or shared value that
	 * must happen before the main program gets past the starting stage.
	 *
	 * The other reason for using this method to specify initialisation
	 * procedures is because it facilitates error detection and reporting. By
	 * returning false, all startup can be simply aborted.
	 *
	 * @param description A QString in which to place a description of the
	 * error, if any.
	 * @return false iff the object could not be started for some reason. The
	 * ErrorType will automatically be filled with ErrorType::Custom.
	 *
	 * @sa initFromProperties() processor() processorStopped()
	 */
	virtual bool processorStarted(QString &description) { description = QString::null; return true; }

	/**
	 * Reimplement to cleanup any stuff that processor may have left open/locked/
	 * whatever. Does nothing by default.
	 *
	 * Note this is executed directly after processor() has exited, in the same
	 * thread. There is another similar (internal) method haveStoppedNow() that you
	 * probably shouldn't touch unless you know you have to.
	 */
	virtual void processorStopped() { }

	/**
	 * Reimplement to (synchronously) handle any sub-data signals ("plungers").
	 * Does nothing by default.
	 *
	 * This will be called from inside the processor() thread when elements are
	 * being read. Make sure that this *under no circumstances* blocks, since
	 * there will likely be nothing to unblock it.
	 */
	virtual void receivedPlunger() {}

	/**
	 * Reimplement to provide property specifications and default values for this
	 * Processor.
	 *
	 * @return The Properties object that contains the property definitions and defaults.
	 */
	virtual PropertiesInfo specifyProperties() const { return PropertiesInfo(); }

	/**
	 * Initialises from the Properties; should call setupIO() and setupVisual() at least.
	 *
	 * @param properties The given properties.
	 */
	virtual void initFromProperties(Properties const& _p);
	virtual void updateFromProperties(Properties const&);
	virtual void initFromProperties() {}
	virtual void updateFromProperties() {}

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
	 */
	//TODO: enforce the same basic class rule.
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes) = 0;

	/**
	 * Reimplement to force the inputs' buffer size to be at least samples big, explicitly
	 * allowing a readSamples(s) to be legal, where s is the size given.
	 *
	 * This gets called *after* verifyAndSpecifyTypes(), but before specifyOutputSpace(), and
	 * so must be based only upon the types and any properties accumulated.
	 *
	 * The default implementation requests only a single sample's worth of data to be
	 * available.
	 *
	 * @param samples A correctly-sized array (well; QValueVector) into which your minima can
	 * be placed.
	 */
	virtual void specifyInputSpace(QVector<uint> &samples);
	virtual void requireInputSpace(QVector<uint> &samples) { specifyInputSpace(samples); } // for use with canprocess. won't skip the data for a plunger unless it's less than this amount.
	inline static uint allInputs(QVector<uint> const& _inputs) { uint ret = UINT_MAX; foreach (uint i, _inputs) if (i == Undefined) return 0; else ret = min(ret, i); return ret; }
	inline static uint anyInput(QVector<uint> const& _inputs) { uint ret = 0; foreach (uint i, _inputs) if (i != Undefined) ret = max(ret, i); return ret; }
	virtual uint cyclesAvailable(QVector<uint> const& _inputs) const { return allInputs(_inputs); }
	virtual bool specifyInputMode(uint) { return false; }

	/**
	 * Reimplement to force the outputs' buffer size to be at least samples big, explicitly
	 * allowing a makeScratchSamples(s) on an empty scratch to be legal, where s is the size
	 * given (to make a mSS(s) always efficient and never block, a minimum of 2*s should be
	 * given).
	 *
	 * This gets called *after* verifyAndSpecifyTypes() and specifyInputSpace(), and
	 * so can be based upon the types, properties and the inputs' buffer sizes.
	 *
	 * The default implementation requests only a single sample's worth of data to be
	 * available.
	 *
	 * @param samples A correctly-sized array (well; QValueVector) into which your minima can
	 * be placed.
	 */
	virtual void specifyOutputSpace(QVector<uint> &samples);

	/**
	 * Simple constructor.
	 *
	 * @param type This must the class name of the derived class. All sorts of things will
	 * break if it isn't.
	 * @param multi Declares the type of multiplicity this Processor offers. Defaults to
	 * NotMulti.
	 */
	Processor(const QString &type, MultiplicityType multi = NotMulti);

	//@}

public:
	/**
	 * @name Control Methods for the external control of Processor objects.
	 * This section of methods are for interfacing with Processor objects.
	 *
	 * They are generally not used when coding a new type of Processor class.
	 */
	//@{

	/**
	 * Returns information about the properties of the object and gives their default values.
	 *
	 * @return The processor's properties and their default values.
	 */
	const PropertiesInfo properties() const;

	/**
	 * Performs basic object construction that cannot otherwise be done in the constructor
	 * due to shared library limitations. Initialises the object fully ready for connect()s
	 * and then go().
	 *
	 * @param name This Processor instance's name, and should be unique across any group or
	 * network the processor is in.
	 * @param group A reference to the ProcessorGroup (if any) that you may use to control a
	 * set of objects.
	 * @param properties The properties with which to initialise this object. This defaults
	 * to an empty properties set. If a needed property is left undefined, the default
	 * value is taken from the properties() method.
	 */
	void init(const QString &name, ProcessorGroup &group, const Properties &properties = Properties()) { doInit(name, &group, properties); }

	/** @overload
	 * Performs basic object construction that cannot otherwise be done in the constructor
	 * due to shared library limitations. Initialises the object fully ready for connect()s
	 * and then go().
	 *
	 * @note The processor will not be initialised as a member of any group.
	 *
	 * @param name This Processor instance's name, and should be unique across any group or
	 * network the processor is in. This defaults to the empty name, which can be used iff
	 * the object is not contained in a ProcessorGroup.
	 * @param properties The properties with which to initialise this object. This defaults
	 * to an empty properties set. If a needed property is left undefined, the default
	 * value is taken from the properties() method.
	 */
	void init(const QString &name = "", const Properties &properties = Properties()) { doInit(name, 0, properties); }
	virtual void update(Properties const& _p) { updateFromProperties(_p); }

	virtual double secondsPassed() const;
	virtual double secondsPassed(float _s, uint _i = 0) const;

	/**
	 * Checks if the previously called init() has failed.
	 *
	 * @return false if there has been an error with the init(). true if initialisation has
	 * been deferred or suceeded. Undefined if init() hasn't been called yet.
	 */
	bool isInitFailed() const { return !(theIsInitialised); }

	/**
	 * Use this method for driving multiple inputs from one output.
	 *
	 * All data is explicitly copied to each connection buffer. Any type of connection
	 * may be used. Once called, just keep calling connect as many times as you need.
	 *
	 * @param sourceIndex The index of the output port you wish to drive multiple inputs
	 * from.
	 *
	 * @sa share() connect()
	 */
	void split(uint sourceIndex);

	/**
	 * Use this method for driving multiple inputs from one output.
	 *
	 * This explicitly shares the data, using only one buffer and is extremely
	 * efficient. Once called, just keep calling connect as many times as you need.
	 *
	 * @note All other connections from this output must be local. No remote connections
	 * may be made from this processor after this call. This means that the output must
	 * only go to other Processor objects in the same program (process space).
	 *
	 * @param sourceIndex The index of the output port you wish to drive multiple inputs
	 * with.
	 * @param bufferSize The minimum size of the connection buffer in elements. Defaults
	 * to 1. Under normal circumstances this will not need to be changed.
	 *
	 * @sa split() connect()
	 */
	void share(uint sourceIndex, uint bufferSize = 1);

	/**
	 * Create a connection to another Processor object running in this program.
	 * Connection uses a shared memory method, thus the processor must be local.
	 *
	 * @note If the source has already been shared()ed, then @a bufferSize is
	 * ignored.
	 *
	 * @param sourceIndex The index of the output port you wish to connect from.
	 * @param sink A pointer to the Processor object you wish to connect to.
	 * @param sinkIndex The input port of @a sink that you wish to connect to.
	 * @param bufferSize The minimum size of the connection buffer in elements. Defaults
	 * to 1. Under normal circumstances this will not need to be changed.
	 * @return A pointer to the outbound connection, if creation was successful,
	 * otherwise 0.
	 */
	const Connection *connect(uint sourceIndex, Sink *sink, uint sinkIndex, uint bufferSize = 1);

	/**
	 * Create a connection to another Processor object. Connection uses a TCP/IP
	 * method, thus the processor can be remote.
	 *
	 * @note This cannot be used on an output that has been share()d.
	 *
	 * @param sourceIndex The index of the output port you wish to connect from.
	 * @param sinkHost The TCP/IP hostname (or IP address) of the host on which the
	 * object resides.
	 * @param sinkKey The predefined key into the remote session. This comes from the
	 * remote node's server software.
	 * @param sinkProcessorName The name of the Processor object you wish to connect to.
	 * @param sinkIndex The input port of @a sink that you wish to connect to.
	 * @param bufferSize The minimum size of the connection buffer in elements. Defaults
	 * to 1. Under normal circumstances this will not need to be changed.
	 * @return A pointer to the outbound connection, if creation was successful,
	 * otherwise 0.
	 */
	const Connection *connect(uint sourceIndex, const QString &sinkHost, uint sinkKey, const QString &sinkProcessorName, uint sinkIndex, uint bufferSize = 1);

	bool isConnected(uint _sourceIndex) const { return theOutputs[_sourceIndex]; }

	/*
	const Connection *connect(uint sourceIndex, Endpoint sinkHost, uint sinkKey, const QString &sinkProcessorName, uint sinkIndex, uint bufferSize = 1);
	void inform(Processor &target);
	*/

	/**
	 * Destoy a connection to another Processor object.
	 *
	 * @param index The output port, which *must* be connected. If the port
	 * has been split() or share()d, then all connections will be deleted.
	 */
	void disconnect(uint index);

	/**
	 * Destoy all outgoing connections.
	 */
	void disconnectAll();

	/**
	 * Returns convenience object that represents one of this object's input/outputs.
	 * Can be used for creating connections.
	 *
	 * @param port The index of the port that you wish to reference.
	 * @return A ProcessorPort object that refers to the given i/o port. Due to the
	 * linearity of the parameter, input and output cannot be differentiated here
	 * and is instead left to later on in its usage for clarification.
	 */
	ProcessorPort operator[](uint port) { return ProcessorPort(this, port); }

	/**
	 * Checks the types (recursively if necessary) and reports if all is ok. Sets
	 * up Connections types on its way. Note this is done automatically if necessary
	 * at go() anyway.
	 *
	 * @return true if all is fine.
	 *
	 * @sa go() errorType() errorData()
	 *
	 * Reimplementation from Source.
	 */
	virtual bool confirmTypes();

	/**
	 * Make the thing start doing stuff. i.e. Starts processor thread.
	 *
	 * @return true, if started correctly.
	 *
	 * @sa waitUntilGoing() confirmTypes() errorType() errorData() stop()
	 */
	bool go();

	virtual bool isRunning() const { return false; }

	/**
	 * Blocks until processor is active, and gives error information if processor startup
	 * failed along the way.
	 *
	 * @param errorData If non-zero, an integer will be placed where this points in case
	 * error occurs in startup. This may represent some numerical data for the error. It
	 * depends on the error type, but may e.g. point to a specific port. Defaults to zero.
	 * @return The type of error that caused a problem or NoError if the Processor was
	 * started correctly.
	 *
	 * @sa go()
	 */
	ErrorType waitUntilGoing(int *errorData = 0);
	Connection::Tristate isGoingYet();

	/**
	 * Blocks until the Processor object is finished and has exited it's main
	 * processor() method.
	 *
	 * This is not the same as a Processor object that has stop()ed. In this
	 * case, the object is in a "zombie" state. While not yet stopped, it will
	 * never again process any data.
	 */
	virtual void waitUntilDone() {}

	/**
	 * Get the last error (if any) from starting up.
	 *
	 * @return Processor::NoError in the case of no error and Processor::Pending
	 * if the processor is still starting up. To avoid Pending, call waitUntilGoing()
	 * first.
	 *
	 * @sa waitUntilGoing().
	 */
	ErrorType errorType() const { QFastMutexLocker lock(&theErrorSystem); return theError; }

	/**
	 * Get any (numerical) data associated with the error condition given
	 * by errorType().
	 *
	 * This value is undefined in the case of no error (or Pending startup).
	 *
	 * @return The accompanying numerical data.
	 *
	 * @sa errorType
	 */
	virtual long int errorData() const { QFastMutexLocker lock(&theErrorSystem); return theErrorData; }

	/**
	 * Get a string which is a human readable interpretation of the last
	 * error that occured.
	 *
	 * @return The error string.
	 */
	QString error() const;

	/**
	 * Pauses the processor temporarily. No processing will occur until unpause()
	 * is called. The unpause() call *must* be called by the same thread as the
	 * pause() call.
	 *
	 * The Processor should not be currently paused.
	 *
	 * @sa unpause() paused()
	 */
	virtual void pause() {}

	/**
	 * Resumes the processor. This call *must* be called by the same thread as
	 * the pause() call.
	 *
	 * Asserts the processor is already paused.
	 *
	 * @sa pause() paused()
	 */
	virtual void unpause() {}

	/**
	 * Get the pause state of the Processor.
	 *
	 * @return true is the processor is currently paused, false if not.
	 *
	 * @sa pause() unpause()
	 */
	virtual bool paused() const { return false; }


	/**
	 * Make the thing stop doing stuff. i.e. Cancels processor thread.
	 *
	 * @sa go() reset()
	 */
	void stop();

	/**
	 * Resets anything neccessary for another start() to happen. All processors
	 * connected to this must also be stop()ed.
	 *
	 * @sa stop()
	 */
	void reset();

	/**
	 * Gets the number of inputs this processor has.
	 *
	 * @return The number of inputs.
	 */
	uint numInputs() const { return theInputs.size(); }

	/**
	 * Gets the number of outputs this processor has.
	 *
	 * @return The number of outputs.
	 */
	uint numOutputs() const { return theOutputs.size(); }

	/**
	 * Gets the minimum width of the processor's image. Used by the Nite for drawing.
	 *
	 * @return The image's width.
	 */
	uint minWidth() const { return theMinWidth; }

	/**
	 * Gets the minimum height of the processor's image. Used by the Nite for drawing.
	 *
	 * @return The image's height.
	 */
	uint minHeight() const { return theMinHeight; }

	/**
	 * Gets the natural width of the processor's image. Used by the Nite for drawing.
	 *
	 * @return The image's width.
	 */
	uint width() const { return theWidth; }

	/**
	 * Gets the natural height of the processor's image. Used by the Nite for drawing.
	 *
	 * @return The image's height.
	 */
	uint height() const { return theHeight; }

	bool isResizable() const { return m_isResizable; }

	/**
	 * Gets the (automatic) redraw period of the processor in milliseconds. Used by
	 * the Nite for drawing.
	 *
	 * @return The automatic redraw rate.
	 */
	uint redrawPeriod() const { return theRedrawPeriod; }

	/**
	 * Front-end routing for drawing the Processor to a canvas. Used by the Nite
	 * for drawing.
	 *
	 * @param _p The painting canvas onto which the drawing should take place.
	 * @param _s The size to draw if possible.
	 * @returns true if it draws according to the supplied size. false if it draws
	 * according to width()/height().
	 */
	bool draw(QPainter& _p, QSizeF const& _s) const;
	QColor outlineColour() const { return specifyOutlineColour(); }

	/**
	 * Determine how full the buffer at input @a index is. Used by the Nite for
	 * drawing.
	 *
	 * @param index The input port's index.
	 * @return A float between 0 and 1 which is the fractional filled-ness of the
	 * port's connection buffer.
	 */
	float bufferCapacity(uint index);

	/**
	 * Gets the (instance-wise) name of the processor. This is unique for any group
	 * or network the processor is in.
	 *
	 * @return The name of the Processor.
	 */
	virtual QString name() const { return theName; }

	/**
	 * Gets the (class-wise) type of the processor.
	 *
	 * @return The type of the Processor.
	 */
	const QString &type() const { return theType; }

	//@}

	/**
	 * Simple, virtual, destructor.
	 */
	virtual ~Processor();
};

class DLLEXPORT CoProcessor: public Processor, public QTask
{
public:
	CoProcessor(const QString &type, const MultiplicityType multi = NotMulti);

	virtual bool isRunning() const { return QTask::isRunning(); }
	virtual void waitUntilDone();

protected:
	virtual int process() { return WillNeverWork; }
	virtual int canProcess() { return CanWork; }
	virtual QString taskName() const { return name(); }

private:
	virtual void start() { QTask::start(); }
	virtual void wait() { QTask::wait(); }

	virtual int doWork();
	virtual void onStopped();

	virtual int cyclesReady();
};

class DLLEXPORT HeavyProcessor: protected QThread, public Processor
{
public:
	enum
	{	Guarded = 1 ///< Indicates a subclass is able to finish when input EOS is given.
	};

	HeavyProcessor(const QString &type, const MultiplicityType multi = NotMulti, uint flags = 0);

	virtual bool isRunning() const { return QThread::isRunning(); }
	virtual void waitUntilDone();

	virtual void pause();
	virtual void unpause();
	virtual bool paused() const { QFastMutexLocker lock(&thePause); return thePaused; }

protected:
	/**
	 * Reimplement to control execution to do processing.
	 * Use Buffer's waitFor methods to control flow for theInputs/theOutputs.
	 * This doesn't ever have to return explicitly - typically a
	 * @code while (thereIsInputForProcessing()) {} @endcode
	 * loop will do the job fine, though you're free to implement anything you like.
	 * If there is a main loop, you should add thereIsInputForProcessing() as a guard upon it (it will,
	 * however, always return true).
	 */
	virtual void processor() {}

	/**
	 * Blocks until either:
	 *
	 * 1) There will never again be enough input for any processing. In this
	 * instance, it returns false.
	 *
	 * 2) There are at least @a samples samples ready for reading immediately.
	 * It guarantees that reading this data will not require any more plunging.
	 * In this case, true is returned.
	 *
	 * If there are any plungers to be read immediately, then they are read.
	 * This is only the case if the next read would cause a plunger to be read.
	 *
	 * @param samples The number of samples that there should be on an input
	 * for reading in order for this to exit true.
	 * @return true iff a read of @a samples will not block or cause a plunger
	 * to be read, false iff no more data can *ever* be read.
	 */
	bool thereIsInputForProcessing(uint samples);

	/** @overload
	 * Blocks until either:
	 *
	 * 1) There will never again be enough input for any processing. In this
	 * instance, it returns false.
	 *
	 * 2) There are at least specifyInputSpace() samples (for each input)ready
	 * for reading immediately. It guarantees that reading this data will not
	 * require any more plunging. In this case, true is returned.
	 *
	 * If there are any plungers to be read immediately, then they are read.
	 * This is only the case if the next read would cause a plunger to be read.
	 *
	 * @return true iff a read of @a samples will not block or cause a plunger
	 * to be read, false iff no more data can *ever* be read.
	 */
	bool thereIsInputForProcessing();

	bool guard();

private:
	virtual void start() { QThread::start(); }
	virtual void wait() { QThread::wait(); }
	virtual void getReadyForStopping();

	/** Thread subsystem. @sa threadProcessor() */
	virtual void run();

	//@{
	/** Pausing subsystem. */
	mutable QFastMutex thePause;
	QFastWaitCondition theUnpaused;
	bool thePaused;
	//@}

	uint theFlags;
};

}
