/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_RLCONNECTION_H
#define _GEDDEI_RLCONNECTION_H

#include <qthread.h>
#include <qmutex.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "qfastwaitcondition.h"
#include "qsocketsession.h"
#include "xlconnectionreal.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <qtextra/qsocketsession.h>
#include <geddei/xlconnectionreal.h>
#endif
using namespace Geddei;

class Q3SocketDevice;

namespace Geddei
{

class Processor;

/** @internal @ingroup Geddei
 * @brief Embodiment of Connection between a remote socket and a local Sink object.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * A realisation of the Connection flow-control class framework.
 * This shunts data from a network socket into it's buffer for use (on the front-line)
 * with the processor object, along with looking after any other comms neccessary,
 * such as for type identifiation/synchronisation.
 */
class DLLEXPORT RLConnection: public xLConnectionReal, protected QThread
{
	bool theBeingDeleted, theHaveType;
	QSocketSession theSource;
	QFastWaitCondition theGotType;
	QMutex theGotTypeM;

	//* Reimplementation from QThread.
	virtual void run();

	//* Reimplementation from xLConnection.
	virtual const bool pullType();

public:
	/**
	 * Simple constructor, for developer's use.
	 */
	RLConnection(Q3SocketDevice *sourceSocketDevice, Sink *newSink, int newSinkIndex, uint bufferSize);

	/**
	 * Simple destructor.
	 */
	~RLConnection();
};

}

#endif
