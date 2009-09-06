/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include "properties.h"
#include "signaltyperefs.h"
#include "subprocessor.h"
#include "xscoupling.h"

#define MESSAGES 0

namespace Geddei
{

xSCoupling::xSCoupling(SubProcessor *subProc) : theSubProc(subProc)
{
	subProc->theCoupling = this;
}

xSCoupling::~xSCoupling()
{
	theSubProc->theCoupling = 0;
}

void xSCoupling::stoppingR()
{
	if (MESSAGES) qDebug("xSCoupling::stopping(): Getting ready to stop. No more transactions allowed on %p.", theSubProc);
	{	QMutexLocker l(&theSubProc->theDataInUse);
		theSubProc->theNoMoreTransactions = true;
		theSubProc->theDataChanged.wakeAll();
	}
}

void xSCoupling::stoppedR()
{
	{	QMutexLocker l(&theSubProc->theDataInUse);
		theSubProc->theNoMoreTransactions = false;
	}
}

void xSCoupling::go()
{
	theSubProc->go();
}

void xSCoupling::stop()
{
	theSubProc->stop();
}

void xSCoupling::transact(const BufferDatas &d, uint chunks)
{
	theSubProc->transact(d, chunks);
}

BufferDatas xSCoupling::deliverResults(uint *timeTaken)
{
	return theSubProc->deliverResults(timeTaken);
}

void xSCoupling::specifyTypes(const SignalTypeRefs &inTypes, const SignalTypeRefs &outTypes)
{
	SignalTypeRefs dummyOutTypes(outTypes.count());

	if (!theSubProc->proxyVSTypes(inTypes, dummyOutTypes))
	{	qDebug("*** CRITICAL: SubProcessor does not verify previously validated types.");
	}
	theSubProc->theOutTypes = outTypes;
	// TODO: check outTypes == dummyOutTypes, taking into account MultiOut
}

void xSCoupling::initFromProperties(const Properties &p)
{
	theSubProc->initFromProperties(p);
}

void xSCoupling::defineIO(uint inputs, uint outputs)
{
	theSubProc->defineIO(inputs, outputs);
}

}

#undef MESSAGES
