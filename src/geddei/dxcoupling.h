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

#include <qmutex.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "qfastwaitcondition.h"
#include "xxcoupling.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <geddei/xxcoupling.h>
#endif

namespace Geddei
{

class DomProcessor;
class BufferReader;

/** @internal @ingroup Geddei
 * @brief Refinement of a Coupling for LHS attachment to a DomProcessor object.
 * @author Gav Wood <gav@kde.org>
 *
 * This class connects to the DomProcessor directly. The right side is left
 * abstract.
 *
 * This class represents the left side of a remote DRCoupling.
 * All overrided commands are essentially just passed down the line
 * with arguments serialised as neccessary.
 *
 * @note If you subclass this in the future (for another mixin), be sure to
 * call ratify() from the constructor of that mixin. It really belongs in the
 * constructor of this class, but it requires methods that will only be defined
 * after the full mixed contruction has occured, i.e. it musty be done in the
 * mixed constructor.
 */
class DxCoupling : virtual public xxCoupling
{
public:
	/**
	 * Basic constructor.
	 */
	DxCoupling(DomProcessor *dom);

	/**
	 * Default destructor.
	 */
	virtual ~DxCoupling();

	/**
	 * Processes the chunks given asynchronously, returns immediately.
	 */
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _chunks) = 0;

	/**
	 * @returns true iff another processChunks() operation may commence.
	 */
	virtual bool isReady() = 0;

	/**
	 * Used by LHS to specify the input/output types to the SubProcessor.
	 * Both are const, since these are already checked as being correct by the
	 * primary. An extra check can be made to make sure that the outTypes given
	 * are equal to those received from the verifyAndSpecifyTypes.
	 *
	 * @note The call to verifyAndSpecifyTypes from this method *MUST* still
	 * be made.
	 *
	 * @param inTypes The "array" of types that describe the input
	 * connection(s).
	 * @param outTypes The "array" of types that describe the output
	 * connection(s).
	 */
	virtual void specifyTypes(const Types &inTypes, const Types &outTypes) = 0;

	/**
	 * Used by the LHS to initialise the properties of the SubProcessor.
	 *
	 * @param p The Properties object in which the properties are given.
	 */
	virtual void initFromProperties(const Properties &p) = 0;

	/**
	 * Used by the LHS to start the SubProcessor's processing thread.
	 */
	virtual void go() = 0;

	/**
	 * Used by the LHS to stop the SubProcessor's processing thread.
	 */
	virtual void stop() = 0;

	/**
	 * Used by the LHS to (re)define the num of inputs/outputs of the SubProcessor.
	 *
	 * @param numInputs The number of inputs for the SubProcessor.
	 * @param numOutputs The number of outputs for the SubProcessor.
	 */
	virtual void defineIO(uint numInputs, uint numOutputs) = 0;

protected:
	DomProcessor *theDomProcessor;
	uint theLastTimeTaken;
};

}
