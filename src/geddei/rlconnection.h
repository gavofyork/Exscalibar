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

class QTcpSocket;

namespace Geddei
{

class Processor;

/** @internal @ingroup Geddei
 * @brief Embodiment of Connection between a remote socket and a local Sink object.
 * @author Gav Wood <gav@kde.org>
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
	QFastMutex theGotTypeM;

	//* Reimplementation from QThread.
	virtual void run();

	//* Reimplementation from xLConnection.
	virtual bool pullType();

public:
	/**
	 * Simple constructor, for developer's use.
	 */
	RLConnection(QTcpSocket *sourceSocketDevice, Sink *newSink, int newSinkIndex, uint bufferSize);

	/**
	 * Simple destructor.
	 */
	~RLConnection();
};

}
