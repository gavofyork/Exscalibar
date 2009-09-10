/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_PROCESSORPORT_H
#define _GEDDEI_PROCESSORPORT_H

namespace Geddei
{

class Processor;
class MultiProcessor;

/** @ingroup Geddei
 * @brief Model of a Processor connection port.
 * @author Gav Wood <gav@kde.org>
 *
 * This is a convenience class meant primarily as an adjunct to the Processor
 * class to allow subscripting (['s and ]'s) to be used to manage connections.
 *
 * See the operators and methods for more information about general use.
 *
 * @note There is no difference between input and output ports in
 * ProcessorPort. A ProcessorPort object determines use depending on context.
 */
class DLLEXPORT ProcessorPort
{
	friend class Processor;

	Processor *theParent;
	uint thePort, theBufferSize;

	/**
	 * Friend constructor for use by Processor. Users (well - developers)
	 * should not be creating their own ProcessorPort objects.
	 *
	 * @param parent The Processor object that this is a port of.
	 * @param port The port number.
	 * @param bufferSize A buffer minimum size. Typically left at one.
	 */
	ProcessorPort(Processor *parent, uint port, uint bufferSize = 1): theParent(parent), thePort(port), theBufferSize(bufferSize) {}

public:
	/**
	 * Set a minimum size for the buffer (in elements).
	 *
	 * This method rarely needs to be used. It is generally considered the job
	 * of the Processor subclass to determine how much buffer space if needed,
	 * however this method may be used in some extreme cases to tweak buffer
	 * usage in the case that a particular amount of high-speed memory is
	 * available.
	 *
	 * Ignore this otherwise.
	 *
	 * @param bufferSize The minimum number of elements to be used for this
	 * connection's buffer.
	 * @return A reference to this object.
	 *
	 * @sa Processor::specifyInputSpace() @sa Processor::specifyOutputSpace()
	 */
	ProcessorPort &setSize(uint bufferSize);

	/**
	 * Share the output of a Processor object.
	 *
	 * See Processor::share() for more details.
	 *
	 * Example, given P, Q, R are Processor subclass objects:
	 * @code
	 * // Share the first output port of P
	 * P[0].share();
	 * // Connect the first output port of P to the first input port of Q
	 * P[0].connect(Q[0]);
	 * // Connect the same output to the second port of Q
	 * P[0].connect(Q[1]);
	 * // Connect the same output to the first port of R
	 * P[0].connect(R[0]);
	 * @endcode
	 */
	void share() const;

	/**
	 * Split the output of a Processor object.
	 *
	 * See Processor::split() for more details.
	 *
	 * Example, given P, Q are Processor subclass objects:
	 *
	 * @code
	 * // Share the first output port of P
	 * P[0].split();
	 * // Connect the first output port of P to the first input port of Q
	 * P[0].connect(Q[0]);
	 * // Connect the same output to the second port of Q
	 * P[0].connect(Q[1]);
	 * // Connect the same output to the first input of some remote Processor
	 * P.connect(0, someRemoteHost, someRemoteKey, someProcessorsName, 0);
	 * @endcode
	 */
	void split() const;

	/**
	 * Assert that this port represents an output port, and create a
	 * connection between it and @a input (asserted as an input port).
	 *
	 * Example, given P and Q are Processor subclass objects, P with two output
	 * ports and Q with two input ports:
	 *
	 * @code
	 * P[0].connect(Q[0]);
	 * P[1].connect(Q[1]);
	 * @endcode
	 *
	 * @param input ProcessorPort object which, it is asserted describes an
	 * input port.
	 *
	 * @sa disconnect()
	 */
	void connect(const ProcessorPort &input) const;
	
	void connect(MultiProcessor &input) const;

	/**
	 * Assert that this port represents an output port and delete the
	 * connection it is sourcing.
	 *
	 * Example, given P and Q are Processor subclass objects:
	 *
	 * @code
	 * P[0].connect(Q[0]);
	 * // Do some processing...
	 * P[0].disconnect();
	 * @endcode
	 *
	 * @sa connect()
	 */
	void disconnect() const;

	/**
	 * Shorthand operator for connecting. Does the same as the connect()
	 * method.
	 *
	 * Example, given P and Q are Processor subclass objects:
	 *
	 * @code
	 * P[0] >>= Q[0];
	 * // Do some processing...
	 * P[0].disconnect();
	 * @endcode
	 *
	 * @param input ProcessorPort object which, it is asserted describes an
	 * input port.
	 *
	 * @sa connect()
	 */
	void operator>>=(const ProcessorPort &input) const { connect(input); }
	
	/**
	 * Shorthand operator for connecting to a multiprocessor. Does the same as
	 * the connect() method. This will share the stream and connect each to the
	 * MultiProcessor's inputs. The MultiProcessor must already know its
	 * multiplicity.
	 * 
	 * This port must not already be connected, split or shared.
	 *
	 * Example, given P is a Processor subclass object, Q is a MultiProcessor:
	 *
	 * @code
	 * P[0] >>= Q;
	 * // Do some processing...
	 * P[0].disconnect();
	 * @endcode
	 *
	 * @param input MultiProcessor object.
	 *
	 * @sa connect()
	 */
	void operator>>=(MultiProcessor &input) const { connect(input); }

	/**
	 * Shorthand operator for disconnecting. Does the same as the disconnect()
	 * method.
	 *
	 * Example, given P and Q are Processor subclass objects:
	 *
	 * @code
	 * P[0].connect(Q[0]);
	 * // Do some processing...
	 * P[0]--;
	 * @endcode
	 *
	 * @sa disconnect()
	 */
	void operator--(int) const { disconnect(); }
};

}

#endif
