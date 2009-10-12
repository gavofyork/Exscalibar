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

namespace rGeddei
{

class AbstractProcessor;

/** @ingroup rGeddei
 * @brief Model of an AbstractProcessor connection port.
 * @author Gav Wood <gav@kde.org>
 *
 * This is a convenience class meant primarily as an adjunct to the
 * AbstractProcessor class to allow subscripting (['s and ]'s) to be used to
 * manage connections.
 *
 * See the operators and methods for more information about general use.
 *
 * @note There is no difference between input and output ports in
 * AbstractProcessorPort. An Abstract ProcessorPort object determines use
 * depending on context.
 */
class DLLEXPORT AbstractProcessorPort
{
	friend class AbstractProcessor;
	friend class AbstractProcessorIn;

	AbstractProcessor *theParent;
	uint thePort, theBufferSize;

	AbstractProcessorPort(AbstractProcessor *parent, uint port) : theParent(parent), thePort(port), theBufferSize(1) {}

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
	AbstractProcessorPort &setSize(uint bufferSize) { theBufferSize = bufferSize; return *this; }

	/**
	 * Split the output of a Processor.
	 *
	 * See AbstractProcessor::split() for more details.
	 *
	 * Example, given P, Q are RemoteProcessor objects:
	 *
	 * @code
	 * // Share the first output port of P
	 * P[0].split();
	 * // Connect the first output port of P to the first input port of Q
	 * P[0].connect(Q[0]);
	 * // Connect the same output to the second port of Q
	 * P[0].connect(Q[1]);
	 * @endcode
	 */
	bool split();

	/**
	 * Share the output of a Processor.
	 *
	 * See AbstractProcessor::share() for more details.
	 *
	 * Example, given P, Q are RemoteProcessor objects, in the same session:
	 *
	 * @code
	 * // Share the first output port of P
	 * P[0].share();
	 * // Connect the first output port of P to the first input port of Q
	 * P[0].connect(Q[0]);
	 * // Connect the same output to the second port of Q
	 * P[0].connect(Q[1]);
	 * @endcode
	 */
	bool share();

	/**
	 * Assert that this port represents an output port, and create a
	 * connection between it and @a input (asserted as an input port).
	 *
	 * Example, given P and Q are AbstractProcessor subclass objects, P with
	 * two output ports and Q with two input ports:
	 *
	 * @code
	 * P[0].connect(Q[0]);
	 * P[1].connect(Q[1]);
	 * @endcode
	 *
	 * @param input AbstractProcessorPort object which, it is asserted
	 * describes an input port.
	 *
	 * @sa disconnect()
	 */
	bool connect(const AbstractProcessorPort &input);

	/**
	 * Assert that this port represents an output port and delete the
	 * connection it is sourcing.
	 *
	 * Example, given P and Q are AbstractProcessor objects:
	 *
	 * @code
	 * P[0].connect(Q[0]);
	 * // Do some processing...
	 * P[0].disconnect();
	 * @endcode
	 *
	 * @sa connect()
	 */
	bool disconnect();

	/**
	 * Shorthand operator for connecting. Does the same as the connect()
	 * method.
	 *
	 * Example, given P and Q are AbstractProcessor objects:
	 *
	 * @code
	 * P[0] >>= Q[0];
	 * // Do some processing...
	 * P[0].disconnect();
	 * @endcode
	 *
	 * @param input AbstractProcessorPort object which, it is asserted
	 * describes an input port.
	 *
	 * @sa connect()
	 */
	bool operator>>=(const AbstractProcessorPort &input) { return connect(input); }

	/**
	 * Shorthand operator for disconnecting. Does the same as the disconnect()
	 * method.
	 *
	 * Example, given P and Q are AbstractProcessor objects:
	 *
	 * @code
	 * P[0].connect(Q[0]);
	 * // Do some processing...
	 * P[0]--;
	 * @endcode
	 *
	 * @sa disconnect()
	 */
	bool operator--(int) { return disconnect(); }
};

}
