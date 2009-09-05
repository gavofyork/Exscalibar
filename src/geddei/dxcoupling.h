/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_DXCOUPLING_H
#define _GEDDEI_DXCOUPLING_H

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
 * @author Gav Wood <gav@cs.york.ac.uk>
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
	//* Reimplementations from xxCoupling
	virtual void stoppingL();
	virtual void stoppedL();

	/**
	 * Just checks whether an outgoing transaction is safe to send (i.e. wont result in a blockage).
	 * Blocks until it's ok.
	 * @return Returns false if it exitted due to stopping.
	 * UNSAFE: Needs theDataX to be locked throughout the *entire* transaction, from start to finish.
	 */
	bool transactCheckerUNSAFE();

protected:
	// TODO: Look into moving any accesses to theReaders into this class to get rid of friend.
	friend class DomProcessor;

	DomProcessor *theDomProcessor;
	Q3ValueVector<BufferReader *> theReaders;

	int theLoad;
	uint theLastTimeTaken;

public:
	/**
	 * Skips @a samples from the inputs.
	 */
	void skip(uint samples);

	/**
	 * Skips @a samples from the inputs, gets rid of any plungers immediately after them and
	 * sends the plunger to the SubProc.
	 */
	void skipPlungeAndSend(uint samples);

	/**
	 * Reads @a samples from the inputs and sends the data to the SubProc.
	 * @a chunks *must* be the number of chunks that @a samples represents.
	 */
	void peekAndSend(uint samples, uint chunks);

	/**
	 * Conceptually similar to deliverResults, in that it returns the resultant
	 * data from the last transaction or empty if the last transaction was a
	 * plunger. However this method adds the benefit of ordering so that it may
	 * be called *before* the corresponding transaction has been called and will
	 * still return the correct results (after blocking, of course). This is
	 * neccessary for its use in DomProcessor::eater.
	 */
	BufferDatas returnResults();

	/**
	 * Basic constructor.
	 */
	DxCoupling(DomProcessor *dom);

	/**
	 * Default destructor.
	 */
	virtual ~DxCoupling();
};


};

#endif
