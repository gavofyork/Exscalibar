/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <cmath>
using namespace std;

#include "domprocessor.h"
#include "bufferreader.h"
#include "dxcoupling.h"

#define MESSAGES 0

namespace Geddei
{

DxCoupling::DxCoupling(DomProcessor *dom): theDomProcessor(dom)
{
}

DxCoupling::~DxCoupling()
{
}

void DxCoupling::skipPlungeAndSend(uint samples)
{
	if (MESSAGES) qDebug("> DRCoupling::skipPlungeAndSend()");

	for (uint i = 0; i < (uint)theReaders.count(); i++)
	{	theReaders[i]->skipElements(samples * theReaders[i]->type()->scope());
		theReaders[i]->skipPlunger();
	}
	theDomProcessor->checkExit();
	transact(BufferDatas(), 0);
	if (MESSAGES) qDebug("< DRCoupling::skipPlungeAndSend()");
}

void DxCoupling::skip(uint samples)
{
	for (uint i = 0; i < (uint)theReaders.count(); i++)
		theReaders[i]->skipElements(samples * theReaders[i]->type()->scope());
}

void DxCoupling::peekAndSend(uint samples, uint chunks)
{
	if (MESSAGES) qDebug("> DxCoupling::peekAndSend()");

	// TODO: Checksum each chunk here against what the SubProcessor sees.

	uint nr = theReaders.count();
	BufferDatas d(nr);
	for (uint j = 0; j < nr; j++)
	{	//qDebug("DxCoupling: Reading from reader %d...", j);
		d.copyData(j, theReaders[j]->readElements(samples * theReaders[j]->type()->scope(), false));
	}
	if (MESSAGES) qDebug("DxCoupling: Done reads.");
	// We might have been fobbed off with one or more duds if the trapdoors have been opened since the last check.
	// If we do have to exit, all read data should be taken care of OK, since BufferDatas has the nice autodeletion facility.
	theDomProcessor->checkExit();
#ifdef EDEBUG
	for (uint j = 0; j < nr; j++)
		assert(d[j].isValid());
#endif

	for (uint i = 0; i < nr; i++)
	{
		for (uint e = 0; e < d[i].elements(); e++)
		{
			if (isnan(d[i][e]) || isinf(d[i][e]))
			{
				qDebug("%s: Contains non-finite value on input %d, element %d", qPrintable(theDomProcessor->name()), i, e);
			}
		}
	}

	if (d[0].plunger())
		qDebug("*** STRANGE: BufferData contains plunger at end! (Asked for %d samples, got %d samples)", samples, d[0].samples());
	if (MESSAGES) qDebug("= DxCoupling::peekAndSend(): transact()ing...");
	transact(d, chunks);
	if (MESSAGES) qDebug("< DxCoupling::peekAndSend()");
}

void DxCoupling::stoppingL()
{
	if (MESSAGES) qDebug("DxCoupling::stopping(): Opening trapdoors...");
	for (uint i = 0; i < (uint)theReaders.count(); i++)
		theReaders[i]->openTrapdoor(theDomProcessor);
}

void DxCoupling::stoppedL()
{
	if (MESSAGES) qDebug("DxCoupling::stopped(): Closing trapdoors...");
	for (uint i = 0; i < (uint)theReaders.count(); i++)
		theReaders[i]->closeTrapdoor(theDomProcessor);
}

BufferDatas DxCoupling::returnResults()
{
	if (MESSAGES) qDebug("= DRCoupling::returnResults()");
	return deliverResults(&theLastTimeTaken);
}

}

#undef MESSAGES
