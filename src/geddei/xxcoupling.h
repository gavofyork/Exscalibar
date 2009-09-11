/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_XXCOUPLING_H
#define _GEDDEI_XXCOUPLING_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "bufferdatas.h"
#else
#include <geddei/bufferdatas.h>
#endif

namespace Geddei
{

class SignalTypeRefs;
class Properties;

/** @internal @ingroup Geddei
 * @brief Abstract class to define the Sub/DomProcessor coupling mechanism.
 * @author Gav Wood <gav@kde.org>
 */
class xxCoupling
{
protected:
	/**
	 * Transmits and registers a processing transaction from the LHS to the
	 * RHS.
	 *
	 * @param d is the data to be processed, which may be legally empty. It may
	 * contain a plunger iff chunks == 0.
	 * @param chunks is the number of chunks that the data represents, which
	 * may be legally zero iff @a d is empty. In this case the transaction is a
	 * plunger.
	 */
	virtual void transact(const BufferDatas &d, uint chunks) = 0;

	/**
	 * Requests the next transaction results to be returned from the RHS.
	 *
	 * @param timeTaken Pointer to an integer with if non-null will be filled
	 * with the time taken for the data to be delivered in ms.
	 * @return the resultant data. This may be empty legally, in which case the
	 * transaction given was a plunger.
	 */
	virtual BufferDatas deliverResults(uint *timeTaken = 0) = 0;

	/**
	 * Actions to be performed by the LHS when stopping the processor.
	 */
	virtual void stoppingL() {}

	/**
	 * Actions to be performed by the LHS with the processor having been
	 * stopped.
	 */
	virtual void stoppedL() {}

	/**
	 * Actions to be performed by the RHS when stopping the processor.
	 */
	virtual void stoppingR() {}

	/**
	 * Actions to be performed by the RHS with the processor having been
	 * stopped.
	 */
	virtual void stoppedR() {}

public:
	/**
	 * Processes the chunks given asynchronously, returns immediately.
	 */
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _chunks) = 0;

	/**
	 * @returns true iff another processChunks operation may commence.
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
	virtual void specifyTypes(const SignalTypeRefs &inTypes, const SignalTypeRefs &outTypes) = 0;

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

	/**
	 * Convenience function that combines the two protected stopping methods
	 * (in the correct order).
	 *
	 * Called when SubProcessor will be stopped.
	 */
	void stopping() { stoppingR(); stoppingL(); }

	/**
	 * Convenience function that combines the two protected stopped methods
	 * (in the correct order).
	 *
	 * Called when SubProcessor has been stopped.
	 */
	void stopped() { stoppedL(); stoppedR(); }

	/**
	 * Virtual destructor.
	 */
	virtual ~xxCoupling() {}
};

};

#endif
